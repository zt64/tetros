#pragma once
#include <cstdarg>

enum LogLevel {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
};

class Logger {
public:
    explicit Logger(const LogLevel initial = LOG_LEVEL_DEBUG) : log_level(initial) {
    }
    void vlog(LogLevel level, const char* fmt, va_list ap) const;
    void log(LogLevel level, const char* fmt, ...) const;

    void debug(const char* fmt, ...) const;
    void info(const char* fmt, ...) const;
    void warn(const char* fmt, ...) const;
    void error(const char* fmt, ...) const;
    void fatal(const char* fmt, ...) const;

private:
    LogLevel log_level;
};

inline Logger logger(LOG_LEVEL_DEBUG);
