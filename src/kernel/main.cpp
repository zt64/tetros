#include <kernel/paging.hpp>

#include "driver/cmos.hpp"
#include "driver/keyboard.hpp"
#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "driver/timer.hpp"
#include "kernel/gdt.hpp"
#include "kernel/idt.hpp"
#include "kernel/irq.hpp"
#include "kernel/multiboot.h"
#include "lib/rand.hpp"
#include "tetris/tetris.hpp"

static void parse_multiboot_info(const uint64_t multiboot_info_ptr) {
    auto* tag_start = reinterpret_cast<multiboot_tag *>(multiboot_info_ptr + 8);

    for (auto* tag = tag_start; tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<multiboot_tag *>(reinterpret_cast<uint8_t *>(tag) + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                const auto* meminfo = reinterpret_cast<multiboot_tag_basic_meminfo *>(tag);
                serial::printf("mem_lower = %uKB, mem_upper = %uKB\n", meminfo->mem_lower, meminfo->mem_upper);
                break;
            }

            case MULTIBOOT_TAG_TYPE_MMAP: {
                serial::printf("mmap\n");
                auto* mmap_tag = reinterpret_cast<multiboot_tag_mmap *>(tag);
                const auto* tag_end = reinterpret_cast<multiboot_uint8_t *>(tag) + tag->size;

                for (auto* mmap = mmap_tag->entries;
                     reinterpret_cast<multiboot_uint8_t *>(mmap) < tag_end;
                     mmap = reinterpret_cast<multiboot_memory_map_t *>(
                         reinterpret_cast<multiboot_uint8_t *>(mmap) + mmap_tag->entry_size)) {
                    serial::printf(
                        " base_addr = 0x%016llX, length = 0x%016llX, type = 0x%X\n",
                        mmap->addr,
                        mmap->len,
                        mmap->type
                    );

                }
                break;
            }

            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                serial::printf("elf sections\n");
                // ELF sections parsing not yet implemented
                break;

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                const auto* fb_tag = reinterpret_cast<multiboot_tag_framebuffer *>(tag);
                fb_init(fb_tag);
                break;
            }

            default:
                break;
        }
    }
}

extern "C" void start_long_mode();

extern "C" void kernel_entry32(const uint64_t multiboot_magic, const uint64_t multiboot_info_ptr) {
    paging::init();
    start_long_mode();
}

extern "C" [[noreturn]] void kernel_entry64(const uint64_t multiboot_magic, const uint64_t multiboot_info_ptr) {
    serial::init();

    // Check to ensure booted by a multiboot2 compliant bootloader
    if (multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        panic("multiboot magic number is incorrect");
    }

    if (multiboot_info_ptr & 7) {
        panic("Unaligned mbi: 0x%x\n", multiboot_info_ptr);
    }

    serial::printf("Initializing kernel...\n");
    serial::printf("Multiboot header size: %d\n", reinterpret_cast<multiboot_header*>(multiboot_info_ptr)->header_length);

    parse_multiboot_info(multiboot_info_ptr);

    gdt_init();
    serial::printf("GDT initialized\n");

    idt_init();
    serial::printf("IDT initialized\n");

    irq_init();
    serial::printf("IRQ initialized\n");

    timer_init();
    serial::printf("Timer initialized\n");

    kb_init();
    serial::printf("Keyboard initialized\n");

    asm volatile("sti");
    serial::printf("Interrupts enabled!\n\nReady\n");

    srand(get_time());

    kb_register_listener(Tetris::handle_key);

    // Testing paging read/write
    auto* location = reinterpret_cast<uint32_t *>(fb_addr + fb_pitch * 320 + 240 * 4);
    *location = 0x1ed760;
    serial::printf("%p\n", location);
    serial::printf("%x\n", *location);

    for (;;) {
        kb_process_queue();
        screen::clear();
        Tetris::update();
        screen::flush();
        timer_wait_ms(10); // ~100 FPS
    }
}
