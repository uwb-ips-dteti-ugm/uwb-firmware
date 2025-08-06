#ifndef __UWBCLIENT_KERNEL_H__
#define __UWBCLIENT_KERNEL_H__

#include <Arduino.h>
#include "apps/shared.h"
#include "middlewares/UWB/client.h"

namespace uwc
{
    struct KernelInst
    {
        uint16_t deviceAddress = 0xFFFF;
        uwb::RangingMode mode = uwb::RANGING_MODE_NONE;
        uwb::DW3000Client *client = nullptr;
    };

    void run(KernelInst *inst);
    void kernelTask(void *pvParameters);
    void kernelSetup(KernelInst *inst);
    void kernelLoop(KernelInst *inst);
    void kernelEventRoutine(KernelInst *inst);
    void kernelMainRoutine(KernelInst *inst);

    namespace fnc
    {
    }
}

#endif