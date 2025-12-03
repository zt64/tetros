#include "driver/screen.hpp"
#include "driver/limine/limine.h"
#include "lib/font8x8.hpp"
#include "memory/mem.hpp"
#include "lib/log.hpp"

static uint8_t* vga_buffer = nullptr;

void fb_init(const limine_framebuffer* fb) {
    framebuffer.width = fb->width;
    framebuffer.height = fb->height;
    framebuffer.pitch = fb->pitch;
    framebuffer.bpp = fb->bpp;
    framebuffer.addr = fb->address;
    framebuffer.size = (framebuffer.width * framebuffer.height * framebuffer.bpp) / 8;

    vga_buffer = static_cast<uint8_t *>(malloc(framebuffer.size));

    logger.debug(
        "fb_addr = 0x%p, fb_width = %u, fb_height = %u, fb_bpp = %u",
        framebuffer.addr,
        framebuffer.width,
        framebuffer.height,
        framebuffer.bpp
    );
}

namespace screen {
    void draw_char(
        const unsigned char c,
        const uint32_t x,
        const uint32_t y,
        const float size,
        const uint32_t fgcolor,
        const uint32_t bgcolor
    ) {
        const unsigned char* glyph = font8x8_basic[c];
        for (uint32_t cy = 0; cy < 8; cy++) {
            for (uint32_t cx = 0; cx < 8; cx++) {
                const uint8_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
                if (!(glyph[cy] & mask[cx])) continue;
                // Calculate rectangle bounds for this pixel using float size
                const uint32_t x0 = x + static_cast<uint32_t>(cx * size);
                const uint32_t y0 = y + static_cast<uint32_t>(cy * size);
                const uint32_t x1 = x + static_cast<uint32_t>((cx + 1) * size);
                const uint32_t y1 = y + static_cast<uint32_t>((cy + 1) * size);
                for (uint32_t py = y0; py < y1; ++py) {
                    for (uint32_t px = x0; px < x1; ++px) {
                        put_pixel(px, py, fgcolor);
                    }
                }
            }
        }
    }

    void draw(const char* str, const uint32_t x, const uint32_t y, const float size) {
        draw(str, x, y, size, 0xFFFFFF);
    }

    void draw(
        const char* str,
        const uint32_t x,
        const uint32_t y,
        const float size,
        const uint32_t fgcolor
    ) {
        for (uint32_t i = 0; str[i] != '\0'; i++) {
            draw_char(str[i], x + static_cast<uint32_t>(i * 8 * size), y, size, fgcolor, 0x000000);
        }
    }

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
        const uint32_t height,
        const uint32_t border_width
    ) {
        draw_rect_outline(start_x, start_y, width, height, border_width, 0xFFFFFF);
    }

    void draw_rect_outline(
        const uint32_t start_x,
        const uint32_t start_y,
        const uint32_t width,
        const uint32_t height,
        const uint32_t stroke_width,
        const uint32_t color
    ) {
        const uint32_t end_x = start_x + width;
        const uint32_t end_y = start_y + height;

        // Draw top border
        for (uint32_t y = start_y; y < start_y + stroke_width; y++) {
            for (uint32_t x = start_x; x < end_x; x++) {
                put_pixel(x, y, color);
            }
        }

        // Draw bottom border
        for (uint32_t y = end_y - stroke_width; y < end_y; y++) {
            for (uint32_t x = start_x; x < end_x; x++) {
                put_pixel(x, y, color);
            }
        }

        // Draw left border
        for (uint32_t y = start_y + stroke_width; y < end_y - stroke_width; y++) {
            for (uint32_t x = start_x; x < start_x + stroke_width; x++) {
                put_pixel(x, y, color);
            }
        }

        // Draw right border
        for (uint32_t y = start_y + stroke_width; y < end_y - stroke_width; y++) {
            for (uint32_t x = end_x - stroke_width; x < end_x; x++) {
                put_pixel(x, y, color);
            }
        }
    }

    void put_pixel(const uint32_t x, const uint32_t y, const uint32_t color) {
        const uint64_t offset = static_cast<uint64_t>(framebuffer.pitch) * y + static_cast<uint64_t>(x) * (framebuffer.bpp / 8);
        if (offset + sizeof(uint32_t) > framebuffer.size) return;
        auto* location = reinterpret_cast<uint32_t *>(vga_buffer + offset);
        *location = color;
    }

    void clear() {
        memset_fast(vga_buffer, 0, framebuffer.size);
    }

    void flush() {
        memcpy_fast(framebuffer.addr, vga_buffer, framebuffer.size);
    }
}
