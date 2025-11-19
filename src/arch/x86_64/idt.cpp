#include "kernel/idt.hpp"
#include "driver/screen.hpp"
#include "kernel/isrs.hpp"
#include "lib/mem.hpp"

__attribute__((aligned(0x10)))
idt_entry_t idt[256];
idtr_t idtr;

void idt_set_gate(const uint8_t num, const uint64_t base, const uint8_t flags) {
    idt_entry_t* descriptor = &idt[num];

    descriptor->isr_low = base & 0xFFFF;
    descriptor->kernel_cs = 0x08;
    descriptor->ist = 0;
    descriptor->isr_high = (base >> 16) & 0xFFFF;
    descriptor->isr_high2 = (base >> 32) & 0xFFFFFFFF;
    descriptor->attributes = flags;
    descriptor->reserved = 0;
}

void idt_init() {
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtr.base = reinterpret_cast<uintptr_t>(&idt);

    /* Clear out the entire IDT, initializing it to zeros */
    memset2(&idt, 0, sizeof(idt_entry_t) * 256);

    isrs_init();

    idt_load();
}
