#pragma once

namespace ports::driven::logger
{
    class Leveled
    {
    public:
        virtual ~Leveled() = default;
        virtual void error(const char *tag, const char *message, ...) = 0;
        virtual void warn(const char *tag, const char *message, ...) = 0;
        virtual void info(const char *tag, const char *message, ...) = 0;
        virtual void debug(const char *tag, const char *message, ...) = 0;
    };
}