#include "driver/ps2/ps2.hpp"
#include "kernel/system.hpp"

namespace ps2 {
    void wait_input_clear() {
        while (inb(PS2_STATUS_PORT) & 0x02);
    }

    void wait_output_full() {
        while (!(inb(PS2_STATUS_PORT) & 0x01));
    }

    void init() {
        // Disable both ports
        wait_input_clear();
        outb(PS2_STATUS_PORT, 0xAD);
        wait_input_clear();
        outb(PS2_STATUS_PORT, 0xA7);

        // Flush output buffer
        while (inb(PS2_STATUS_PORT) & 1) inb(PS2_DATA_PORT);

        // Get config byte
        wait_input_clear();
        outb(PS2_STATUS_PORT, 0x20);
        wait_output_full();
        uint8_t config = inb(PS2_DATA_PORT);

        // Modify config byte
        config &= ~0x40; // disable translation
        config |= 0x01; // enable IRQ1
        config |= 0x04; // enable port1 clock

        // Write config
        wait_input_clear();
        outb(PS2_STATUS_PORT, 0x60);
        wait_input_clear();
        outb(PS2_DATA_PORT, config);

        // Controller self-test
        wait_input_clear();
        outb(PS2_STATUS_PORT, 0xAA);
        wait_output_full();
        if (inb(PS2_DATA_PORT) != 0x55) {
            // TODO: handle fail
        }

        // Enable port
        wait_input_clear();
        outb(PS2_STATUS_PORT, 0xAE);

        // Reset keyboard
        wait_input_clear();
        outb(PS2_DATA_PORT, 0xFF);
        wait_output_full();
        inb(PS2_DATA_PORT);
        wait_output_full();
        inb(PS2_DATA_PORT);

        // Enable scanning
        wait_input_clear();
        outb(PS2_DATA_PORT, 0xF4);
        wait_output_full();
        inb(PS2_DATA_PORT);
    }
}
