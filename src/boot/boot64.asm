BITS 64

global long_mode_start
extern kernel_main
long_mode_start:
    ; load data segment selector into all data segment registers
    mov ax, 0x10      ; GDT data segment
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	call kernel_main

	pop rdi
	pop rsi

	cli
.hang:
	hlt
	jmp .hang

global gdt_flush
extern gp
gdt_flush:
    lgdt [gp]           ; Load the GDT pointer

    mov rax, .reload_cs
    push 0x08           ; Push code segment selector (offset into GDT)
    push rax
    retfq               ; Far return to reload CS

.reload_cs:
    ; Reload data segment registers
    mov ax, 0x10        ; Data segment selector (offset 0x10 in GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

global idt_load
extern idtr
idt_load:
    lidt [idtr]
    ret
