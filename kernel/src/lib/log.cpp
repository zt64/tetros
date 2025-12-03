#include "lib/log.hpp"

#include <cstdarg>

#include "driver/screen.hpp"
#include "driver/serial.hpp"
#include "lib/format.hpp"
#include "memory/mem.hpp"

#define LOG_LINE_MAX    256
#define LOG_MAX_LINES   512

static const char* level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const uint32_t level_colors[] = {
    0x00FFFF, // DEBUG - cyan
    0x00FF00, // INFO  - green
    0xFFFF00, // WARN  - yellow
    0xFF0000, // ERROR - red
    0xFF00FF  // FATAL - magenta
};

struct LogEntry {
    uint64_t timestamp;
    LogLevel level;
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

int line = 0;
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

    if (serial::available()) {
        serial::printf("%s[%s] %s\033[0m\n", prefix, level_names[level], body);
    }

    const char* formatted = format("[%s] %s\n", level_names[level], body);

    screen::draw(formatted, 0, line++ * 8 * 1, 1, level_colors[level]);
    screen::flush();

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
