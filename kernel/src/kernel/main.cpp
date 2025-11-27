#include "driver/cmos.hpp"
#include "driver/pci.hpp"
#include "driver/ps2.hpp"
#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "driver/timer.hpp"
#include "driver/limine/limine_requests.hpp"
#include "kernel/gdt.hpp"
#include "kernel/idt.hpp"
#include "kernel/irq.hpp"
#include "lib/log.hpp"
#include "lib/rand.hpp"
#include "tetris/tetris.hpp"

extern "C" [[noreturn]] void kmain() {
    serial::init();

    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_requests::limine_base_revision)) {
        panic("Unsupported base revision");
    }

    if (limine_requests::framebuffer_request.response == nullptr
        || limine_requests::framebuffer_request.response->framebuffer_count < 1) {
        panic("No framebuffer provided");
    }

    const limine_framebuffer* limine_framebuffer = limine_requests::framebuffer_request.response->framebuffers[0];

    fb_init(limine_framebuffer);
    logger.debug("Framebuffer initialized");

    logger.info("Kernel cmdline=%s", limine_requests::executable_cmdline_request.response->cmdline);

    logger.info("Initializing kernel...");

    gdt_init();
    logger.info("GDT initialized");

    idt_init();
    logger.info("IDT initialized");

    irq_init();
    logger.info("IRQ initialized");

    // paging::init();

    timer::init(100); // 100 times per second
    logger.info("PIC Timer initialized");

    kb_init();
    logger.info("PS/2 Keyboard initialized");

    asm volatile("sti");
    logger.info("Interrupts enabled");

    logger.info("Ready!");

    uint64_t seed = get_time();
    logger.debug("Seeding RNG with time: %llu", seed);
    srand(seed);

    kb_register_listener(Tetris::handle_key);

    logger.info("Starting PCI bus enumeration");
    pci::enumerate_busses();
    logger.info("PCI enumeration completed");

    logger.debug("Entering main loop");

    for (;;) {
        kb_process_queue();
        screen::clear();
        Tetris::update();
        screen::flush();
        timer::wait_ms(10); // ~100 FPS
    }
}
