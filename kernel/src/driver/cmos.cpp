#include "driver/cmos.hpp"

#include "driver/timer.hpp"
#include "kernel/system.hpp"

#define NMI_disable_bit 0

#define CMOS_RTC_SECONDS 0x00

uint32_t get_time() {
    outb(0x70, (NMI_disable_bit << 7) | CMOS_RTC_SECONDS);
    timer::wait_ms(10);
    return inb(0x71);
}
