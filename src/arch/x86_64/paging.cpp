#include "kernel/paging.hpp"

extern "C" void set_up_page_tables();

namespace paging {
    constexpr uint64_t PAGE_SIZE = 0x1000;

    constexpr uint64_t PTE_PRESENT = 1ull << 0;
    constexpr uint64_t PTE_WRITE = 1ull << 1;

    constexpr size_t PAGE_TABLE_ENTRIES = 512;

    uint64_t* pml4_table = nullptr;

    void init() {
        set_up_page_tables(); // TODO: Setup page tables in C++ (once i figure out how)

        for (int i = 0; i < 512; i++) {
        }

        asm volatile("mov %0, %%cr3" : : "r"(pml4_table) : "memory");
    }
}
