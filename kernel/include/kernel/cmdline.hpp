#pragma once
#include "lib/log.hpp"

struct Config {
    LogLevel log_level;
};

void parse_cmdline(char* cmdline);