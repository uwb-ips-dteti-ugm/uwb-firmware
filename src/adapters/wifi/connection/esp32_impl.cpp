#include "adapters/wifi/connection/esp32_impl.h"

#include <WiFi.h>

namespace adapters::wifi::connection
{

    // Adapter implementations

    ESP32Impl::ESP32Impl() = default;

    void ESP32Impl::connect(const char *ssid, const char *password)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
    }

    void ESP32Impl::disconnect()
    {
        WiFi.disconnect(true);
    }

    bool ESP32Impl::isConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }
}
