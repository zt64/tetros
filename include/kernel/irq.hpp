#pragma once

#include "system.hpp"

typedef void (*irq_handler)(regs* r);

void irq_install_handler(uint32_t irq, irq_handler handler);

void irq_uninstall_handler(uint32_t irq);

void irq_init();
