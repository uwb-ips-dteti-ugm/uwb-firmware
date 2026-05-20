#pragma once

#include <WebSocketsClient.h>

#include "adapters/client/uwb_server/wsclient_impl.h"
#include "adapters/logger/leveled/serial_impl.h"
#include "adapters/ranging/stateless/dw3000_impl.h"
#include "adapters/wifi/connection/esp32_impl.h"
#include "composition/config.h"
#include "controllers/task/uwb_stateless.h"
#include "controllers/task/wifi_connection.h"
#include "services/task/uwb_stateless/base_impl.h"
#include "services/task/wifi_connection/base_impl.h"

namespace composition
{
    class App
    {
    public:
        App();
        ~App() = default;
        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void setup();
        void initInfrastructures();
        void initServices();
        void initControllers();
        void run();

    private:
        adapters::logger::leveled::SerialImpl logger;
        WebSocketsClient websocket_client;
        adapters::wifi::connection::ESP32Impl wifi_connection;
        adapters::ranging::stateless::DW3000Impl ranging;
        adapters::client::uwb_server::WSClientImpl uwb_server;
        services::task::wifi_connection::BaseImpl wifi_service;
        services::task::uwb_stateless::BaseImpl uwb_service;
        controllers::task::WiFiConnection wifi_controller;
        controllers::task::UWBStateless uwb_controller;
    };
}
