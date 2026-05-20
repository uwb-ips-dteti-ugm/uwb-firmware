#pragma once

#include <cstddef>
#include <cstdint>

#include "domain/models/error.h"

namespace ports::driving::task
{
    class UWBStateless
    {
    public:
        virtual ~UWBStateless() = default;
        virtual bool getDeviceId(char *device_id, std::size_t length) const = 0;
        virtual bool isWiFiConnected() const = 0;
        virtual models::Error sendConnectRequest(const char *device_id) = 0;
        virtual models::Error initiate(
            uint16_t pan_id,
            uint16_t source_address,
            uint16_t destination_address,
            uint32_t timeout_uus,
            float *distance) = 0;
        virtual models::Error listen(
            uint16_t pan_id,
            uint16_t destination_address,
            uint16_t source_address,
            uint32_t timeout_uus) = 0;
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
