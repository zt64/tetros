#include "tetris/tetris.hpp"

#include "driver/keyboard.hpp"
#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "lib/mem.hpp"
#include "lib/rand.hpp"
#include "lib/format.hpp"
#include "lib/string.hpp"

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

Tetromino Tetris::held = {PIECE_I_DEF, 3, 0};
Block Tetris::game_blocks[height][width] = {};
GameState Tetris::state = STATE_START;
int Tetris::time, Tetris::score, Tetris::full_lines = 0;
int Tetris::level = 1;

static uint32_t frame_counter = 0;
static uint32_t frames_per_drop = 50;

bool Tetris::collides(uint8_t piece[PIECE_SIZE][PIECE_SIZE], int8_t x, int8_t y) {
    for (uint8_t rel_y = 0; rel_y < PIECE_SIZE; rel_y++) {
        for (uint8_t rel_x = 0; rel_x < PIECE_SIZE; rel_x++) {
            if (piece[rel_y][rel_x] != 1) continue;

            const int8_t screen_x = rel_x + x;
            const int8_t screen_y = rel_y + y;

            if (screen_y >= height) return true;

            if (screen_x < 0 || screen_x >= width) return true;

            if (screen_y >= 0) {
                if (game_blocks[screen_y][screen_x].color != 0) {
                    return true;
                }
            }
        }
    }

    return false;
}

void Tetris::move(const int8_t dir_x, const int8_t dir_y) {
    const int8_t next_x = held.x + dir_x;
    const int8_t next_y = held.y + dir_y;

    if (collides(held.def.pixels, next_x, next_y)) {
        if (dir_y > 0) drop_piece();
        return;
    }

    held.x = next_x;
    held.y = next_y;
}

void Tetris::rotate() {
    // TODO: Don't rotate O piece

    uint8_t tmp[PIECE_SIZE][PIECE_SIZE];
    for (uint8_t i = 0; i < PIECE_SIZE; i++) {
        for (uint8_t j = 0; j < PIECE_SIZE; j++) {
            tmp[j][PIECE_SIZE - i - 1] = held.def.pixels[i][j];
        }
    }

    if (collides(tmp, held.x, held.y)) return;

    memcpy(held.def.pixels, tmp, 16 * sizeof(uint8_t));
}

void Tetris::new_piece() {
    const uint8_t n = (rand() & 0x7FFF) % 7;

    const PieceDef* def_ptr = nullptr;

    switch (n) {
        case 0: def_ptr = &PIECE_I_DEF;
            break;
        case 1: def_ptr = &PIECE_J_DEF;
            break;
        case 2: def_ptr = &PIECE_T_DEF;
            break;
        case 3: def_ptr = &PIECE_L_DEF;
            break;
        case 4: def_ptr = &PIECE_O_DEF;
            break;
        case 5: def_ptr = &PIECE_Z_DEF;
            break;
        case 6: def_ptr = &PIECE_S_DEF;
            break;
        default: {
            def_ptr = &PIECE_I_DEF;

            break;
        };
    }

    memcpy(&held.def, def_ptr, sizeof(PieceDef));
    held.x = 3;
    held.y = 0;
}

void Tetris::drop_piece() {
    for (uint8_t rel_y = 0; rel_y < PIECE_SIZE; rel_y++) {
        for (uint8_t rel_x = 0; rel_x < PIECE_SIZE; rel_x++) {
            if (held.def.pixels[rel_y][rel_x] != 1) continue;

            const uint8_t x = rel_x + held.x;
            const uint8_t y = rel_y + held.y;

            if (y < 0 || y >= height || x < 0 || x >= width) continue;

            game_blocks[y][x] = Block{
                .color = held.def.color,
            };
        }
    }

    new_piece();
    check_row();

    // Check for game over: if the new piece immediately collides at its spawn position
    if (collides(held.def.pixels, held.x, held.y)) {
        state = STATE_GAME_OVER;
    }
}

void Tetris::check_row() {
    uint8_t scored_lines = 0;
    for (uint16_t y = height - 1; y > 0; y--) {
        const Block (&row)[10] = game_blocks[y];
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
                memcpy(game_blocks[y2], game_blocks[y2 - 1], sizeof(Block) * 10);
            }
            memset(game_blocks[0], 0, sizeof(Block) * width);
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
        if (held.y < height) move(0, 1);
        frame_counter = 0;
        time++;
    }

    draw();

    frame_counter++;
}

void Tetris::handle_key(const uint8_t sc) {
    if (sc & 0x80) return; // key release
    switch (sc) {
        case KEY_ARROW_UP:
            rotate();
            break;
        case KEY_ARROW_DOWN:
            move(0, 1);
            break;
        case KEY_ARROW_LEFT:
            move(-1, 0);
            break;
        case KEY_ARROW_RIGHT:
            move(1, 0);
            break;
        case KEY_SPACE:
            if (state == STATE_START) state = STATE_ACTIVE;
            break;
        case KEY_R:
            restart();
            break;
        case KEY_P:
            state == STATE_PAUSED ? state = STATE_ACTIVE : state = STATE_PAUSED;
            break;
        default: break;
    }
}

constexpr uint16_t game_width = 150;
constexpr uint16_t game_height = 300;

void Tetris::draw() {
    constexpr uint16_t border = 15;

    const uint16_t center_x = fb_width / 2;
    const uint16_t start_x = center_x - game_width / 2;
    const uint16_t start_y = fb_height / 2 - game_height / 2;
    const uint16_t text_start_x = center_x + game_width / 2 + border + 16;

    screen::draw_rect_outline(
        start_x - border,
        start_y - border,
        game_width + (border * 2),
        game_height + (border * 2)
    );
    int line = 12;

    if (state == STATE_START) {
        screen::draw(
            "Press [SPACE] to start",
            text_start_x,
            ++line * 8
        );
    } else {
        if (state == STATE_GAME_OVER) {
            const auto game_over_str = "Game Over!";
            const size_t x_offset = (strlen(game_over_str) * 8) / 2;
            screen::draw(
                game_over_str,
                center_x - x_offset,
                12
            );
        }

        constexpr uint16_t block_size = 15;
        screen::draw(format("FULL LINES: %d", full_lines), text_start_x, ++line * 8);
        screen::draw(format("LEVEL: %d", level), text_start_x, ++line * 8);
        screen::draw(format("SCORE: %d", score), text_start_x, ++line * 8);

        const int minutes = time / 60;
        const int seconds = time % 60;
        screen::draw(format("TIME: %02d:%02d", minutes, seconds), text_start_x, ++line * 8);
        line++;
        if (state == STATE_PAUSED) {
            screen::draw("[P]: Unpause", text_start_x, ++line * 8);
        } else {
            screen::draw("[P]: Pause", text_start_x, ++line * 8);
        }

        screen::draw("[R]: Restart", text_start_x, ++line * 8);

        // Draw the active piece
        for (uint8_t y = 0; y < 4; ++y) {
            for (uint8_t x = 0; x < 4; ++x) {
                if (held.def.pixels[y][x]) {
                    const uint8_t grid_x = held.x + x;
                    const uint8_t grid_y = held.y + y;

                    const uint16_t pixel_x = start_x + (grid_x * block_size);
                    const uint16_t pixel_y = start_y + (grid_y * block_size);

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

        for (uint8_t y = 0; y < height; y++) {
            for (uint8_t x = 0; x < width; x++) {
                const auto [color] = game_blocks[y][x];
                if (color == 0) continue;

                const uint16_t pixel_x = start_x + (x * block_size);
                const uint16_t pixel_y = start_y + (y * block_size);

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
    memset(game_blocks, {}, sizeof(game_blocks));
    new_piece();
}
