#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <asm-generic/errno-base.h>

#include "driver/acpi.hpp"
#include "lib/log.hpp"

namespace acpi {
    int init() {
        const uacpi_status ret = uacpi_setup_early_table_access(tables, sizeof(tables));
        if (uacpi_unlikely_error(ret)) {
            logger.error("uacpi_setup_early_table_access error: %s", uacpi_status_to_string(ret));
            return -ENODEV;
        }

        return 0;
    }
}
