#pragma once

#include <cstddef>
#include <cstdint>

void* memcpy(void* dest, const void* src, size_t size);

void* memset(void* dest, uint8_t val, size_t count);

void *memcpy_fast(void *dest, const void *src, size_t n);
void *memmove_fast(void *dest, const void *src, size_t n);

void* memmove(void* dst_ptr, const void* src_ptr, size_t size);

void* malloc(size_t size);

void free(void* block);

void* realloc(void* ptr, std::size_t size);
