#pragma once

#include <cstdint>

#include "domain/ports/driven/logger/leveled.h"
#include "domain/ports/driven/wifi/connection.h"
#include "domain/ports/driving/task/wifi_connection.h"

namespace services::task::wifi_connection
{
    class BaseImpl : public ports::driving::task::WiFiConnection
    {
    public:
        BaseImpl(
            ports::driven::wifi::Connection *connection,
            ports::driven::logger::Leveled *logger,
            uint32_t check_interval_ms = 5000);
        ~BaseImpl() override = default;
        BaseImpl(const BaseImpl &) = delete;
        BaseImpl &operator=(const BaseImpl &) = delete;

        void connect(const char *ssid, const char *password) override;
        void disconnect() override;
        bool isConnected() const override;

    private:
        ports::driven::wifi::Connection *connection;
        ports::driven::logger::Leveled *logger;
        uint32_t check_interval_ms;
    };
}
