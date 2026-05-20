#pragma once

#include <cstddef>
#include <cstdint>

#include "domain/ports/driven/ranging/stateless.h"
#include "domain/ports/driven/logger/leveled.h"

namespace adapters::ranging::stateless
{
    class DW3000Impl : public ports::driven::ranging::Stateless
    {
    public:
        DW3000Impl(
            ports::driven::logger::Leveled *logger,
            uint16_t tx_antenna_delay = 16385,
            uint32_t poll_tx_to_resp_rx_delay_uus = 240,
            uint32_t poll_rx_to_resp_tx_delay_uus = 450);
        ~DW3000Impl() override = default;
        DW3000Impl(const DW3000Impl &) = delete;
        DW3000Impl &operator=(const DW3000Impl &) = delete;

        models::Error initiate(
            uint16_t pan_id,
            uint16_t source_address,
            uint16_t destination_address,
            uint32_t timeout_uus,
            float *distance) override;
        models::Error listen(
            uint16_t pan_id,
            uint16_t destination_address,
            uint16_t source_address,
            uint32_t timeout_uus) override;

    private:
        uint8_t sequence_number;
        uint16_t tx_antenna_delay;
        uint32_t poll_tx_to_resp_rx_delay_uus;
        uint32_t poll_rx_to_resp_tx_delay_uus;
        ports::driven::logger::Leveled *logger;
    };
}
