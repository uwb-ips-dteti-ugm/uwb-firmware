#ifndef __API_UTILS_H__
#define __API_UTILS_H__

#include <Arduino.h>
#include <LittleFS.h>
#include "config.h"
#include "registry.h"
#include "transport.h"

namespace api
{
    namespace utl
    {
        KernelState rGetKernelState();
        void rSetKernelState(KernelState state);

        bool rWiFiConfigExists(fs::LittleFSFS *fs);
        void rLoadWiFiConfig(fs::LittleFSFS *fs);
        void rSaveWiFiConfig(fs::LittleFSFS *fs);
        bool rIsWiFiAutoConnect();
        void rEnableWiFiAutoConnect();
        void rDisableWiFiAutoConnect();
        void rGetWiFiAPCredentials(char *ssidBuf, char *passBuf);
        void rGetWiFiSTACredentials(char *ssidBuf, char *passBuf);
        void rSetWiFiAPCredentials(const char *ssid, const char *pass);
        void rSetWiFiSTACredentials(const char *ssid, const char *pass);

        bool rServerConfigExists(fs::LittleFSFS *fs);
        void rLoadServerConfig(fs::LittleFSFS *fs);
        void rSaveServerConfig(fs::LittleFSFS *fs);
        uint16_t rGetServerPort();
        void rGetMDNS(char *buffer);
        void rSetServerPort(uint16_t port);
        void rSetMDNS(const char *mdns);

        bool rUWBConfigExists(fs::LittleFSFS *fs);
        void rLoadUWBConfig(fs::LittleFSFS *fs);
        void rSaveUWBConfig(fs::LittleFSFS *fs);
        bool rIsUWBAutoStart();
        void rEnableUWBAutoStart();
        void rDisableUWBAutoStart();
        bool rIsUWBServer();
        void rSetAsUWBServer();
        void rSetAsUWBClient();
        void rGetUWBConfig(uint8_t *clientMax, uint8_t *mode, uint16_t *networkAddr, uint16_t *deviceAddr);
        void rSetUWBConfig(uint8_t clientMax, uint8_t mode, uint16_t networkAddr, uint16_t deviceAddr);

        void tWiFiConnect(bool autoConnect, const char *apSSID, const char *apPASS, const char *staSSID, const char *staPASS);
        void tWiFiDisconnect();
        void tServerConfig(uint16_t port, const char *mdns);
        void tUWBConfig(bool autoStart, bool isServer, uint8_t clientMax, uint8_t mode, uint16_t networkAddr, uint16_t deviceAddr);
        void tUWBClientInfo();
        void tUWBClientTWR();
    }
}

#endif