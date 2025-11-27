#include "kernel/irq.hpp"
#include "kernel/idt.hpp"

inline irq_handler irq_routines[16];
extern "C" void* irq_stub_table[];

void irq_install_handler(const uint32_t irq, const irq_handler handler) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(const uint32_t irq) {
    irq_routines[irq] = nullptr;
}

void irq_unmask(uint8_t irq) {
    uint16_t port;

    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }

    outb(port, inb(port) & ~(1 << irq));
}

void irq_mask(uint8_t irq) {
    uint16_t port;

    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }

    outb(port, inb(port) | (1 << irq));
}

// remap irq
void irq_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    // outb(0x21, 0x0);
    // outb(0xA1, 0x0);
}

void irq_init() {
    irq_remap();
    irq_unmask(1);

    for (uint8_t i = 0; i < 16; i++) {
        // Hardware interrupts start after 32
        idt_set_gate(32 + i, reinterpret_cast<uint64_t>(irq_stub_table[i]), 0x8E);
    }
}

extern "C" void irq_handle(regs* r) {
    if (const irq_handler handler = irq_routines[r->int_no - 32]) handler(r);

    if (r->int_no >= 40) {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20); // send end of interrupt (EOI) signal
}
