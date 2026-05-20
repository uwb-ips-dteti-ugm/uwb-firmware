#pragma once

#include <cstddef>
#include <cstdint>

#include "domain/ports/driven/client/uwb_server.h"
#include "domain/ports/driven/logger/leveled.h"
#include "domain/ports/driven/ranging/stateless.h"
#include "domain/ports/driven/wifi/connection.h"
#include "domain/ports/driving/task/uwb_stateless.h"

namespace services::task::uwb_stateless
{
    class BaseImpl : public ports::driving::task::UWBStateless
    {
    public:
        BaseImpl(
            ports::driven::logger::Leveled *logger,
            ports::driven::client::UWBServer *client,
            ports::driven::ranging::Stateless *ranging,
            ports::driven::wifi::Connection *wifi);
        ~BaseImpl() override = default;
        BaseImpl(const BaseImpl &) = delete;
        BaseImpl &operator=(const BaseImpl &) = delete;

        bool getDeviceId(char *device_id, std::size_t length) const override;
        bool isWiFiConnected() const override;
        models::Error sendConnectRequest(const char *device_id) override;
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
        models::Error sendRangingResult(
            const char *device_id,
            uint16_t pan_id,
            uint16_t source_address,
            uint16_t destination_address,
            float distance) override;
        models::Error sendError(
            const char *device_id,
            uint16_t pan_id,
            uint16_t source_address,
            uint16_t destination_address,
            const char *message) override;

    private:
        ports::driven::logger::Leveled *logger;
        ports::driven::client::UWBServer *client;
        ports::driven::ranging::Stateless *ranging;
        ports::driven::wifi::Connection *wifi;
    };
}
