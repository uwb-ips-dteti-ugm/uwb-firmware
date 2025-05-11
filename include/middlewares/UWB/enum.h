#ifndef __UWB_ENUM_H__
#define __UWB_ENUM_H__

#include <Arduino.h>

namespace uwb
{
    enum FrameIndex : uint8_t
    {
        FRAME_INDEX_CONTROL = 0,
        FRAME_INDEX_SEQUENCE_NUMBER = 2,
        FRAME_INDEX_NETWORK_ADDRESS = 3,
        FRAME_INDEX_DESTINATION_ADDRESS = 5,
        FRAME_INDEX_SOURCE_ADDRESS = 7,
        FRAME_INDEX_FUNCTION_CODE = 9,
        FRAME_INDEX_PAYLOAD = 10
    };

    enum FunctionCode : uint8_t
    {
        FUNCTION_CODE_AUTHORIZE = 0x60,
        FUNCTION_CODE_NETWORK_UPDATE = 0x61,
        FUNCTION_CODE_CLOCK_SYNC = 0x62,
        FUNCTION_CODE_TDOA_SCHEDULE = 0xA0,
        FUNCTION_CODE_TWR_SCHEDULE = 0xB0,
        FUNCTION_CODE_TWR_REQUEST = 0xB1,
        FUNCTION_CODE_TWR_RESPONSE = 0xB2,
        FUNCTION_CODE_TWR_ACKNOWLEDGE = 0xB3,
        FUNCTION_CODE_TWR_ACCESS = 0xB4
    };

    enum OperationMode : uint8_t
    {
        OPERATION_MODE_NONE = 0x00,
        OPERATION_MODE_SERVER = 0x01,
        OPERATION_MODE_CLIENT = 0x02
    };

    enum RangingMode : uint8_t
    {
        RANGING_MODE_NONE = 0x00,
        RANGING_MODE_TDOA = 0x01,
        RANGING_MODE_TWR = 0x02
    };

    enum NetworkEvent : uint8_t
    {
        NETWORK_EVENT_NONE,
        NETWORK_EVENT_AUTH,
        NETWORK_EVENT_UPDATE,
        NETWORK_EVENT_SYNC
    };
}

#endif