#include "adapters/outbound/logging/leveled/generic.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <sys/time.h>

namespace ao::logging
{
    LeveledGeneric::LeveledGeneric(HardwareSerial *serial, LogLevel level) : serial(serial), level(level) {}

    void LeveledGeneric::error(const char *tag, const char *message, ...)
    {
        if (level < LOG_LEVEL_ERROR)
            return;
        va_list args;
        va_start(args, message);
        log(serial, "ERROR", tag, message, args);
        va_end(args);
    }

    void LeveledGeneric::warn(const char *tag, const char *message, ...)
    {
        if (level < LOG_LEVEL_WARN)
            return;
        va_list args;
        va_start(args, message);
        log(serial, "WARN", tag, message, args);
        va_end(args);
    }

    void LeveledGeneric::info(const char *tag, const char *message, ...)
    {
        if (level < LOG_LEVEL_INFO)
            return;
        va_list args;
        va_start(args, message);
        log(serial, "INFO", tag, message, args);
        va_end(args);
    }

    void LeveledGeneric::debug(const char *tag, const char *message, ...)
    {
        if (level < LOG_LEVEL_DEBUG)
            return;
        va_list args;
        va_start(args, message);
        log(serial, "DEBUG", tag, message, args);
        va_end(args);
    }

    void LeveledGeneric::log(HardwareSerial *serial, const char *level, const char *tag, const char *message, va_list args)
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        struct tm *t = localtime(&tv.tv_sec);
        char ts[24];
        snprintf(ts, sizeof(ts), "%02d/%02d/%04d %02d:%02d:%02d.%03ld",
                 t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                 t->tm_hour, t->tm_min, t->tm_sec,
                 tv.tv_usec / 1000);
        char buf[512];
        vsnprintf(buf, sizeof(buf), message, args);
        serial->printf("%s [%s] [%s] %s\n", ts, level, tag ? tag : "", buf);
    }
}
