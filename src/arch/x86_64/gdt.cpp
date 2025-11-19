#include <cstdint>
#include "kernel/gdt.hpp"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

gdt_entry gdt[3];
gdt_ptr gp;

extern "C" void gdt_flush();

/* Set up a descriptor in the Global Descriptor Table */
void gdt_set_gate(
    const int32_t num,
    const uint64_t base,
    const uint64_t limit,
    const unsigned char access,
    const unsigned char gran
) {
    /* Set up the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

/* Should be called by main. This will setup the special GDT
*  pointer, set up the first 3 entries in our GDT, and then
*  finally call gdt_flush() in our assembler file in order
*  to tell the processor where the new GDT is and update the
*  new segment registers */
void gdt_init() {
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(gdt_entry) * 3) - 1;
    gp.base = reinterpret_cast<uintptr_t>(&gdt);

    // null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    /* 64-bit Code Segment.
    *  In long mode, base and limit are ignored.
    *  Access: 0x9A = Present(1) | DPL(00) | Code(1) | Execute/Read(1010)
    *  Granularity: 0xA0 = Long mode(1) | Reserved(0) | Limit high(0000) */
    gdt_set_gate(1, 0, 0, 0x9A, 0xA0);

    /* 64-bit Data Segment.
    *  Access: 0x92 = Present(1) | DPL(00) | Data(1) | Read/Write(0010)
    *  Granularity: 0x00 (base/limit ignored in 64-bit) */
    gdt_set_gate(2, 0, 0, 0x92, 0x00);

    gdt_flush();
}
