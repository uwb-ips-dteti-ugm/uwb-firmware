#include "apps/API/route.h"

void api::route(WebServer *server)
{
    server->on(
        API_URL_GET_UWB_CLIENT_INFO,
        HTTP_GET,
        api::routeWrapper(
            server,
            api::service::onGetUWBClientInfo));

    server->on(
        API_URL_GET_UWB_CLIENT_TWR,
        HTTP_GET,
        api::routeWrapper(
            server,
            api::service::onGetUWBClientTWR));

    server->on(
        API_URL_POST_WIFI_CONNECT,
        HTTP_POST,
        api::routeWrapper(
            server,
            api::service::onPostWiFiConnect));

    server->on(
        API_URL_POST_WIFI_DISCONNECT,
        HTTP_POST,
        api::routeWrapper(
            server,
            api::service::onPostWiFiDisconnect));

    server->on(
        API_URL_POST_SERVER_CONFIG,
        HTTP_POST,
        api::routeWrapper(
            server,
            api::service::onPostServerConfig));

    server->on(
        API_URL_POST_UWB_CONFIG,
        HTTP_POST,
        api::routeWrapper(
            server,
            api::service::onPostUWBConfig));

    server->on(
        API_URL_POST_DEVICE_RESTART,
        HTTP_POST,
        api::routeWrapper(
            server,
            api::service::onDeviceRestart));
}

std::function<void(void)> api::routeWrapper(WebServer *server, std::function<void(WebServer *)> service)
{
    return [server, service]()
    {
        service(server);
    };
}

void api::service::onGetUWBClientInfo(WebServer *server)
{
    api::utl::tUWBClientInfo();
    api::response::sendReceived(server);
}

void api::service::onGetUWBClientTWR(WebServer *server)
{
    api::utl::tUWBClientTWR();
    api::response::sendReceived(server);
}

void api::service::onPostWiFiConnect(WebServer *server)
{
    if (!server->hasArg("plain"))
    {
        api::response::sendBadRequest(server);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server->arg("plain")))
    {
        api::response::sendInvalidJSON(server);
        return;
    }

    if (!doc.containsKey("autoconnect") ||
        !doc.containsKey("ap_ssid") ||
        !doc.containsKey("ap_pass") ||
        !doc.containsKey("sta_ssid") ||
        !doc.containsKey("sta_pass"))
    {
        api::response::sendInvalidJSON(server);
        return;
    }

    api::utl::tWiFiConnect(
        doc["autoconnect"].as<bool>(),
        doc["ap_ssid"].as<String>().c_str(),
        doc["ap_pass"].as<String>().c_str(),
        doc["sta_ssid"].as<String>().c_str(),
        doc["sta_pass"].as<String>().c_str());

    api::response::sendReceived(server);
}

void api::service::onPostWiFiDisconnect(WebServer *server)
{
    api::utl::tWiFiDisconnect();
    api::response::sendReceived(server);
}

void api::service::onPostServerConfig(WebServer *server)
{
    if (!server->hasArg("plain"))
    {
        api::response::sendBadRequest(server);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server->arg("plain")))
    {
        api::response::sendInvalidJSON(server);
        return;
    }

    if (!doc.containsKey("port") ||
        doc.containsKey("mdns"))
    {
        api::response::sendInvalidJSON(server);
        return;
    }

    api::utl::tServerConfig(
        doc["port"].as<uint16_t>(),
        doc["mdns"].as<String>().c_str());

    api::response::sendReceived(server);
}

void api::service::onPostUWBConfig(WebServer *server)
{
    if (!server->hasArg("plain"))
    {
        api::response::sendBadRequest(server);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server->arg("plain")))
    {
        api::response::sendInvalidJSON(server);
        return;
    }

    if (!doc.containsKey("autostart") ||
        !doc.containsKey("is_server") ||
        !doc.containsKey("client_max") ||
        !doc.containsKey("mode") ||
        !doc.containsKey("network_addr") ||
        !doc.containsKey("device_addr"))
    {
        api::response::sendInvalidJSON(server);
        return;
    }

    api::utl::tUWBConfig(
        doc["autostart"].as<bool>(),
        doc["is_server"].as<bool>(),
        doc["client_max"].as<uint8_t>(),
        doc["mode"].as<uint8_t>(),
        doc["network_addr"].as<uint16_t>(),
        doc["device_addr"].as<uint16_t>());

    api::response::sendReceived(server);
}

void api::service::onDeviceRestart(WebServer *server)
{
    ESP.restart();
}

void api::response::sendBadRequest(WebServer *server)
{
    server->send(400, "text/plain", "Bad Request");
}

void api::response::sendInvalidJSON(WebServer *server)
{
    server->send(400, "text/plain", "Invalid JSON");
}

void api::response::sendReceived(WebServer *server)
{
    server->send(200, "application/json", "{\"status\":\"received\"}");
}
