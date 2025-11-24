#include <kernel/paging.hpp>

#include "driver/cmos.hpp"
#include "driver/keyboard.hpp"
#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "driver/timer.hpp"
#include "driver/limine/limine_requests.hpp"
#include "kernel/gdt.hpp"
#include "kernel/idt.hpp"
#include "kernel/irq.hpp"
#include "lib/rand.hpp"
#include "tetris/tetris.hpp"

extern "C" [[noreturn]] void kmain() {
    serial::init();

    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
        panic("Unsupported base revision");
    }

    if (framebuffer_request.response == nullptr || framebuffer_request.response->framebuffer_count < 1) {
        panic("No framebuffer provided");
    }

    const limine_framebuffer* limine_framebuffer = framebuffer_request.response->framebuffers[0];

    fb_init(limine_framebuffer);

    serial::printf("Initializing kernel...\n");

    gdt_init();
    serial::printf("GDT initialized\n");

    idt_init();
    serial::printf("IDT initialized\n");

    irq_init();
    serial::printf("IRQ initialized\n");

    // paging::init();

    timer_init(100); // 100 times per second
    serial::printf("Timer initialized\n");

    kb_init();
    serial::printf("Keyboard initialized\n");

    asm volatile("sti");
    serial::printf("Interrupts enabled!\n\nReady\n");

    srand(get_time());

    kb_register_listener(Tetris::handle_key);

    for (;;) {
        kb_process_queue();
        screen::clear();
        Tetris::update();
        screen::flush();
        timer_wait_ms(10); // ~100 FPS
    }
}
