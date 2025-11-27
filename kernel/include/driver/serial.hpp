#pragma once

#define PORT 0x3F8 // COM1

namespace serial {
    /**
     *
     * @return 0 if successful, 1 if faulty
     */
    int init();

    bool received();

    char read();

    bool transmitted();

    void putchar(char c);

    void print(const char* str);

    __attribute__ ((format (printf, 1, 2)))
    void printf(const char* fmt, ...);
}
