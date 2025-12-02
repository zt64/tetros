#include "uacpi/kernel_api.h"
#include "driver/limine/limine_requests.hpp"
#include "lib/log.hpp"
#include "kernel/system.hpp"

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr* out_rsdp_address) {
    if (!out_rsdp_address) return UACPI_STATUS_INVALID_ARGUMENT;
    if (!limine_requests::rsdp_request.response) return UACPI_STATUS_NOT_FOUND;

    const auto limine_addr = reinterpret_cast<uintptr_t>(limine_requests::rsdp_request.response->address);
    uint64_t phys = limine_addr;

    if (limine_requests::hhdm_request.response) {
        const uint64_t hhdm = limine_requests::hhdm_request.response->offset;
        if (limine_addr >= hhdm) {
            phys = limine_addr - hhdm;
        }
    }

    *out_rsdp_address = phys;
    return UACPI_STATUS_OK;
}

void* uacpi_kernel_map(const uacpi_phys_addr addr, const uacpi_size len) {
    if (!limine_requests::hhdm_request.response) return nullptr;
    const uint64_t hhdm = limine_requests::hhdm_request.response->offset;

    auto phys = static_cast<uint64_t>(addr);
    if (phys >= hhdm) phys = phys - hhdm;
    const uacpi_phys_addr phys_addr = phys;

    const uacpi_phys_addr page = phys_addr & ~static_cast<uacpi_phys_addr>(0xFFFULL);
    const size_t offset = phys_addr - page;

    const uintptr_t virt_page = hhdm + page;
    return reinterpret_cast<void*>(virt_page + offset);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void uacpi_kernel_unmap(void* addr, const uacpi_size len) {
    (void)addr; (void)len;
    // No-op
}

void uacpi_kernel_log(const uacpi_log_level level, const uacpi_char* msg) {
    const auto m = reinterpret_cast<const char*>(msg ? msg : "");
    switch (level) {
        case UACPI_LOG_DEBUG:
            logger.debug("uACPI: %s", m);
            break;
        case UACPI_LOG_TRACE:
            logger.debug("uACPI(trace): %s", m);
            break;
        case UACPI_LOG_INFO:
            logger.info("uACPI: %s", m);
            break;
        case UACPI_LOG_WARN:
            logger.warn("uACPI: %s", m);
            break;
        case UACPI_LOG_ERROR:
            logger.error("uACPI: %s", m);
            break;
        default:
            logger.info("uACPI: %s", m);
    }
}
