#include "driver/serial.hpp"
#include "kernel/system.hpp"
#include "lib/format.hpp"

namespace serial {
    static bool s_available = false;

    int init() {
        outb(PORT + 1, 0x00); // Disable all interrupts
        outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, 0x00); //                  (hi byte)
        outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
        outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
        outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
        outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)
        // Check if serial is faulty (i.e: not same byte as sent)
        if (inb(PORT + 0) != 0xAE) {
            s_available = false;
        }

        // If serial is not faulty set it in normal operation mode
        outb(PORT + 4, 0x0F);

        // Mark initialized (we'll attempt to write but with a bounded wait)
        s_available = true;
        return 0;
    }

    bool received() {
        return inb(PORT + 5) & 1;
    }

    char read() {
        while (received() == 0) {}

        return static_cast<char>(inb(PORT));
    }

    bool transmitted() {
        return inb(PORT + 5) & 0x20;
    }

    void putchar(const char c) {
        if (!s_available) return;
        int timeout = 10000;
        while (timeout-- > 0 && !transmitted()) {
            asm volatile ("pause");
        }
        outb(PORT, c);
    }

    void print(const char* str) {
        if (!str) return;
        while (*str) {
            putchar(*str++);
        }
    }

    void printf(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        print(vformat(fmt, ap));
        va_end(ap);
    }

    bool available() {
        return s_available;
    }
}