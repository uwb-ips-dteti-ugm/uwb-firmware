#pragma once

#include <cstdint>

namespace dom::models
{
    enum class Error : uint8_t
    {
        Ok,
        InvalidArgument,
        Malloc,
        SystemFail,
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
            return "Invalid argument error";
        case Error::Malloc:
            return "Memory allocation error";
        case Error::SystemFail:
            return "System failure error";
        case Error::Unimplemented:
            return "Unimplemented";
        case Error::Unknown:
        default:
            return "Unknown error";
        }
    }
}