#pragma once

#include <cstdint>

struct regs {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no;
    uint64_t err_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

unsigned char inb(uint16_t _port);

void outb(uint16_t _port, unsigned char _data);

[[noreturn]] void panic(const char* msg, ...);