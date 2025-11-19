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

void parse_multiboot_info(const uint64_t multiboot_info_ptr) {
    for (auto* tag = reinterpret_cast<multiboot_tag *>(multiboot_info_ptr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<multiboot_tag *>(reinterpret_cast<uint8_t *>(tag) + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                serial::printf(
                    "mem_lower = %uKB, mem_upper = %uKB\n",
                    reinterpret_cast<multiboot_tag_basic_meminfo *>(tag)->mem_lower,
                    reinterpret_cast<multiboot_tag_basic_meminfo *>(tag)->mem_upper
                );
                break;
            case MULTIBOOT_TAG_TYPE_MMAP: {
                serial::printf("mmap\n");

                for (multiboot_memory_map_t* mmap = reinterpret_cast<multiboot_tag_mmap *>(tag)->entries;
                     reinterpret_cast<multiboot_uint8_t *>(mmap)
                     < reinterpret_cast<multiboot_uint8_t *>(tag) + tag->size;
                     mmap = mmap
                            + reinterpret_cast<multiboot_tag_mmap *>(tag)->
                            entry_size)
                    serial::printf(
                        " base_addr = 0x%x%x,"
                        " length = 0x%x%x, type = 0x%x\n",
                        static_cast<unsigned>(mmap->addr >> 32),
                        static_cast<unsigned>(mmap->addr & 0xffffffff),
                        static_cast<unsigned>(mmap->len >> 32),
                        static_cast<unsigned>(mmap->len & 0xffffffff),
                        static_cast<unsigned>(mmap->type)
                    );
                }
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
                serial::printf("elf sections\n");
                const auto* sections_tag = reinterpret_cast<multiboot_tag_elf_sections *>(tag);
                // TODO
                break;
            }
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                const auto* fb_tag = reinterpret_cast<multiboot_tag_framebuffer *>(tag);
                fb_addr = reinterpret_cast<uint8_t *>(fb_tag->common.framebuffer_addr);
                fb_width = fb_tag->common.framebuffer_width;
                fb_height = fb_tag->common.framebuffer_height;
                fb_pitch = fb_tag->common.framebuffer_pitch;
                fb_bpp = fb_tag->common.framebuffer_bpp;
                break;
            }
            default: break;
        }
    }
}

static void init_hardware() {
    serial::printf("Initializing kernel...\n");

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
}

[[noreturn]] static void game_loop() {
    kb_register_listener(Tetris::handle_key);

    for (;;) {
        kb_process_queue();
        screen::clear();
        Tetris::update();
        screen::flush();
        timer_wait_ms(10); // ~100 FPS
    }
}

extern "C" [[noreturn]] void kernel_main(const uint64_t multiboot_magic, const uint64_t multiboot_info_ptr) {
    serial::init();

    // Check to ensure booted by a multiboot2 compliant bootloader
    if (multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        panic("multiboot magic number is incorrect");
    }

    if (multiboot_info_ptr & 7) {
        panic("Unaligned mbi: 0x%x\n", multiboot_info_ptr);
    }

    parse_multiboot_info(multiboot_info_ptr);
    init_hardware();

    srand(get_time());

    game_loop();
}
