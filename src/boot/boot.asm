BITS 32

section .multiboot
align 8
multiboot_header_start:
    dd 0xE85250D6                ; magic number
    dd 0                         ; architecture (0 = i386)
    dd multiboot_header_end - multiboot_header_start; header length
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header_start)) ; checksum

    ; Framebuffer tag (type 5)
    align 8
    dw 5       ; type
    dw 0       ; flags
    dd 20      ; size
    dd 640     ; width
    dd 480     ; height
    dd 32      ; depth

    ; End tag
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096
p1_table_0:
    resb 4096
p1_table_1:
    resb 4096
stack_bottom:
    resb 16384
stack_top:

section .rodata
gdt64:
    dq 0
    dq 0x00af9b000000ffff     ; code segment (64-bit)
    dq 0x00af93000000ffff     ; data segment (64-bit)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .text
extern long_mode_start
global start
start:
	mov esp, stack_top  ; setup stack pointer
	mov edi, eax    ; put multiboot info on stack
	mov esi, ebx    ; put multiboot magic number on stack

    call set_up_page_tables
    call enable_paging

    lgdt [gdt64.pointer]

    jmp 0x08:long_mode_start

; https://os.phil-opp.com/entering-longmode/
set_up_page_tables:
    ; map first P4 entry to P3 table
    mov eax, p3_table
    or eax, 0x3 ; present + writable
    mov [p4_table], eax

    mov esi, p2_table

.map_p3_entries:
    mov eax, esi
    or eax, 0x3 ; present + writable
    mov [p3_table + ecx * 8], eax
    add esi, 4096

    mov ecx, 0         ; counter variable

.map_p2_table:
    ; map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000  ; 2MiB
    mul ecx            ; start address of ecx-th page
    or eax, 0b10000011 ; present + writable + huge
    mov [p2_table + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table  ; else map the next entry

    ret

enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret
