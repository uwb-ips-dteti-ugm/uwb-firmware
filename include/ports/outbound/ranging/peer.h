#pragma once

#include <cstdint>
#include "domain/models/error.h"

namespace po::ranging
{
    class Peer
    {
    public:
        virtual ~Peer() = default;
        virtual dom::models::Error getDistance(uint16_t target, float *distance) = 0;
    };
}