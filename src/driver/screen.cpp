#include <cstddef>
#include <cstdint>

#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "kernel/multiboot.h"
#include "lib/font8x8.hpp"
#include "lib/mem.hpp"

uint64_t* fb_addr = nullptr;
uint32_t fb_width = 0;
uint32_t fb_height = 0;
uint32_t fb_pitch = 0;
uint8_t fb_bpp = 0;

static constexpr size_t VGA_BUFFER_BYTES = (640 * 480 * 32) / 8;

static uint8_t vga_buffer[VGA_BUFFER_BYTES] = {};

void fb_init(const multiboot_tag_framebuffer *fb_tag) {
    fb_addr = reinterpret_cast<uint64_t *>(fb_tag->common.framebuffer_addr);
    fb_width = fb_tag->common.framebuffer_width;
    fb_height = fb_tag->common.framebuffer_height;
    fb_pitch = fb_tag->common.framebuffer_pitch;
    fb_bpp = fb_tag->common.framebuffer_bpp;
    serial::printf("Framebuffer: addr: 0x%p %ux%u, %u bpp\n", fb_addr, fb_width, fb_height, fb_bpp);
}

namespace screen {
    // TODO: Implement font scaling
    void draw_char(
        const unsigned char c,
        const uint32_t x,
        const uint32_t y,
        const uint32_t fgcolor,
        const uint32_t bgcolor
    ) {
        const unsigned char* glyph = font8x8_basic[c];

        for (uint32_t cy = 0; cy < 8; cy++) {
            for (uint32_t cx = 0; cx < 8; cx++) {
                const uint8_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
                put_pixel(x + cx, y + cy, glyph[cy] & mask[cx] ? fgcolor : bgcolor);
            }
        }
    }

    void draw(const char* str, const uint32_t x, const uint32_t y) {
        draw(str, x, y, 0xFFFFFF, 0x000000);
    }

    void draw(
        const char* str,
        const uint32_t x,
        const uint32_t y,
        const uint32_t fgcolor,
        const uint32_t bgcolor
    ) {
        for (uint32_t i = 0; str[i] != '\0'; i++) {
            draw_char(str[i], x + i * 8, y, fgcolor, bgcolor);
        }
    }

    constexpr int border_size = 15;

    void draw_rect(const uint32_t start_x, const uint32_t start_y, const uint32_t width, const uint32_t height) {
        draw_rect(start_x, start_y, width, height, 0xFFFFFF);
    }

    void draw_rect(
        const uint32_t start_x,
        const uint32_t start_y,
        const uint32_t width,
        const uint32_t height,
        const uint32_t color
    ) {
        for (uint32_t y = start_y; y < start_y + height; y++) {
            for (uint32_t x = start_x; x < start_x + width; x++) {
                put_pixel(x, y, color);
            }
        }
    }

    void draw_rect_outline(
        const uint32_t start_x,
        const uint32_t start_y,
        const uint32_t width,
        const uint32_t height
    ) {
        draw_rect_outline(start_x, start_y, width, height, 0xFFFFFF);
    }

    void draw_rect_outline(
        const uint32_t start_x,
        const uint32_t start_y,
        const uint32_t width,
        const uint32_t height,
        const uint32_t color
    ) {
        const uint32_t end_x = start_x + width;
        const uint32_t end_y = start_y + height;

        // Draw top border
        for (uint32_t y = start_y; y < start_y + border_size; y++) {
            for (uint32_t x = start_x; x < end_x; x++) {
                put_pixel(x, y, color);
            }
        }

        // Draw bottom border
        for (uint32_t y = end_y - border_size; y < end_y; y++) {
            for (uint32_t x = start_x; x < end_x; x++) {
                put_pixel(x, y, color);
            }
        }

        // Draw left border
        for (uint32_t y = start_y + border_size; y < end_y - border_size; y++) {
            for (uint32_t x = start_x; x < start_x + border_size; x++) {
                put_pixel(x, y, color);
            }
        }

        // Draw right border
        for (uint32_t y = start_y + border_size; y < end_y - border_size; y++) {
            for (uint32_t x = end_x - border_size; x < end_x; x++) {
                put_pixel(x, y, color);
            }
        }
    }

    void put_pixel(const uint32_t x, const uint32_t y, const uint32_t color) {
        auto* location = reinterpret_cast<uint32_t *>(vga_buffer + fb_pitch * y + x * 4);
        *location = color;
    }

    void clear() {
        memset(vga_buffer, 0, VGA_BUFFER_BYTES);
    }

    void flush() {
        memcpy(fb_addr, vga_buffer, VGA_BUFFER_BYTES);
    }
}
