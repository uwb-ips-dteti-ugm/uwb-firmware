#pragma once

#include <cstdint>

#ifndef UWB_FIRMWARE_WIFI_SSID
#define UWB_FIRMWARE_WIFI_SSID ""
#endif

#ifndef UWB_FIRMWARE_WIFI_PASSWORD
#define UWB_FIRMWARE_WIFI_PASSWORD ""
#endif

#ifndef UWB_FIRMWARE_SERVER_HOST
#define UWB_FIRMWARE_SERVER_HOST ""
#endif

#ifndef UWB_FIRMWARE_SERVER_SCHEME
#define UWB_FIRMWARE_SERVER_SCHEME "ws"
#endif

#ifndef UWB_FIRMWARE_SERVER_PORT
#define UWB_FIRMWARE_SERVER_PORT 80
#endif

#ifndef UWB_FIRMWARE_SERVER_PATH
#define UWB_FIRMWARE_SERVER_PATH "/"
#endif

namespace composition::config
{
    constexpr uint32_t serialBaud = 115200;

    constexpr uint8_t uwbPinReset = 27;
    constexpr uint8_t uwbPinIrq = 34;
    constexpr uint8_t uwbPinSelect = 4;
    constexpr uint32_t uwbSpiFastClockHz = 16000000;

    constexpr uint16_t uwbTxAntennaDelay = 16385;
    constexpr uint16_t uwbRxAntennaDelay = 16385;
    constexpr uint32_t uwbPollTxToRespRxDelayUus = 240;
    constexpr uint32_t uwbPollRxToRespTxDelayUus = 650;
    constexpr uint32_t uwbDefaultResponseRxTimeoutUus = 400;

    constexpr const char *wifiSsid = UWB_FIRMWARE_WIFI_SSID;
    constexpr const char *wifiPassword = UWB_FIRMWARE_WIFI_PASSWORD;

    constexpr const char *uwbServerScheme = UWB_FIRMWARE_SERVER_SCHEME;
    constexpr const char *uwbServerHost = UWB_FIRMWARE_SERVER_HOST;
    constexpr uint16_t uwbServerPort = UWB_FIRMWARE_SERVER_PORT;
    constexpr const char *uwbServerPath = UWB_FIRMWARE_SERVER_PATH;
    constexpr uint32_t uwbServerConnectTimeoutMs = 5000;

    constexpr uint32_t wifiTaskCheckIntervalMs = 5000;
    constexpr uint32_t wifiTaskStackDepth = 4096;
    constexpr uint8_t wifiTaskPriority = 3;

    constexpr uint32_t uwbTaskCheckIntervalMs = 20;
    constexpr uint32_t uwbTaskStackDepth = 8192;
    constexpr uint8_t uwbTaskPriority = 9;
}
