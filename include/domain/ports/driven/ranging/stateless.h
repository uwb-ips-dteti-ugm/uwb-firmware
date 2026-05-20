#pragma once

#include <cstdint>

#include "domain/models/error.h"

namespace ports::driven::ranging
{
    class Stateless
    {
    public:
        virtual ~Stateless() = default;
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
    };
}
