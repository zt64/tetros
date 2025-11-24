#pragma once

#include <cstdint>

#define PIT_FREQUENCY 1193182

inline volatile uint64_t timer_ticks = 0;

void timer_init(uint32_t frequency);

void set_pit(uint32_t frequency);

void timer_wait(uint32_t ticks);

void timer_wait_ms(uint32_t ms);
