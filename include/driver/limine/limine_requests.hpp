#pragma once

#include "driver/limine/limine.h"

extern volatile uint64_t limine_base_revision[];
extern volatile limine_framebuffer_request framebuffer_request;
extern volatile limine_executable_address_request executable_addr_request;
extern volatile limine_memmap_request memmap_request;
extern volatile limine_hhdm_request hhdm_request;