#pragma once

#include <cstdarg>

char* vformat(const char* format, va_list ap);
int vformat(char** out, const char* format, va_list ap);
int simple_vsprintf(char** out, const char* format, va_list ap);

__attribute__ ((format (printf, 1, 2)))
char* format(const char* fmt, ...);