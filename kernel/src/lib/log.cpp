#include "lib/log.hpp"

#include <cstdarg>

#include "driver/serial.hpp"
#include "lib/format.hpp"
#include "memory/mem.hpp"

#define LOG_LINE_MAX    256
#define LOG_MAX_LINES   512

static const char* level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

struct LogEntry {
    uint64_t timestamp;
    uint8_t  level;
    uint16_t len;
    char     payload[LOG_LINE_MAX];
} __attribute__((packed, aligned(8)));

static LogEntry log_buffer[LOG_MAX_LINES];
static uint32_t log_buffer_head = 0;
static uint32_t log_buffer_tail = 0;

static void push_log_record(const LogLevel level, const char*) {
    uint32_t pos = log_buffer_head + 1;
    uint32_t log_index = log_buffer_head + 1 & (LOG_MAX_LINES - 1);
}

void Logger::vlog(const LogLevel level, const char* fmt, va_list ap) const {
    if (level < log_level) return;

    char* body = vformat(fmt, ap);
    if (!body) return;

    const char* prefix;
    switch (level) {
        case LOG_LEVEL_DEBUG: prefix = "\033[36m"; break; // cyan
        case LOG_LEVEL_INFO:  prefix = "\033[32m"; break; // green
        case LOG_LEVEL_WARN:  prefix = "\033[33m"; break; // yellow
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_FATAL: prefix = "\033[31m"; break; // red
        default: prefix = ""; break;
    }

    serial::printf("%s[%s] %s\033[0m\n", prefix, level_names[level], body);

    free(body);
}

void Logger::log(const LogLevel level, const char* fmt, ...) const {
    va_list ap;
    va_start(ap, fmt);
    vlog(level, fmt, ap);
    va_end(ap);
}

void Logger::debug(const char* fmt, ...) const {
    va_list ap;
    va_start(ap, fmt);
    vlog(LOG_LEVEL_DEBUG, fmt, ap);
    va_end(ap);
}

void Logger::info(const char* fmt, ...) const {
    va_list ap;
    va_start(ap, fmt);
    vlog(LOG_LEVEL_INFO, fmt, ap);
    va_end(ap);
}

void Logger::warn(const char* fmt, ...) const {
    va_list ap;
    va_start(ap, fmt);
    vlog(LOG_LEVEL_WARN, fmt, ap);
    va_end(ap);
}

void Logger::error(const char* fmt, ...) const {
    va_list ap;
    va_start(ap, fmt);
    vlog(LOG_LEVEL_ERROR, fmt, ap);
    va_end(ap);
}

void Logger::fatal(const char* fmt, ...) const {
    va_list ap;
    va_start(ap, fmt);
    vlog(LOG_LEVEL_FATAL, fmt, ap);
    va_end(ap);
}
