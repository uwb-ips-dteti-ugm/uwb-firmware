#ifndef __UWBSERVER_REGISTRY_H__
#define __UWBSERVER_REGISTRY_H__

#include <Arduino.h>
#include <forward_list>
#include <queue>
#include "config.h"
#include "middlewares/UWB/server.h"

namespace uws
{
    enum KernelState : uint8_t
    {
        KERNEL_STATE_CONFIG,
        KERNEL_STATE_CONFIG_FAILED,
        KERNEL_STATE_RUNNING
    };

    struct KernelRegistry
    {
        TaskHandle_t taskHandle = NULL;
        KernelState kernelState = KERNEL_STATE_CONFIG;
    };

    struct NetworkInfo
    {
        SemaphoreHandle_t mtx = xSemaphoreCreateMutex();
        uint8_t clientNum = 0;
        uwb::DW3000Server::ClientInfo *clientInfo = nullptr;
        std::queue<uwb::DW3000Server::TWRData> twrData;
    };

    extern KernelRegistry kerReg;
    extern NetworkInfo netInfo;
}

#endif