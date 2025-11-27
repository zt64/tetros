#pragma once

#include <cstdint>

#include "driver/ps2/keyboard.hpp"

enum PieceType {
    PIECE_O,
    PIECE_I,
    PIECE_S,
    PIECE_Z,
    PIECE_L,
    PIECE_J,
    PIECE_T
};

enum GameState {
    STATE_START,
    STATE_ACTIVE,
    STATE_PAUSED,
    STATE_GAME_OVER
};

struct Mino {
    uint32_t color;
};

#define PIECE_SIZE 4

struct PieceDef {
    uint8_t minos[PIECE_SIZE][PIECE_SIZE];
    uint32_t color;
};

struct Tetromino {
    PieceDef def;
    int8_t x;
    int8_t y;
};

class Tetris {
public:
    static void update();
    static void handle_key(KeyEvent ev);

private:
    static constexpr uint16_t board_height = 20;
    static constexpr uint16_t board_width = 10;
    static Tetromino held, next;
    static Mino board[board_height][board_width];
    static GameState state;
    static uint32_t time, score, level, full_lines;

    static PieceDef pieces[7];
    static uint8_t piece_index;

    static bool collides(uint8_t piece[PIECE_SIZE][PIECE_SIZE], int8_t x, int8_t y);
    static void move(int8_t dir_x, int8_t dir_y);
    static void rotate_cw();
    static void rotate_ccw();

    static uint8_t bag_pieces[7];
    static uint8_t bag_size;

    static void new_piece();
    static void drop_piece();
    static void check_row();

    static void restart();

    static void draw();
};
