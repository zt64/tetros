#include "tetris/tetris.hpp"

#include "driver/ps2/keyboard.hpp"
#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "memory/mem.hpp"
#include "lib/rand.hpp"
#include "lib/format.hpp"

constexpr PieceDef PIECE_O_DEF = {
    {
        {1, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    0xFFFF00
};

constexpr PieceDef PIECE_I_DEF = {
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    0x00FFFF,
};

constexpr PieceDef PIECE_S_DEF = {
    {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    0x00FF00
};

constexpr PieceDef PIECE_Z_DEF = {
    {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    0xFF0000
};

constexpr PieceDef PIECE_L_DEF = {
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    0xFFA500
};

constexpr PieceDef PIECE_J_DEF = {
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
    },
    0x0000FF
};

constexpr PieceDef PIECE_T_DEF = {
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    0x800080
};

Tetromino Tetris::held;
Mino Tetris::board[board_height][board_width] = {};
GameState Tetris::state = STATE_START;
uint32_t Tetris::time, Tetris::score, Tetris::full_lines = 0;
uint32_t Tetris::level = 1;

static uint32_t frame_counter = 0;
static uint32_t frames_per_drop = 100;

bool Tetris::collides(uint8_t piece[PIECE_SIZE][PIECE_SIZE], const int8_t x, const int8_t y) {
    for (uint8_t rel_y = 0; rel_y < PIECE_SIZE; rel_y++) {
        for (uint8_t rel_x = 0; rel_x < PIECE_SIZE; rel_x++) {
            if (piece[rel_y][rel_x] != 1) continue;

            const auto screen_x = static_cast<int8_t>(rel_x + x);
            const auto screen_y = static_cast<int8_t>(rel_y + y);

            // Out of bounds horizontally or below the board
            if (screen_x < 0 || screen_x >= board_width || screen_y >= board_height) return true;

            // Collision with existing block
            if (board[screen_y][screen_x].color != 0) return true;
        }
    }

    return false;
}

void Tetris::move(const int8_t dir_x, const int8_t dir_y) {
    const auto next_x = static_cast<int8_t>(held.x + dir_x);
    const auto next_y = static_cast<int8_t>(held.y + dir_y);

    if (collides(held.def.minos, next_x, next_y)) {
        if (dir_y > 0) drop_piece();
        return;
    }

    held.x = next_x;
    held.y = next_y;
}

void Tetris::rotate_cw() {
    // TODO: Don't rotate O piece

    uint8_t tmp[PIECE_SIZE][PIECE_SIZE];
    for (uint8_t i = 0; i < PIECE_SIZE; i++) {
        for (uint8_t j = 0; j < PIECE_SIZE; j++) {
            tmp[j][PIECE_SIZE - i - 1] = held.def.minos[i][j];
        }
    }

    if (collides(tmp, held.x, held.y)) return;

    memcpy(held.def.minos, tmp, 16 * sizeof(uint8_t));
}

void Tetris::rotate_ccw() {
    // TODO: Don't rotate O piece

    uint8_t tmp[PIECE_SIZE][PIECE_SIZE];
    for (uint8_t i = 0; i < PIECE_SIZE; i++) {
        for (uint8_t j = 0; j < PIECE_SIZE; j++) {
            tmp[PIECE_SIZE - j - 1][i] = held.def.minos[i][j];
        }
    }

    if (collides(tmp, held.x, held.y)) return;

    memcpy(held.def.minos, tmp, 16 * sizeof(uint8_t));
}

uint8_t Tetris::bag_pieces[7] = {};
uint8_t Tetris::bag_size = 0;

// https://tetris.wiki/Random_Generator
void Tetris::new_piece() {
    static const PieceDef* defs[7] = {
        &PIECE_I_DEF, &PIECE_J_DEF, &PIECE_T_DEF, &PIECE_L_DEF, &PIECE_O_DEF, &PIECE_Z_DEF, &PIECE_S_DEF
    };

    if (bag_size == 0) {
        for (uint8_t i = 0; i < 7; i++) bag_pieces[i] = i;
        for (uint32_t i = 6; i > 0; --i) {
            const int j = rand() % (i + 1);
            const uint8_t tmp = bag_pieces[i];
            bag_pieces[i] = bag_pieces[j];
            bag_pieces[j] = tmp;
        }
        bag_size = 7;
    }

    held.def = *defs[bag_pieces[--bag_size]];
    held.x = 3;
    held.y = 0;
}

void Tetris::drop_piece() {
    for (uint8_t rel_y = 0; rel_y < PIECE_SIZE; rel_y++) {
        for (uint8_t rel_x = 0; rel_x < PIECE_SIZE; rel_x++) {
            if (held.def.minos[rel_y][rel_x] != 1) continue;

            const uint8_t x = rel_x + held.x;
            const uint8_t y = rel_y + held.y;

            if (y < 0 || y >= board_height || x < 0 || x >= board_width) continue;

            board[y][x] = Mino{
                .color = held.def.color,
            };
        }
    }

    new_piece();
    check_row();

    // Check for game over: if the new piece immediately collides at its spawn position
    if (collides(held.def.minos, held.x, held.y)) {
        state = STATE_GAME_OVER;
    }
}

void Tetris::check_row() {
    uint8_t scored_lines = 0;
    for (uint16_t y = board_height - 1; y > 0; y--) {
        const Mino (&row)[board_width] = board[y];
        bool full_row = true;
        bool empty_row = true;

        for (auto block: row) {
            if (block.color == 0) {
                full_row = false;
            } else {
                empty_row = false; // No point in continuing
            }
        }

        if (empty_row) break;

        if (full_row) {
            for (uint16_t y2 = y; y2 > 0; y2--) {
                memcpy(board[y2], board[y2 - 1], sizeof(Mino) * board_width);
            }
            memset(board[0], 0, sizeof(Mino) * board_width);
            y++;
            full_lines++;
            scored_lines++;
        }
    }

    // Increase level every 10 lines
    if (full_lines / 10 > level - 1) {
        level = (full_lines / 10) + 1;
        frames_per_drop = 50 - (level * 2);
    }

    switch (scored_lines) {
        case 1:
            score += 100 * level;
            break;
        case 2:
            score += 300 * level;
            break;
        case 3:
            score += 500 * level;
            break;
        default: break;
    }
}

void Tetris::update() {
    if (state == STATE_START || state == STATE_PAUSED || state == STATE_GAME_OVER) {
        draw();
        return;
    }

    if (frame_counter >= frames_per_drop) {
        if (held.y < board_height) move(0, 1);
        frame_counter = 0;
        time++;
    }

    draw();

    frame_counter++;
}

void Tetris::handle_key(const KeyEvent ev) {
    if (ev.break_key) return;
    switch (ev.scancode) {
        case KEY_X:
        case KEY_W:
        case KEY_ARROW_UP:
            rotate_cw();
            break;
        case KEY_Z:
        case KEY_CONTROL:
            rotate_ccw();
            break;
        case KEY_S:
        case KEY_ARROW_DOWN:
            move(0, 1);
            break;
        case KEY_A:
        case KEY_ARROW_LEFT:
            move(-1, 0);
            break;
        case KEY_D:
        case KEY_ARROW_RIGHT:
            move(1, 0);
            break;
        case KEY_SHIFT_LEFT:
        case KEY_SHIFT_RIGHT:
        case KEY_C:
            // TODO: Hold piece
            break;
        case KEY_SPACE:
            if (state == STATE_START) {
                state = STATE_ACTIVE;
                new_piece();
            } else {
                // TODO: Hard drop
            }
            break;
        case KEY_R:
            restart();
            break;
        case KEY_ESCAPE:
        case KEY_P:
            state == STATE_PAUSED ? state = STATE_ACTIVE : state = STATE_PAUSED;
            break;
        default: break;
    }
}

constexpr uint16_t board_width = 150;
constexpr uint16_t board_height = 300;

void Tetris::draw() {
    auto ui_scale = static_cast<float>(framebuffer.height) / 480.0f;
    if (ui_scale < 1.0f) ui_scale = 1.0f;

    const auto block_size = static_cast<uint16_t>(15 * ui_scale);
    const auto border = static_cast<uint16_t>(15 * ui_scale);
    const auto font_height = static_cast<uint16_t>(16 * ui_scale);
    const auto game_width_scaled = block_size * board_width;
    const auto game_height_scaled = block_size * board_height;

    const auto center_x = framebuffer.width / 2;
    const auto center_y = framebuffer.height / 2;
    const auto start_x = center_x - game_width_scaled / 2;
    const auto start_y = center_y - game_height_scaled / 2;
    const auto text_start_x = start_x + game_width_scaled + border + static_cast<uint16_t>(20 * ui_scale);
    auto text_y = start_y;
    auto draw_text_line = [&](const char* text) {
        screen::draw(text, text_start_x, text_y, 1.4);
        text_y += font_height;
    };

    // Draw game border
    screen::draw_rect_outline(
        start_x - border,
        start_y - border,
        game_width_scaled + (border * 2),
        game_height_scaled + (border * 2),
        border
    );

    screen::draw(
        "TetrOS",
        text_start_x,
        start_y - border - 40,
        4,
        0x1ed760
    );
    if (state == STATE_START) {
        screen::draw(
            "Press [SPACE] to start",
            text_start_x,
            start_y,
            1.3
        );
    } else {
        if (state == STATE_GAME_OVER) {
            draw_text_line("Game Over!");
            text_y += font_height / 2;
        }

        draw_text_line(format("FULL LINES: %d", full_lines));
        draw_text_line(format("LEVEL: %d", level));
        draw_text_line(format("SCORE: %d", score));

        const uint32_t minutes = time / 60;
        const uint32_t seconds = time % 60;
        draw_text_line(format("TIME: %02d:%02d", minutes, seconds));
        text_y += font_height / 2;

        draw_text_line(state == STATE_PAUSED ? "[P]: Unpause" : "[P]: Pause");
        draw_text_line("[R]: Restart");

        // Draw the active piece
        for (uint8_t y = 0; y < 4; ++y) {
            for (uint8_t x = 0; x < 4; ++x) {
                if (held.def.minos[y][x]) {
                    const uint8_t grid_x = held.x + x;
                    const uint8_t grid_y = held.y + y;
                    const uint32_t pixel_x = start_x + (grid_x * block_size);
                    const uint32_t pixel_y = start_y + (grid_y * block_size);
                    screen::draw_rect(
                        pixel_x,
                        pixel_y,
                        block_size,
                        block_size,
                        held.def.color
                    );
                }
            }
        }

        // Draw the game board
        for (uint8_t y = 0; y < board_height; y++) {
            for (uint8_t x = 0; x < board_width; x++) {
                const auto [color] = board[y][x];
                if (color == 0) continue;
                const uint32_t pixel_x = start_x + (x * block_size);
                const uint32_t pixel_y = start_y + (y * block_size);
                screen::draw_rect(
                    pixel_x,
                    pixel_y,
                    block_size,
                    block_size,
                    color
                );
            }
        }
    }
}

void Tetris::restart() {
    serial::print("Restarting\n");
    state = STATE_START;
    full_lines = 0;
    score = 0;
    level = 1;
    time = 0;
    memset(board, {}, sizeof(board));
    new_piece();
}
