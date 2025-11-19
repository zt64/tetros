#include "kernel/system.hpp"

#include <cstdarg>
#include <lib/format.hpp>

#include "driver/screen.hpp"
#include "driver/serial.hpp"

// read from b
unsigned char inb(uint16_t _port) {
    unsigned char rv;
    asm volatile("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outb(uint16_t _port, unsigned char _data) {
    asm volatile("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void panic(const char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    const char* formatted = vformat(msg, ap);
    va_end(ap);
    screen::clear();
    screen::draw(formatted, 0, 0);
    screen::flush();
    serial::print(formatted);

    for (;;) {
        asm volatile("hlt");
    }
}