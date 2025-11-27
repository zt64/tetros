#pragma once

#include <cstdint>

constexpr uint64_t PAGE_SIZE = 4096;

namespace paging {
    void init();
};