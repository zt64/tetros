#pragma once

#include <cstdarg>

const char* format(const char* format, ...);

char* vformat(const char* format, va_list args);