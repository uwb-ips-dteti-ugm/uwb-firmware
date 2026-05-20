#pragma once

namespace ports::driven::wifi
{
    class Connection
    {
    public:
        virtual ~Connection() = default;
        virtual void connect(const char *ssid, const char *password) = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
    };
}