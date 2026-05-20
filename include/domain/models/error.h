#pragma once

#include <cstdint>

namespace models
{
    enum class Error : uint8_t
    {
        Ok,
        InvalidArgument,
        MemoryAllocation,
        SystemFail,
        BadState,
        Unimplemented,
        Unknown
    };

    inline const char *errorToString(Error err)
    {
        switch (err)
        {
        case Error::Ok:
            return "Ok";
        case Error::InvalidArgument:
            return "Invalid argument";
        case Error::MemoryAllocation:
            return "Memory allocation";
        case Error::SystemFail:
            return "System failure";
        case Error::BadState:
            return "Bad state";
        case Error::Unimplemented:
            return "Unimplemented";
        case Error::Unknown:
        default:
            return "Unknown error";
        }
    }
}