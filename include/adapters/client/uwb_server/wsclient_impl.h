#pragma once

#include <cstdint>

#include <Arduino.h>
#include <WebSocketsClient.h>

#include "domain/ports/driven/client/uwb_server.h"
#include "domain/ports/driven/logger/leveled.h"

namespace adapters::client::uwb_server
{
    class WSClientImpl : public ports::driven::client::UWBServer
    {
    public:
        WSClientImpl(
            ports::driven::logger::Leveled *logger,
            WebSocketsClient *client,
            const char *scheme,
            const char *host,
            uint16_t port,
            const char *path,
            uint32_t connect_timeout_ms = 5000);
        ~WSClientImpl() override = default;
        WSClientImpl(const WSClientImpl &) = delete;
        WSClientImpl &operator=(const WSClientImpl &) = delete;

        models::Error sendConnectRequest(const char *device_id) override;
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
        WebSocketsClient *client;
        String scheme;
        String host;
        uint16_t port;
        String path;
        uint32_t connect_timeout_ms;
        ports::driven::logger::Leveled *logger;
    };
}
