#include "kernel/paging.hpp"
#include "driver/serial.hpp"
#include "driver/limine/limine.h"
#include "driver/limine/limine_requests.hpp"
#include "lib/mem.hpp"

namespace paging {
    constexpr uint64_t PAGE_SIZE = 0x1000;

    constexpr uint64_t PTE_PRESENT = 1ull << 0;
    constexpr uint64_t PTE_WRITE = 1ull << 1;
    constexpr uint64_t PTE_PS = 1ull << 7;

    constexpr size_t PAGE_TABLE_ENTRIES = 512;

    uint64_t g_kernel_phys_base = 0;
    uint64_t g_kernel_virt_base = 0;
    uint64_t g_kernel_size = 0;
    uint64_t g_cr3_value = 0;

    uint64_t* pml4_table = nullptr;

    extern "C" char kernel_start[];
    extern "C" char kernel_end[];

    static uint64_t virt_to_phys(void* v) {
        return g_kernel_phys_base + (reinterpret_cast<uint64_t>(v) - g_kernel_virt_base);
    }

    void init() {
        g_kernel_phys_base = executable_addr_request.response->physical_base;
        g_kernel_virt_base = reinterpret_cast<uint64_t>(&kernel_start);
        g_cr3_value = virt_to_phys(pml4_table);

        asm volatile("mov %0, %%cr3" : : "r"(g_cr3_value) : "memory");
    }
}
