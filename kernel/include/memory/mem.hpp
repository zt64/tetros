#pragma once

#include <cstddef>
#include <cstdint>

void* memcpy(void* dst_ptr, const void* src_ptr, size_t size);

void* memset(void* dst_ptr, uint8_t val, size_t count);

void *memcpy_fast(void *dst_ptr, const void *src_ptr, size_t n);
void *memmove_fast(void *dst_ptr, const void *src_ptr, size_t n);
void *memset_fast(void *dst_ptr, uint8_t val, size_t n);

void* memmove(void* dst_ptr, const void* src_ptr, size_t size);

void* malloc(size_t size);

void free(void* block);

void* realloc(void* ptr, std::size_t size);
