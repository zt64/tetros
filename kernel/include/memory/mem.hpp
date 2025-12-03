#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy(void* dst_ptr, const void* src_ptr, size_t size);
void* memset(void* dst_ptr, uint8_t val, size_t count);
void* memmove(void* dst_ptr, const void* src_ptr, size_t size);
int32_t memcmp(const void* a, const void* b, size_t n);
void *memcpy_fast(void *dst_ptr, const void *src_ptr, size_t n);
void *memmove_fast(void *dst_ptr, const void *src_ptr, size_t n);
void *memset_fast(void *dst_ptr, uint8_t val, size_t n);

void* malloc(size_t size);

void free(void* block);

void* realloc(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif

namespace mem {
    // Initialize the physical memory manager
    void init_pmm();

    // Allocate a single 4KB physical page (returns physical address)
    // The page is automatically zeroed
    void* allocate_physical_page();

    // Allocate multiple contiguous physical pages
    // Returns nullptr if not enough contiguous memory available
    void* allocate_physical_pages(size_t count);

    // Free a single physical page
    void free_physical_page(void* phys_addr);

    // Free multiple contiguous physical pages
    void free_physical_pages(void* phys_addr, size_t count);

    // Get memory statistics
    size_t get_free_pages();
    size_t get_used_pages();
    size_t get_total_pages();

    size_t get_free_memory();  // In bytes
    size_t get_used_memory();  // In bytes
    size_t get_total_memory(); // In bytes
}
