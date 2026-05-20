#pragma once

#include <cstdint>

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "domain/ports/driving/task/wifi_connection.h"

namespace controllers::task
{
    class WiFiConnection
    {
    public:
        WiFiConnection(
            ports::driving::task::WiFiConnection *service,
            const char *ssid,
            const char *password,
            uint32_t check_interval_ms = 5000,
            const char *task_name = "wifi_connection",
            uint32_t stack_depth = 4096,
            UBaseType_t priority = 1);
        ~WiFiConnection();
        WiFiConnection(const WiFiConnection &) = delete;
        WiFiConnection &operator=(const WiFiConnection &) = delete;

        bool start();
        void stop();

    private:
        static void task(void *argument);

        ports::driving::task::WiFiConnection *service;
        String ssid;
        String password;
        uint32_t check_interval_ms;
        String task_name;
        uint32_t stack_depth;
        UBaseType_t priority;
        TaskHandle_t task_handle;
        volatile bool running;
    };
}
