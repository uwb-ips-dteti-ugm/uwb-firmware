#pragma once

#include <cstdint>
#include <Arduino.h>
#include "ports/outbound/ranging/peer.h"
#include "ports/outbound/logging/leveled.h"

namespace ao::ranging
{
    class DW3000SSTWR : public po::ranging::Peer
    {
    public:
        DW3000SSTWR(uint16_t pan_id, uint16_t address, po::logging::Leveled *logger = nullptr);
        ~DW3000SSTWR() override = default;
        DW3000SSTWR(const DW3000SSTWR &) = delete;
        DW3000SSTWR &operator=(const DW3000SSTWR &) = delete;
        dom::models::Error getDistance(uint16_t target, float *distance) override;

    private:
        uint16_t pan_id;
        uint16_t address;
        po::logging::Leveled *logger;
    };
}