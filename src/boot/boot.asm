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
PML4:
    resb 4096
PDP:
    resb 4096
PD:
    resb 4096
PT:
    resb 4096
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

    xor ecx, ecx
.map_pdp_table:
    ; Map first PDP entry to PD table
    mov eax, PD
    or eax, 0x3 ; Present + writable
    mov [PDP + ecx * 8], eax
    inc ecx            ; Increase counter
    cmp ecx, 512       ; If counter == 512, the whole PDP table is mapped
    jne .map_pdp_table  ; Else map the next entry

    xor ecx, ecx       ; Counter variable = 0
.map_pd_table:
    ; Map first PD entry to PT table
    mov eax, PT
    or eax, 0x3 ; Present + writable
    mov [PD + ecx * 8], eax
    inc ecx            ; Increase counter
    cmp ecx, 512       ; If counter == 512, the whole PD table is mapped
    jne .map_pd_table  ; Else map the next entry

    xor ecx, ecx       ; Counter variable = 0
.map_pt_table:
    ; Map ecx-th PT entry to a 4 KiB page that starts at address 4 KiB * ecx
    mov eax, 0x1000  ; 4 KiB
    mul ecx          ; Start address of ecx-th page
    or eax, 0x3      ; Present + writable
    mov [PT + ecx * 8], eax ; Map ecx-th entry
    inc ecx          ; Increase counter
    cmp ecx, 512     ; If counter == 512, the whole PT table is mapped
    jne .map_pt_table ; Else map the next entry
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