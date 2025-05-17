#ifndef __API_ROUTES_H__
#define __API_ROUTES_H__

#include <Arduino.h>
#include <functional>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "apps/shared.h"
#include "middlewares/UWB/server.h"
#include "middlewares/UWB/client.h"

#define API_URL_GET_UWB_CLIENT_INFO "/api/uwb/client/info"
#define API_URL_GET_UWB_CLIENT_TWR "/api/uwb/client/twr"
#define API_URL_POST_WIFI_CONNECT "/api/wifi/connect"
#define API_URL_POST_WIFI_DISCONNECT "/api/wifi/disconnect"
#define API_URL_POST_SERVER_CONFIG "/api/wifi/config"
#define API_URL_POST_UWB_CONFIG "/api/uwb/config"
#define API_URL_POST_DEVICE_RESTART "/api/device/restart"

namespace api
{
    void route(WebServer *server);
    std::function<void(void)> routeWrapper(WebServer *server, std::function<void(WebServer *)> service);

    namespace service
    {
        void onGetUWBClientInfo(WebServer *server);
        void onGetUWBClientTWR(WebServer *server);
        void onPostWiFiConnect(WebServer *server);
        void onPostWiFiDisconnect(WebServer *server);
        void onPostServerConfig(WebServer *server);
        void onPostUWBConfig(WebServer *server);
        void onDeviceRestart(WebServer *server);
    }

    namespace response
    {
        void sendBadRequest(WebServer *server);
        void sendInvalidJSON(WebServer *server);
        void sendReceived(WebServer *server);
    }
}

#endif