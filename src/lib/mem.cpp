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

constexpr size_t kWordSize = sizeof(uint64_t);

inline void copy_forward_align(
    uint8_t*&dst,
    const uint8_t*&src,
    size_t&remaining
) {
    while (remaining != 0 &&
           (reinterpret_cast<uintptr_t>(dst) & (kWordSize - 1)) != 0) {
        *dst++ = *src++;
        --remaining;
    }
}

inline void copy_backward_align(
    uint8_t*&dst,
    const uint8_t*&src,
    size_t&remaining
) {
    while (remaining != 0 &&
           (reinterpret_cast<uintptr_t>(dst) & (kWordSize - 1)) != 0) {
        --dst;
        --src;
        *dst = *src;
        --remaining;
    }
}

void* memcpy_fast(void* dest, const void* src, size_t n) {
    if (n == 0 || dest == src) {
        return dest;
    }

    if (n < 32) {
        return memcpy(dest, src, n);
    }

    auto* d = static_cast<uint8_t *>(dest);
    auto* s = static_cast<const uint8_t *>(src);
    size_t remaining = n;

    copy_forward_align(d, s, remaining);

    while (remaining >= kWordSize * 4) {
        auto* dst64 = reinterpret_cast<uint64_t *>(d);
        auto* src64 = reinterpret_cast<const uint64_t *>(s);
        dst64[0] = src64[0];
        dst64[1] = src64[1];
        dst64[2] = src64[2];
        dst64[3] = src64[3];
        d += kWordSize * 4;
        s += kWordSize * 4;
        remaining -= kWordSize * 4;
    }

    while (remaining >= kWordSize) {
        *reinterpret_cast<uint64_t *>(d) = *reinterpret_cast<const uint64_t *>(s);
        d += kWordSize;
        s += kWordSize;
        remaining -= kWordSize;
    }

    while (remaining != 0) {
        *d++ = *s++;
        --remaining;
    }

    return dest;
}

void* memmove_fast(void* dest, const void* src, size_t n) {
    if (n == 0 || dest == src) {
        return dest;
    }

    auto* d = static_cast<uint8_t *>(dest);
    auto* s = static_cast<const uint8_t *>(src);

    if (d < s) {
        return memcpy_fast(dest, src, n);
    }

    if (n < 32) {
        return memmove(dest, src, n);
    }

    size_t remaining = n;
    auto* d_end = d + n;
    auto* s_end = s + n;

    copy_backward_align(d_end, s_end, remaining);

    while (remaining >= kWordSize * 4) {
        d_end -= kWordSize * 4;
        s_end -= kWordSize * 4;
        auto* dst64 = reinterpret_cast<uint64_t *>(d_end);
        auto* src64 = reinterpret_cast<const uint64_t *>(s_end);
        dst64[3] = src64[3];
        dst64[2] = src64[2];
        dst64[1] = src64[1];
        dst64[0] = src64[0];
        remaining -= kWordSize * 4;
    }

    while (remaining >= kWordSize) {
        d_end -= kWordSize;
        s_end -= kWordSize;
        *reinterpret_cast<uint64_t *>(d_end) =
                *reinterpret_cast<const uint64_t *>(s_end);
        remaining -= kWordSize;
    }

    while (remaining != 0) {
        --d_end;
        --s_end;
        *d_end = *s_end;
        --remaining;
    }

    return dest;
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
