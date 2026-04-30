#pragma once

#include <cstdint>
#include <cstdlib>

namespace dom::models
{
    enum RangingFunctionCode : uint8_t
    {
        Poll = 0xE0,
        Resp = 0xE1,
    };

    enum RangingFrameLength : size_t
    {
        Poll = 12U,
        Resp = 20U,
    };

    enum RangingFrameIndex : uint8_t
    {
        FrameControlLow = 0,
        FrameControlHigh = 1,
        SequenceNumber = 2,
        PanIdLow = 3,
        PanIdHigh = 4,
        DestinationAddressLow = 5,
        DestinationAddressHigh = 6,
        SourceAddressLow = 7,
        SourceAddressHigh = 8,
        FunctionCode = 9,
        RespPollRxTime = 10,
        RespRespTxTime = 14
    };
}
