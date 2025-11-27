#include "kernel/system.hpp"

#include <cstdarg>
#include <lib/format.hpp>

#include "driver/screen.hpp"
#include "lib/log.hpp"

// read from b
uint8_t inb(uint16_t port) {
    uint8_t rv;
    asm volatile("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

uint32_t inl(uint16_t port) {
    uint32_t rv;
    asm volatile("inl %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void outl(uint16_t port, uint32_t data) {
    asm volatile("outl %1, %0" : : "dN" (port), "a" (data));
}

void panic(const char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    const char* formatted = vformat(msg, ap);
    va_end(ap);
    logger.fatal(formatted);
    screen::clear();
    screen::draw(formatted, 0, 0, 1.5);
    screen::flush();

    for (;;) asm volatile("hlt");
}