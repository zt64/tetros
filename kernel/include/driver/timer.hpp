#pragma once

#include <cstdint>

#define PIT_FREQUENCY 1193182

namespace timer {
    inline volatile uint64_t timer_ticks = 0;

    void init(uint32_t frequency);

    void set_pit(uint32_t frequency);

    void wait(uint32_t ticks);

    void wait_ms(uint32_t ms);
}
