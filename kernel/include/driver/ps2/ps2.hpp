#pragma once

#define PS2_DATA_PORT    0x60
#define PS2_COMMAND_PORT 0x64
#define PS2_STATUS_PORT  0x64

namespace ps2 {
    void init();
}