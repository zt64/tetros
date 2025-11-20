#include "lib/format.hpp"

#include <cstdarg>
#include <cwchar>

#include "driver/serial.hpp"
#include "lib/mem.hpp"
#include "lib/string.hpp"

int isdigit(const int c) {
    return ((c >= '0') && (c <= '9'));
}

// https://wiki.osdev.org/User:A22347/Printf
char* int_str(
    intmax_t i,
    char b[],
    int base,
    const bool plusSignIfNeeded,
    const bool spaceSignIfNeeded,
    const int paddingNo,
    const bool justify,
    const bool zeroPad
) {
    char digit[32] = {};
    memset(digit, 0, 32);
    strcpy(digit, "0123456789");

    if (base == 16) {
        strcat(digit, "ABCDEF");
    } else if (base == 17) {
        strcat(digit, "abcdef");
        base = 16;
    }

    char* p = b;
    if (i < 0) {
        *p++ = '-';
        i *= -1;
    } else if (plusSignIfNeeded) {
        *p++ = '+';
    } else if (spaceSignIfNeeded) {
        *p++ = ' ';
    }

    intmax_t shifter = i;
    do {
        ++p;
        shifter = shifter / base;
    }
    while (shifter);

    *p = '\0';
    do {
        *--p = digit[i % base];
        i = i / base;
    }
    while (i);

    int padding = paddingNo - static_cast<int>(strlen(b));
    if (padding < 0) padding = 0;

    if (justify) {
        while (padding--) {
            if (zeroPad) {
                b[strlen(b)] = '0';
            } else {
                b[strlen(b)] = ' ';
            }
        }
    } else {
        char a[256] = {};
        while (padding--) {
            if (zeroPad) {
                a[strlen(a)] = '0';
            } else {
                a[strlen(a)] = ' ';
            }
        }
        strcat(a, b);
        strcpy(b, a);
    }

    return b;
}

void displayCharacter(const char c, int* a) {
    serial::putchar(c);
    *a += 1;
}

void displayString(const char* c, int* a) {
    for (int i = 0; c[i]; ++i) {
        displayCharacter(c[i], a);
    }
}

void appendCharacter(const char c, char* buffer, int* pos) {
    if (*pos < 1023) {
        // Leave room for null terminator
        buffer[*pos] = c;
        *pos += 1;
    }
}

void appendString(const char* c, char* buffer, int* pos) {
    if (!c) {
        c = "(null)";
    }
    for (int i = 0; c[i]; ++i) {
        appendCharacter(c[i], buffer, pos);
    }
}

char* vformat(const char* format, va_list args) {
    static char outputBuffer[1024];
    int pos = 0;
    char intStrBuffer[256] = {};

    for (int i = 0; format[i]; ++i) {
        char length = '\0';

        int lengthSpec = 0;
        int precSpec = 0;
        bool leftJustify = false;
        bool zeroPad = false;
        bool spaceNoSign = false;
        bool altForm = false;
        bool plusSign = false;
        bool emode = false;
        int expo = 0;
        char specifier = '\0';

        if (format[i] == '%') {
            ++i;

            if (!format[i]) {
                appendCharacter('%', outputBuffer, &pos);
                break;
            }

            bool extBreak = false;
            while (format[i]) {
                switch (format[i]) {
                    case '-':
                        leftJustify = true;
                        ++i;
                        break;

                    case '+':
                        plusSign = true;
                        ++i;
                        break;

                    case '#':
                        altForm = true;
                        ++i;
                        break;

                    case ' ':
                        spaceNoSign = true;
                        ++i;
                        break;

                    case '0':
                        zeroPad = true;
                        ++i;
                        break;

                    default:
                        extBreak = true;
                        break;
                }

                if (extBreak) break;
            }

            if (!format[i]) break;

            while (isdigit(format[i])) {
                lengthSpec *= 10;
                lengthSpec += format[i] - 48;
                ++i;
            }

            if (format[i] == '*') {
                lengthSpec = va_arg(args, int);
                ++i;
                if (!format[i]) break;
            }

            if (format[i] == '.') {
                ++i;
                if (!format[i]) break;
                while (isdigit(format[i])) {
                    precSpec *= 10;
                    precSpec += format[i] - 48;
                    ++i;
                    if (!format[i]) break;
                }

                if (!format[i]) break;
                if (format[i] == '*') {
                    precSpec = va_arg(args, int);
                    ++i;
                    if (!format[i]) break;
                }
            } else {
                precSpec = 6;
            }

            if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' ||
                format[i] == 'z' || format[i] == 't' || format[i] == 'L') {
                length = format[i];
                ++i;
                if (format[i] == 'h') {
                    length = 'H';
                } else if (format[i] == 'l') {
                    length = 'q';
                    ++i;
                }
            }
            specifier = format[i];

            memset(intStrBuffer, 0, 256);

            int base = 10;
            if (specifier == 'o') {
                base = 8;
                specifier = 'u';
                if (altForm) {
                    appendString("0", outputBuffer, &pos);
                }
            }
            if (specifier == 'p') {
                base = 16;
                length = 'z';
                specifier = 'u';
            }
            switch (specifier) {
                case 'X':
                    base = 16;
                case 'x':
                    base = base == 10 ? 17 : base;
                    if (altForm) {
                        appendString("0x", outputBuffer, &pos);
                    }

                case 'u': {
                    switch (length) {
                        case 0: {
                            const unsigned int integer = va_arg(args, unsigned int);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'H': {
                            const auto integer = static_cast<unsigned char>(va_arg(args, unsigned int));
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'h': {
                            const auto integer = static_cast<unsigned short int>(va_arg(args, unsigned int));
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'l': {
                            const uint64_t integer = va_arg(args, uint64_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'q': {
                            uintmax_t integer = va_arg(args, unsigned long long);
                            int_str(
                                static_cast<intmax_t>(integer),
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'j': {
                            const uintmax_t integer = va_arg(args, uintmax_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'z': {
                            const size_t integer = va_arg(args, size_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 't': {
                            const ptrdiff_t integer = va_arg(args, ptrdiff_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }

                case 'd':
                case 'i': {
                    switch (length) {
                        case 0: {
                            const int integer = va_arg(args, int);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'H': {
                            const auto integer = static_cast<signed char>(va_arg(args, int));
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'h': {
                            const auto integer = static_cast<short int>(va_arg(args, int));
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'l': {
                            const long integer = va_arg(args, long);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'q': {
                            long long integer = va_arg(args, long long);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'j': {
                            const intmax_t integer = va_arg(args, intmax_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 'z': {
                            const size_t integer = va_arg(args, size_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        case 't': {
                            const ptrdiff_t integer = va_arg(args, ptrdiff_t);
                            int_str(
                                integer,
                                intStrBuffer,
                                base,
                                plusSign,
                                spaceNoSign,
                                lengthSpec,
                                leftJustify,
                                zeroPad
                            );
                            appendString(intStrBuffer, outputBuffer, &pos);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }

                case 'c': {
                    if (length == 'l') {
                        appendCharacter(va_arg(args, wint_t), outputBuffer, &pos);
                    } else {
                        appendCharacter(va_arg(args, int), outputBuffer, &pos);
                    }

                    break;
                }

                case 's': {
                    appendString(va_arg(args, char*), outputBuffer, &pos);
                    break;
                }

                case 'n': {
                    switch (length) {
                        case 'H':
                            *(va_arg(args, signed char*)) = static_cast<signed char>(pos);
                            break;
                        case 'h':
                            *(va_arg(args, short int*)) = static_cast<short int>(pos);
                            break;

                        case 0: {
                            int* a = va_arg(args, int*);
                            *a = pos;
                            break;
                        }

                        case 'l':
                            *(va_arg(args, long*)) = pos;
                            break;
                        case 'q':
                            *(va_arg(args, long long*)) = pos;
                            break;
                        case 'j':
                            *(va_arg(args, intmax_t*)) = pos;
                            break;
                        case 'z':
                            *(va_arg(args, size_t*)) = pos;
                            break;
                        case 't':
                            *(va_arg(args, ptrdiff_t*)) = pos;
                            break;
                        default:
                            break;
                    }
                    break;
                }

                case 'e':
                case 'E':
                    emode = true;

                case 'a':
                case 'A':
                    break;

                default:
                    break;
            }

            if (specifier == 'e') {
                appendString("e+", outputBuffer, &pos);
            } else if (specifier == 'E') {
                appendString("E+", outputBuffer, &pos);
            }

            if (specifier == 'e' || specifier == 'E') {
                int_str(expo, intStrBuffer, 10, false, false, 2, false, true);
                appendString(intStrBuffer, outputBuffer, &pos);
            }
        } else {
            appendCharacter(format[i], outputBuffer, &pos);
        }
    }

    outputBuffer[pos] = '\0';
    return outputBuffer;
}

__attribute__ ((format (printf, 1, 2)))
const char* format(const char* format, ...) {
    va_list list;
    va_start(list, format);
    const char* str = vformat(format, list);
    va_end(list);
    return str;
}
