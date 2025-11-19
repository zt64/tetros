#include "lib/mem.hpp"

#include <cstddef>
#include <cstdint>
#include <driver/serial.hpp>

union alignas(16) Block {
    struct {
        size_t size; // Size of the block
        unsigned is_free; // Block status flag
        union Block* next; // Next block in list
    } s;

    intptr_t* data[1];
};

static Block* heapStart = nullptr;
static auto top = heapStart;

inline size_t align(const size_t n) {
    return (n + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
}


void* memcpy(void* dstptr, const void* src_ptr, const size_t size) {
    const auto dst = static_cast<unsigned char *>(dstptr);
    const auto src = static_cast<const unsigned char *>(src_ptr);
    for (size_t i = 0; i < size; i++) dst[i] = src[i];
    return dstptr;
}

void* memset(void* dest, const uint8_t val, const size_t count) {
    auto* p = static_cast<uint8_t *>(dest);
    for (size_t i = 0; i < count; i++) {
        p[i] = static_cast<uint8_t>(val);
    }
    return dest;
}

void* memmove(void* dst_ptr, const void* src_ptr, const size_t size) {
    const auto dst = static_cast<unsigned char *>(dst_ptr);
    const auto src = static_cast<const unsigned char *>(src_ptr);
    if (dst < src) {
        for (size_t i = 0; i < size; i++) dst[i] = src[i];
    } else {
        for (size_t i = size; i != 0; i--) dst[i - 1] = src[i - 1];
    }
    return dst_ptr;
}

void* malloc(const size_t size) {
    if (!size) return nullptr;
    // TODO: Implement memory deallocation
    return nullptr;
}

void free(void* block) {
    if (!block) return; // Already freed
    // TODO: Implement memory deallocation
    (void)block;
}

void* realloc(void* ptr, const size_t size) {
    // TODO: Implement memory reallocation
    (void)ptr;
    (void)size;
    return nullptr;
}
