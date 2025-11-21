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
    dd 1024     ; width
    dd 768     ; height
    dd 32      ; depth

    ; End tag
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

section .bss
align 4096
PML4:
    resb 4096
PDP:
    resb 4096
PD:
    resb 4096 * 4 ; 4 pd tables
PT:
    resb 4096 * 2048 ; 4 * 512 = 2048 pt tables
stack_bottom:
    resb 4096
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
extern start64
extern kernel_entry32
global start
start:
	mov esp, stack_top  ; setup stack pointer
	mov edi, eax    ; put multiboot info on stack
	mov esi, ebx    ; put multiboot magic number on stack

    push edi
    push esi
    call kernel_entry32

    lgdt [gdt64.pointer]
    jmp 0x08:start64

; https://os.phil-opp.com/entering-longmode/
global set_up_page_tables
set_up_page_tables:
    ; map first PML4 entry to PDP table
    mov eax, PDP
    or eax, 0x3 ; present + writable
    mov [PML4], eax

    ; map first 4 PDP entries to 4 PD tables
    xor ecx, ecx
.map_pdp_table:
    mov eax, PD
    mov edx, ecx
    shl edx, 12        ; ecx * 4096 (size of each PD table)
    add eax, edx
    or eax, 0x3        ; present + writable
    mov [PDP + ecx * 8], eax
    inc ecx
    cmp ecx, 4         ; map 4 PDP entries
    jne .map_pdp_table

    ; map all 512 entries in each of the 4 PD tables
    xor ecx, ecx       ; total counter for all PD entries (0-2047)
.map_pd_table:
    mov eax, PT
    mov edx, ecx
    shl edx, 12        ; ecx * 4096 (size of each PT table)
    add eax, edx
    or eax, 0x3        ; present + writable
    mov [PD + ecx * 8], eax
    inc ecx
    cmp ecx, 2048      ; 512 * 4 = 2048 total PD entries
    jne .map_pd_table

    ; map all 512 entries in each of the 2048 PT tables
    xor ecx, ecx       ; total counter for all PT entries (0-1048575)
.map_pt_table:
    mov eax, ecx
    shl eax, 12        ; ecx * 4096 (physical address)
    or eax, 0x13        ; present + writable
    mov [PT + ecx * 8], eax
    inc ecx
    cmp ecx, 1048576   ; 512 * 2048 = 1048576 total pages (4GB)
    jne .map_pt_table

    ret

global start_long_mode
start_long_mode:
    ; load PML4 to cr3 register (cpu uses this to access the PML4 table)
    mov eax, PML4
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