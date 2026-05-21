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
        ConnectionRejected,
        UwbPollTxFailed,
        UwbPollRxTimeout,
        UwbPollRxFailed,
        UwbPollFrameTooLong,
        UwbUnexpectedPollFrame,
        UwbResponseRxTimeout,
        UwbResponseRxFailed,
        UwbResponseFrameTooLong,
        UwbUnexpectedResponseFrame,
        UwbResponseTxFailed,
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
        case Error::ConnectionRejected:
            return "Connection rejected";
        case Error::UwbPollTxFailed:
            return "UWB poll TX failed";
        case Error::UwbPollRxTimeout:
            return "UWB poll RX timeout";
        case Error::UwbPollRxFailed:
            return "UWB poll RX failed";
        case Error::UwbPollFrameTooLong:
            return "UWB poll frame too long";
        case Error::UwbUnexpectedPollFrame:
            return "UWB unexpected poll frame";
        case Error::UwbResponseRxTimeout:
            return "UWB response RX timeout";
        case Error::UwbResponseRxFailed:
            return "UWB response RX failed";
        case Error::UwbResponseFrameTooLong:
            return "UWB response frame too long";
        case Error::UwbUnexpectedResponseFrame:
            return "UWB unexpected response frame";
        case Error::UwbResponseTxFailed:
            return "UWB response TX failed";
        case Error::Unimplemented:
            return "Unimplemented";
        case Error::Unknown:
        default:
            return "Unknown error";
        }
    }
}
