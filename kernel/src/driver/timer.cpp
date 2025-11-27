#include "driver/timer.hpp"
#include "kernel/irq.hpp"
#include "driver/screen.hpp"
#include "driver/sound.hpp"
#include "kernel/system.hpp"

namespace timer {
    void timer_handler(regs* r) {
        timer_ticks++;

        snd_tick();
    }

    void init(const uint32_t frequency) {
        irq_install_handler(0, timer_handler);
        set_pit(frequency); // 100 hz
    }

    void set_pit(const uint32_t frequency) {
        // Set PIT to fire at frequency in hz
        // PIT frequency = 1193180 Hz
        // Divisor for 100Hz = 1193180 / 100 = 11931 (0x2E9B)
        const int divisor = PIT_FREQUENCY / frequency;

        outb(0x43, 0x36);

        outb(0x40, divisor & 0xFF);
        outb(0x40, (divisor >> 8) & 0xFF);
    }

    /**
     * Wait for some time
     * @param ticks
     */
    void wait(const uint32_t ticks) {
        const uint32_t eticks = timer_ticks + ticks;
        while (static_cast<int32_t>(eticks - timer_ticks) > 0) {
            // halt until the next interrupt
            asm volatile("hlt");
        }
    }

    void wait_ms(const uint32_t ms) {
        wait(ms / 10);
    }
}