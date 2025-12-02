#pragma once

#include <stddef.h>
#include <stdarg.h>

#include "memory/mem.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define uacpi_memcpy     memcpy_fast
#define uacpi_memmove    memmove_fast

#ifdef __cplusplus
}
#endif

