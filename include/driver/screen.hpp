#pragma once

#include <cstdint>

extern uint8_t* fb_addr;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint8_t fb_bpp;

void fb_init(uint32_t multiboot_info_ptr);

namespace screen {
    void draw_char(unsigned char c, uint32_t x, uint32_t y, uint32_t fgcolor, uint32_t bgcolor);

    void draw(const char* str, uint32_t x, uint32_t y);

    void draw(const char* str, uint32_t x, uint32_t y, uint32_t fgcolor, uint32_t bgcolor);

    void draw_rect(uint32_t start_x, uint32_t start_y, uint32_t width, uint32_t height);

    void draw_rect(uint32_t start_x, uint32_t start_y, uint32_t width, uint32_t height, uint32_t color);

    void draw_rect_outline(uint32_t start_x, uint32_t start_y, uint32_t width, uint32_t height);

    void draw_rect_outline(uint32_t start_x, uint32_t start_y, uint32_t width, uint32_t height, uint32_t color);

    void put_pixel(uint32_t x, uint32_t y, uint32_t color);

    void clear();

    void flush();
}
