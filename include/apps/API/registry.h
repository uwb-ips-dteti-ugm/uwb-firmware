#ifndef __API_REGISTRY_H__
#define __API_REGISTRY_H__

#include <Arduino.h>
#include "config.h"

namespace api
{
    enum KernelState : uint8_t
    {
        KERNEL_STATE_WIFI_INIT,
        KERNEL_STATE_WIFI_ESTABLISH,
        KERNEL_STATE_WIFI_CONNECTING,
        KERNEL_STATE_SERVER_INIT,
        KERNEL_STATE_SERVER_LISTENING
    };

    struct KernelRegistry
    {
        TaskHandle_t taskHandle = NULL;
        KernelState kernelState = KERNEL_STATE_WIFI_INIT;
    };

    struct WiFiConfig
    {
        uint8_t autoConnect = 0;
        char apSSID[32] = API_WIFI_CONFIG_DEFAULT_AP_SSID;
        char apPASS[32] = API_WIFI_CONFIG_DEFAULT_AP_PASS;
        char staSSID[32] = "";
        char staPASS[32] = "";
    };

    struct ServerConfig
    {
        uint16_t port = 80;
        char mDNS[32] = API_SERVER_CONFIG_DEFAULT_MDNS;
    };

    struct UWBConfig
    {
        uint8_t autoStart = 0;
        uint8_t isServer = 0;
        uint8_t clientMax = 16;
        uint8_t rangingMode = 0;
        uint16_t networkAddr = 0xFFFF;
        uint16_t deviceAddr = 0xFFFF;
    };

    extern KernelRegistry kerReg;
    extern WiFiConfig wifiCfg;
    extern ServerConfig srvCfg;
    extern UWBConfig uwbCfg;
}

#endif