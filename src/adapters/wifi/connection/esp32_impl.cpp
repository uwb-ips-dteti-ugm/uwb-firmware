#include "adapters/wifi/connection/esp32_impl.h"

#include <cstdio>

#include <WiFi.h>
#include <esp_mac.h>

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

    bool ESP32Impl::getDeviceId(char *device_id, std::size_t length) const
    {
        if (device_id == nullptr || length < 13)
            return false;

        uint8_t mac[6] = {};
        if (esp_efuse_mac_get_default(mac) != ESP_OK)
            return false;

        const int written = snprintf(
            device_id,
            length,
            "%02X%02X%02X%02X%02X%02X",
            static_cast<unsigned int>(mac[0]),
            static_cast<unsigned int>(mac[1]),
            static_cast<unsigned int>(mac[2]),
            static_cast<unsigned int>(mac[3]),
            static_cast<unsigned int>(mac[4]),
            static_cast<unsigned int>(mac[5]));

        return written == 12;
    }
}
