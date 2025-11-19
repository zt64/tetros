#include "kernel/isrs.hpp"
#include "driver/serial.hpp"
#include "kernel/idt.hpp"
#include "kernel/system.hpp"

extern "C" void* isr_stub_table[];

void isrs_init() {
    for (uint32_t i = 0; i < 32; i++) {
        idt_set_gate(i, reinterpret_cast<uint64_t>(isr_stub_table[i]), 0x8E);
    }
}

const char* exception_messages[] = {
    "Divide By Zero",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

extern "C" void fault_handler(const regs* r) {
    serial::printf("%s Exception. System Halted.\n\n", exception_messages[r->int_no]);

    serial::printf("Registers:\n");
    serial::printf("RAX=0x%016lx RBX=0x%016lx RCX=0x%016lx RDX=0x%016lx\n", r->rax, r->rbx, r->rcx, r->rdx);
    serial::printf("RSI=0x%016lx RDI=0x%016lx RBP=0x%016lx\n", r->rsi, r->rdi, r->rbp);
    serial::printf("R8 =0x%016lx R9 =0x%016lx R10=0x%016lx R11=0x%016lx\n", r->r8, r->r9, r->r10, r->r11);
    serial::printf("R12=0x%016lx R13=0x%016lx R14=0x%016lx R15=0x%016lx\n", r->r12, r->r13, r->r14, r->r15);
    serial::printf("RIP=0x%016lx RSP=0x%016lx RFLAGS=0x%016lx\n", r->rip, r->rsp, r->rflags);
    serial::printf("CS=0x%04lx SS=0x%04lx\n", r->cs, r->ss);
    serial::printf("Int#=0x%02lx Err=0x%016lx\n", r->int_no, r->err_code);

    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));

    serial::printf("CR2=0x%016lx\n", cr2);

    for (;;) asm("hlt");
}
