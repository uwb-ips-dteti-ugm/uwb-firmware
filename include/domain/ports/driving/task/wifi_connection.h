#pragma once

namespace ports::driving::task
{
    class WiFiConnection
    {
    public:
        virtual ~WiFiConnection() = default;
        virtual void connect(const char *ssid, const char *password) = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
    };
}
