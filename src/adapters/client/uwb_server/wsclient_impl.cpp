#include "adapters/client/uwb_server/wsclient_impl.h"

#include <cstdint>

#include <ArduinoJson.h>

namespace adapters::client::uwb_server
{
    constexpr const char *connectTag = "client::uwb_server::WSClientImpl::sendConnectRequest";
    constexpr const char *rangingTag = "client::uwb_server::WSClientImpl::sendRangingResult";
    constexpr const char *errorTag = "client::uwb_server::WSClientImpl::sendError";

    // Helpers

    bool isEmpty(const char *value)
    {
        return value == nullptr || value[0] == '\0';
    }

    String normalizePath(const char *path)
    {
        String normalized = path ? path : "/";
        normalized.trim();

        if (normalized.length() == 0)
            normalized = "/";

        if (!normalized.startsWith("/"))
            normalized = "/" + normalized;

        return normalized;
    }

    String normalizeScheme(const char *scheme)
    {
        String normalized = scheme ? scheme : "ws";
        normalized.trim();
        normalized.toLowerCase();

        if (normalized.endsWith("://"))
            normalized.remove(normalized.length() - 3);

        return normalized;
    }

    String buildDevicePath(const String &path, const char *device_id)
    {
        String device_path = path;
        if (!device_path.endsWith("/"))
            device_path += "/";
        device_path += device_id;
        return device_path;
    }

    bool isSupportedScheme(const String &scheme)
    {
        return scheme == "ws" || scheme == "wss";
    }

    models::Error sendJson(
        WebSocketsClient *client,
        ports::driven::logger::Leveled *logger,
        const char *tag,
        const String &payload)
    {
        if (client == nullptr)
        {
            logger->error(tag, "Invalid adapter configuration");
            return models::Error::InvalidArgument;
        }

        client->loop();
        if (!client->isConnected())
        {
            logger->error(tag, "WebSocket is not connected");
            return models::Error::BadState;
        }

        if (!client->sendTXT(payload.c_str()))
        {
            logger->error(tag, "Failed to send payload: %s", payload.c_str());
            return models::Error::SystemFail;
        }

        return models::Error::Ok;
    }

    // Adapter implementations

    WSClientImpl::WSClientImpl(
        ports::driven::logger::Leveled *logger,
        WebSocketsClient *client,
        const char *scheme,
        const char *host,
        uint16_t port,
        const char *path,
        uint32_t connect_timeout_ms)
        : client(client),
          scheme(normalizeScheme(scheme)),
          host(host ? host : ""),
          port(port),
          path(normalizePath(path)),
          connect_timeout_ms(connect_timeout_ms),
          logger(logger)
    {
        this->host.trim();
    }

    models::Error WSClientImpl::sendConnectRequest(const char *device_id)
    {
        if (isEmpty(device_id))
        {
            logger->error(connectTag, "Invalid argument: device_id is empty");
            return models::Error::InvalidArgument;
        }

        if (client == nullptr || !isSupportedScheme(scheme) || host.length() == 0 || port == 0 || path.length() == 0 || connect_timeout_ms == 0)
        {
            logger->error(connectTag, "Invalid adapter configuration");
            return models::Error::InvalidArgument;
        }

        const String device_path = buildDevicePath(path, device_id);
        client->disconnect();

        if (scheme == "wss")
            client->beginSSL(host.c_str(), port, device_path.c_str());
        else
            client->begin(host.c_str(), port, device_path.c_str());

        const uint32_t started_at = millis();
        while (!client->isConnected() && millis() - started_at < connect_timeout_ms)
        {
            client->loop();
            delay(1);
        }

        if (!client->isConnected())
        {
            logger->error(
                connectTag,
                "Connection rejected or timed out (scheme=%s host=%s port=%u path=%s)",
                scheme.c_str(),
                host.c_str(),
                static_cast<unsigned int>(port),
                device_path.c_str());
            return models::Error::ConnectionRejected;
        }

        return models::Error::Ok;
    }

    models::Error WSClientImpl::sendRangingResult(
        const char *device_id,
        uint16_t pan_id,
        uint16_t source_address,
        uint16_t destination_address,
        float distance)
    {
        if (isEmpty(device_id))
        {
            logger->error(rangingTag, "Invalid argument: device_id is empty");
            return models::Error::InvalidArgument;
        }

        JsonDocument document;
        document["status"] = "OK";
        document["device_id"] = device_id;
        document["pan_id"] = pan_id;
        document["source_address"] = source_address;
        document["destination_address"] = destination_address;
        document["distance"] = distance;

        String payload;
        serializeJson(document, payload);
        if (payload.length() == 0)
        {
            logger->error(rangingTag, "Failed to serialize ranging result");
            return models::Error::MemoryAllocation;
        }

        return sendJson(client, logger, rangingTag, payload);
    }

    models::Error WSClientImpl::sendError(
        const char *device_id,
        uint16_t pan_id,
        uint16_t source_address,
        uint16_t destination_address,
        const char *message)
    {
        if (isEmpty(device_id))
        {
            logger->error(errorTag, "Invalid argument: device_id is empty");
            return models::Error::InvalidArgument;
        }

        if (isEmpty(message))
        {
            logger->error(errorTag, "Invalid argument: message is empty");
            return models::Error::InvalidArgument;
        }

        JsonDocument document;
        document["status"] = "ERROR";
        document["device_id"] = device_id;
        document["pan_id"] = pan_id;
        document["source_address"] = source_address;
        document["destination_address"] = destination_address;
        document["message"] = message;

        String payload;
        serializeJson(document, payload);
        if (payload.length() == 0)
        {
            logger->error(errorTag, "Failed to serialize error");
            return models::Error::MemoryAllocation;
        }

        return sendJson(client, logger, errorTag, payload);
    }
}
