#include "driver/serial.hpp"
#include "kernel/system.hpp"
#include "lib/format.hpp"
static int simple_vsprintf(char** out, const char* format, va_list ap);

namespace serial {
    int init() {
        outb(PORT + 1, 0x00); // Disable all interrupts
        outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, 0x00); //                  (hi byte)
        outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
        outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
        outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
        outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)
        // Check if serial is faulty (i.e: not same byte as sent)
        if (inb(PORT + 0) != 0xAE) {
            return 1;
        }

        // If serial is not faulty set it in normal operation mode
        // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
        outb(PORT + 4, 0x0F);

        return 0;
    }

    bool received() {
        return inb(PORT + 5) & 1;
    }

    char read() {
        while (received() == 0) {
        }

        return inb(PORT);
    }

    bool transmitted() {
        return inb(PORT + 5) & 0x20;
    }

    void putchar(const char c) {
        while (transmitted() == false) {
        }

        outb(PORT, c);
    }

    void print(const char* str) {
        if (!str) return;
        while (*str) {
            putchar(*str++);
        }
    }

    void printf(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        simple_vsprintf(nullptr, fmt, ap);
        va_end(ap);
    }
}

static void simple_outputchar(char** str, const char c) {
    serial::putchar(c);
}

enum flags {
    PAD_ZERO = 1,
    PAD_RIGHT = 2,
};

static int prints(char** out, const char* string, int width, int flags) {
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char* ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (flags & PAD_ZERO)
            padchar = '0';
    }
    if (!(flags & PAD_RIGHT)) {
        for (; width > 0; --width) {
            simple_outputchar(out, padchar);
            ++pc;
        }
    }
    for (; *string; ++string) {
        simple_outputchar(out, *string);
        ++pc;
    }
    for (; width > 0; --width) {
        simple_outputchar(out, padchar);
        ++pc;
    }

    return pc;
}

#define PRINT_BUF_LEN 64

static int simple_outputi(
    char** out,
    const long long i,
    const int base,
    const int sign,
    int width,
    const int flags,
    const int letbase
) {
    char print_buf[PRINT_BUF_LEN];
    int neg = 0, pc = 0;
    unsigned long long u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints(out, print_buf, width, flags);
    }

    if (sign && base == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    char* s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    while (u) {
        int t = u % base;
        if (t >= 10)
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= base;
    }

    if (neg) {
        if (width && (flags & PAD_ZERO)) {
            simple_outputchar(out, '-');
            ++pc;
            --width;
        } else {
            *--s = '-';
        }
    }

    return pc + prints(out, s, width, flags);
}

static int simple_vsprintf(char** out, const char* format, va_list ap) {
    int width, flags;
    int pc = 0;
    char scr[2];
    union {
        char c;
        char* s;
        int i;
        unsigned int u;
        long li;
        unsigned long lu;
        long long lli;
        unsigned long long llu;
        short hi;
        unsigned short hu;
        signed char hhi;
        unsigned char hhu;
        void* p;
    } u{};

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = flags = 0;
            if (*format == '\0')
                break;
            if (*format == '%')
                goto out;
            if (*format == '-') {
                ++format;
                flags = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                flags |= PAD_ZERO;
            }
            if (*format == '*') {
                width = va_arg(ap, int);
                format++;
            } else {
                for (; *format >= '0' && *format <= '9'; ++format) {
                    width *= 10;
                    width += *format - '0';
                }
            }
            switch (*format) {
                case 'd':
                    u.i = va_arg(ap, int);
                    pc += simple_outputi(out, u.i, 10, 1, width, flags, 'a');
                    break;

                case 'u':
                    u.u = va_arg(ap, unsigned int);
                    pc += simple_outputi(out, u.u, 10, 0, width, flags, 'a');
                    break;

                case('x'):
                    u.u = va_arg(ap, unsigned int);
                    pc += simple_outputi(out, u.u, 16, 0, width, flags, 'a');
                    break;

                case('X'):
                    u.u = va_arg(ap, unsigned int);
                    pc += simple_outputi(out, u.u, 16, 0, width, flags, 'A');
                    break;

                case 'p':
                    u.u = va_arg(ap, uint64_t);
                    pc += simple_outputi(out, u.u, 16, 0, width, flags, 'A');
                    break;

                case('c'):
                    u.c = va_arg(ap, int);
                    scr[0] = u.c;
                    scr[1] = '\0';
                    pc += prints(out, scr, width, flags);
                    break;

                case('s'):
                    u.s = va_arg(ap, char *);
                    pc += prints(out, u.s ? u.s : "(null)", width, flags);
                    break;
                case('l'):
                    ++format;
                    switch (*format) {
                        case('d'):
                            u.li = va_arg(ap, long);
                            pc += simple_outputi(out, u.li, 10, 1, width, flags, 'a');
                            break;

                        case('u'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += simple_outputi(out, u.lu, 10, 0, width, flags, 'a');
                            break;

                        case('x'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += simple_outputi(out, u.lu, 16, 0, width, flags, 'a');
                            break;

                        case('X'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += simple_outputi(out, u.lu, 16, 0, width, flags, 'A');
                            break;

                        case('l'):
                            ++format;
                            switch (*format) {
                                case'd':
                                    u.lli = va_arg(ap, long long);
                                    pc += simple_outputi(out, u.lli, 10, 1, width, flags, 'a');
                                    break;

                                case'u':
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += simple_outputi(out, u.llu, 10, 0, width, flags, 'a');
                                    break;

                                case'x':
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += simple_outputi(out, u.llu, 16, 0, width, flags, 'a');
                                    break;

                                case'X':
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += simple_outputi(out, u.llu, 16, 0, width, flags, 'A');
                                    break;

                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case('h'):
                    ++format;
                    switch (*format) {
                        case('d'):
                            u.hi = va_arg(ap, int);
                            pc += simple_outputi(out, u.hi, 10, 1, width, flags, 'a');
                            break;

                        case('u'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += simple_outputi(out, u.lli, 10, 0, width, flags, 'a');
                            break;

                        case('x'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'a');
                            break;

                        case('X'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'A');
                            break;

                        case('h'):
                            ++format;
                            switch (*format) {
                                case('d'):
                                    u.hhi = va_arg(ap, int);
                                    pc += simple_outputi(out, u.hhi, 10, 1, width, flags, 'a');
                                    break;

                                case('u'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += simple_outputi(out, u.lli, 10, 0, width, flags, 'a');
                                    break;

                                case('x'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'a');
                                    break;

                                case('X'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'A');
                                    break;

                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        } else {
        out:
            simple_outputchar(out, *format);
            ++pc;
        }
    }
    return pc;
}
