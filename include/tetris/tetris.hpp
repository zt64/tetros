#pragma once

#include <cstdint>

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

struct Block {
    uint32_t color;
};

struct PieceDef {
    uint8_t pixels[4][4];
    uint32_t color;
};

struct Tetromino {
    PieceDef def;
    int8_t x;
    int8_t y;
};

#define PIECE_SIZE 4

class Tetris {
public:
    static void update();
    static void handle_key(uint8_t sc);

private:
    static constexpr uint16_t height = 20;
    static constexpr uint16_t width = 10;
    static Tetromino held, next;
    static Block game_blocks[height][width];
    static GameState state;
    static int time, score, level, full_lines;

    static bool collides(uint8_t piece[PIECE_SIZE][PIECE_SIZE], int8_t x, int8_t y);
    static void move(int8_t dir_x, int8_t dir_y);
    static void rotate();

    static void new_piece();
    static void drop_piece();
    static void check_row();

    static void restart();

    static void draw();
};
