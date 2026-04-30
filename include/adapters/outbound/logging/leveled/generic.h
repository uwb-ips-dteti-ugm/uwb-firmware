#pragma once

#include <cstdint>
#include <Arduino.h>
#include "ports/outbound/logging/leveled.h"

namespace ao::logging
{
    enum LogLevel : uint8_t
    {
        LOG_LEVEL_NONE,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_WARN,
        LOG_LEVEL_INFO,
        LOG_LEVEL_DEBUG
    };

    class LeveledGeneric : public po::logging::Leveled
    {
    public:
        LeveledGeneric(HardwareSerial *serial, LogLevel level);
        ~LeveledGeneric() override = default;
        LeveledGeneric(const LeveledGeneric &) = delete;
        LeveledGeneric &operator=(const LeveledGeneric &) = delete;
        void error(const char *tag, const char *message, ...) override;
        void warn(const char *tag, const char *message, ...) override;
        void info(const char *tag, const char *message, ...) override;
        void debug(const char *tag, const char *message, ...) override;

    private:
        HardwareSerial *serial;
        LogLevel level;

        static void log(HardwareSerial *serial, const char *level, const char *tag, const char *message, va_list args);
    };
}