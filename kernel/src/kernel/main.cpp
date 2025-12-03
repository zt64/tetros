#include "driver/acpi.hpp"
#include "driver/apic.hpp"
#include "driver/cmos.hpp"
#include "driver/pic.hpp"
#include "driver/ps2/keyboard.hpp"
#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "driver/timer.hpp"
#include "driver/limine/limine_requests.hpp"
#include "driver/ps2/ps2.hpp"
#include "kernel/cmdline.hpp"
#include "kernel/gdt.hpp"
#include "kernel/idt.hpp"
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

    parse_cmdline(limine_requests::executable_cmdline_request.response->cmdline);

    logger.info("Initializing kernel...");

    gdt_init();
    logger.info("GDT initialized");

    idt_init();
    logger.info("IDT initialized");

    pic::init();
    logger.info("IRQ initialized");

    // mem::init_pmm();
    // logger.info("Phyiscal memory manager initialized");
    //
    // paging::init();
    // logger.info("Paging initialized");
    //
    // apic::init();
    // logger.info("APIC initialized");

    acpi::init();
    logger.info("ACPI tables initialized");

    timer::init(100); // 100 times per second
    logger.info("PIC Timer initialized");

    asm volatile("sti");
    logger.info("Interrupts enabled");

    // apic::apic_start_timer();

    ps2::init();
    logger.info("PS/2 controller initialized");

    kb_init();
    logger.info("PS/2 Keyboard initialized");

    logger.info("Ready!");

    uint64_t seed = get_time();
    logger.debug("Seeding RNG with time: %llu", seed);
    srand(seed);

    kb_register_listener(Tetris::handle_key);
    Tetris::init();

    // logger.info("Starting PCI bus enumeration");
    // pci::enumerate_busses();
    // logger.info("PCI enumeration completed");

    logger.debug("Entering main loop");

    // Event-driven main loop using APIC timer ticks
    // uint64_t last_tick = timer::get_ticks();
    // constexpr uint64_t MAX_TICK_PROCESS = 8; // cap to avoid spiral of death

    for (;;) {
        kb_process_queue();
        screen::clear();
        Tetris::update();
        screen::flush();
        timer::wait_ms(10); // ~100 FPS
        // // Handle timer ticks (process accumulated ticks)
        // const uint64_t now = timer::get_ticks();
        //
        // if (now != last_tick) {
        //     uint64_t delta = now - last_tick;
        //     if (delta > MAX_TICK_PROCESS) delta = MAX_TICK_PROCESS;
        //
        //     for (uint64_t i = 0; i < delta; ++i) {
        //         Tetris::update();
        //     }
        //
        //     last_tick += delta;
        //
        //     // redraw after processing
        //     screen::flush();
        // } else {
        //     // No new tick: wait for next tick or interrupt (keyboard)
        //     timer::wait_for_tick(last_tick);
        // }
    }
}
