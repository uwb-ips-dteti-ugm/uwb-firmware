#pragma once

#include "domain/models/error.h"

namespace ports::driven::client
{
    class UWBServer
    {
    public:
        virtual ~UWBServer() = default;
        virtual models::Error sendConnectRequest(const char *device_id) = 0;
        virtual models::Error sendRangingResult(
            const char *device_id,
            uint16_t pan_id,
            uint16_t source_address,
            uint16_t destination_address,
            float distance) = 0;
        virtual models::Error sendError(
            const char *device_id,
            uint16_t pan_id,
            uint16_t source_address,
            uint16_t destination_address,
            const char *message) = 0;
    };
}