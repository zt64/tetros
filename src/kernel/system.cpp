#include "kernel/system.hpp"

#include <cstdarg>
#include <lib/format.hpp>

#include "driver/screen.hpp"
#include "driver/serial.hpp"

// read from b
uint8_t inb(uint16_t port) {
    uint8_t rv;
    asm volatile("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

void panic(const char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    const char* formatted = vformat(msg, ap);
    va_end(ap);
    screen::clear();
    screen::draw(formatted, 0, 0, 1.5);
    screen::flush();
    serial::print(formatted);

    for (;;) asm volatile("hlt");
}