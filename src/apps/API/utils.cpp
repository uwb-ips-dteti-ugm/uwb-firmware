#include "apps/API/utils.h"

bool api::utl::rIsWiFiAutoConnect()
{
    return true;
    // return (bool)api::wifiCfg.autoConnect;
}

void api::utl::rGetWiFiSTACredentials(char *ssidBuf, char *passBuf)
{
    const char *ssid = "Qi";
    const char *password = "hurufgede";
    memcpy(ssidBuf, ssid, 32);
    memcpy(passBuf, password, 32);
    // memcpy(ssidBuf, api::wifiCfg.staSSID, 32);
    // memcpy(passBuf, api::wifiCfg.staPASS, 32);
}

void api::utl::rGetMDNS(char *buffer)
{
    // const char *dns = "server"; 
    const char *dns = "anchor"; 
    // const char *dns = "client1"; 
    // const char *dns = "client2"; 
    // const char *dns = "client3"; 
    // const char *dns = "client4"; 
    memcpy(buffer, dns, 32);
    // memcpy(buffer, api::srvCfg.mDNS, 32);
}

bool api::utl::rIsUWBServer()
{
    // return true;
    return false;
    // return (bool)api::uwbCfg.isServer;
}

bool api::utl::rIsUWBAutoStart()
{
    return true;
    // return (bool)api::uwbCfg.autoStart;
}

api::KernelState api::utl::rGetKernelState()
{
    api::KernelState state = api::kerReg.kernelState;
    return state;
}

void api::utl::rSetKernelState(api::KernelState state)
{
    api::kerReg.kernelState = state;
}

bool api::utl::rWiFiConfigExists(fs::LittleFSFS *fs)
{
    return fs->exists(API_WIFI_CONFIG_FILENAME);
}

void api::utl::rLoadWiFiConfig(fs::LittleFSFS *fs)
{
    fs::File file = fs->open(API_WIFI_CONFIG_FILENAME, FILE_READ);
    if (file)
    {
        file.read((uint8_t *)&api::wifiCfg, sizeof(api::WiFiConfig));
        file.close();
    }
}

void api::utl::rSaveWiFiConfig(fs::LittleFSFS *fs)
{
    fs::File file = fs->open(API_WIFI_CONFIG_FILENAME, FILE_WRITE);
    if (file)
    {
        file.write((uint8_t *)&api::wifiCfg, sizeof(api::WiFiConfig));
        file.close();
    }
}


void api::utl::rEnableWiFiAutoConnect()
{
    api::wifiCfg.autoConnect = 1;
}

void api::utl::rDisableWiFiAutoConnect()
{
    api::wifiCfg.autoConnect = 0;
}

void api::utl::rGetWiFiAPCredentials(char *ssidBuf, char *passBuf)
{
    memcpy(ssidBuf, api::wifiCfg.apSSID, 32);
    memcpy(passBuf, api::wifiCfg.apPASS, 32);
}


void api::utl::rSetWiFiAPCredentials(const char *ssid, const char *pass)
{
    memset(api::wifiCfg.apSSID, 0, 32);
    memset(api::wifiCfg.apPASS, 0, 32);

    strcpy(api::wifiCfg.apSSID, ssid);
    strcpy(api::wifiCfg.apPASS, pass);
}

void api::utl::rSetWiFiSTACredentials(const char *ssid, const char *pass)
{
    memset(api::wifiCfg.staSSID, 0, 32);
    memset(api::wifiCfg.staPASS, 0, 32);

    strcpy(api::wifiCfg.staSSID, ssid);
    strcpy(api::wifiCfg.staPASS, pass);
}

bool api::utl::rServerConfigExists(fs::LittleFSFS *fs)
{
    return fs->exists(API_SERVER_CONFIG_FILENAME);
}

void api::utl::rLoadServerConfig(fs::LittleFSFS *fs)
{
    fs::File file = fs->open(API_SERVER_CONFIG_FILENAME, FILE_READ);
    if (file)
    {
        file.read((uint8_t *)&api::srvCfg, sizeof(api::ServerConfig));
        file.close();
    }
}

void api::utl::rSaveServerConfig(fs::LittleFSFS *fs)
{
    fs::File file = fs->open(API_SERVER_CONFIG_FILENAME, FILE_WRITE);
    if (file)
    {
        file.write((uint8_t *)&api::srvCfg, sizeof(api::ServerConfig));
        file.close();
    }
}

uint16_t api::utl::rGetServerPort()
{
    return api::srvCfg.port;
}

void api::utl::rSetServerPort(uint16_t port)
{
    api::srvCfg.port = port;
}

void api::utl::rSetMDNS(const char *mdns)
{
    strcpy(api::srvCfg.mDNS, mdns);
}

bool api::utl::rUWBConfigExists(fs::LittleFSFS *fs)
{
    return fs->exists(API_UWB_CONFIG_FILENAME);
}

void api::utl::rLoadUWBConfig(fs::LittleFSFS *fs)
{
    fs::File file = fs->open(API_UWB_CONFIG_FILENAME, FILE_READ);
    if (file)
    {
        file.read((uint8_t *)&api::uwbCfg, sizeof(api::UWBConfig));
        file.close();
    }
}

void api::utl::rSaveUWBConfig(fs::LittleFSFS *fs)
{
    fs::File file = fs->open(API_UWB_CONFIG_FILENAME, FILE_WRITE);
    if (file)
    {
        file.write((uint8_t *)&api::uwbCfg, sizeof(api::UWBConfig));
        file.close();
    }
}



void api::utl::rEnableUWBAutoStart()
{
    api::uwbCfg.autoStart = 1;
}

void api::utl::rDisableUWBAutoStart()
{
    api::uwbCfg.autoStart = 0;
}

void api::utl::rSetAsUWBServer()
{
    api::uwbCfg.isServer = 1;
}

void api::utl::rSetAsUWBClient()
{
    api::uwbCfg.isServer = 0;
}

void api::utl::rGetUWBConfig(uint8_t *mode, uint8_t *clientMax, uint16_t *networkAddr, uint16_t *deviceAddr)
{
    *mode = api::uwbCfg.rangingMode;
    *clientMax = api::uwbCfg.clientMax;
    *networkAddr = api::uwbCfg.networkAddr;
    *deviceAddr = api::uwbCfg.deviceAddr;
}

void api::utl::rSetUWBConfig(uint8_t mode, uint8_t clientMax, uint16_t networkAddr, uint16_t deviceAddr)
{
    api::uwbCfg.rangingMode = mode;
    api::uwbCfg.clientMax = clientMax;
    api::uwbCfg.networkAddr = networkAddr;
    api::uwbCfg.deviceAddr = deviceAddr;
}

void api::utl::tWiFiConnect(bool autoConnect, const char *apSSID, const char *apPASS, const char *staSSID, const char *staPASS)
{
    if (autoConnect)
        api::utl::rEnableWiFiAutoConnect();
    else
        api::utl::rDisableWiFiAutoConnect();

    api::utl::rSetWiFiAPCredentials(apSSID, apPASS);
    api::utl::rSetWiFiSTACredentials(staSSID, staPASS);

    api::trp::models::Generic q;
    q.type = api::trp::types::WIFI_CONNECT;
    xQueueSend(api::trp::queue, &q, 0);
}

void api::utl::tWiFiDisconnect()
{
    api::trp::models::Generic q;
    q.type = api::trp::types::WIFI_DISCONNECT;
    xQueueSend(api::trp::queue, &q, 0);
}

void api::utl::tServerConfig(uint16_t port, const char *mdns)
{
    api::utl::rSetServerPort(port);
    api::utl::rSetMDNS(mdns);

    api::trp::models::Generic q;
    q.type = api::trp::types::SERVER_CONFIG;
    xQueueSend(api::trp::queue, &q, 0);
}

void api::utl::tUWBConfig(bool autoStart, bool isServer, uint8_t clientMax, uint8_t mode, uint16_t networkAddr, uint16_t deviceAddr)
{
    if (autoStart)
        api::utl::rEnableUWBAutoStart();
    else
        api::utl::rDisableUWBAutoStart();

    if (isServer)
        api::utl::rSetAsUWBServer();
    else
        api::utl::rSetAsUWBClient();

    api::utl::rSetUWBConfig(mode, clientMax, networkAddr, deviceAddr);

    api::trp::models::Generic q;
    q.type = api::trp::types::UWB_CONFIG;
    xQueueSend(api::trp::queue, &q, 0);
}

void api::utl::tUWBClientInfo()
{
    api::trp::models::Generic q;
    q.type = api::trp::types::UWB_CLIENT_INFO;
    xQueueSend(api::trp::queue, &q, 0);
}

void api::utl::tUWBClientTWR()
{
    api::trp::models::Generic q;
    q.type = api::trp::types::UWB_CLIENT_TWR;
    xQueueSend(api::trp::queue, &q, 0);
}
