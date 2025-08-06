#ifndef __API_KERNEL_H__
#define __API_KERNEL_H__

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "route.h"
#include "apps/shared.h"
#include "apps/UWBServer/kernel.h"
#include "apps/UWBClient/kernel.h"

namespace api
{
    struct KernelInst
    {
        fs::LittleFSFS *fs = nullptr;
        WiFiClass *wifi = nullptr;
        WebServer *srv = nullptr;
        bool uwbRunning = false;
    };

    /*
     */
    void run(KernelInst *inst);

    /*
     */
    void kernelTask(void *pvParameters);

    /*
     */
    void kernelSetup(KernelInst *inst);

    /*
     */
    void kernelLoop(KernelInst *inst);

    /*
     */
    void kernelEventRoutine(KernelInst *inst);

    /*
     */
    void kernelMainRoutine(KernelInst *inst);

    namespace fnc
    {
    }
}

#endif