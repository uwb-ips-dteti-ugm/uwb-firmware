#ifndef __UWBSERVER_KERNEL_H__
#define __UWBSERVER_KERNEL_H__

#include <Arduino.h>
#include "utils.h"
#include "middlewares/UWB/server.h"

namespace uws
{
    struct KernelInst
    {
        uint8_t clientMax = 16;
        uint16_t networkAddress = 0xFFFF;
        uint16_t deviceAddress = 0xFFFF;
        uwb::DW3000Server *server = nullptr;
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