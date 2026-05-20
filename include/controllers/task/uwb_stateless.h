#pragma once

#include <cstddef>
#include <cstdint>

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "domain/ports/driving/task/uwb_stateless.h"

namespace controllers::task
{
    class UWBStateless
    {
    public:
        UWBStateless(
            ports::driving::task::UWBStateless *service,
            WebSocketsClient *client,
            uint32_t check_interval_ms = 100,
            const char *task_name = "uwb_stateless",
            uint32_t stack_depth = 8192,
            UBaseType_t priority = 1);
        ~UWBStateless();
        UWBStateless(const UWBStateless &) = delete;
        UWBStateless &operator=(const UWBStateless &) = delete;

        bool start();
        void stop();

    private:
        static void task(void *argument);

        void handleEvent(WStype_t type, uint8_t *payload, std::size_t length);
        void handleCommand(const uint8_t *payload, std::size_t length);

        ports::driving::task::UWBStateless *service;
        WebSocketsClient *client;
        uint32_t check_interval_ms;
        String task_name;
        uint32_t stack_depth;
        UBaseType_t priority;
        TaskHandle_t task_handle;
        volatile bool running;
        char device_id[13];
    };
}
