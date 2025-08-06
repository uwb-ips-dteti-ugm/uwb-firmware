#ifndef __UWBCLIENT_REGISTRY_H__
#define __UWBCLIENT_REGISTRY_H__

#include <Arduino.h>
#include "config.h"

namespace uwc
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
    };

    extern KernelRegistry kerReg;
    extern NetworkInfo netInfo;
}

#endif