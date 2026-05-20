#include "services/task/wifi_connection/base_impl.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace services::task::wifi_connection
{
    constexpr const char *connectTag = "task::wifi_connection::BaseImpl::connect";
    constexpr const char *disconnectTag = "task::wifi_connection::BaseImpl::disconnect";

    // Service implementations

    BaseImpl::BaseImpl(
        ports::driven::wifi::Connection *connection,
        ports::driven::logger::Leveled *logger,
        uint32_t check_interval_ms)
        : connection(connection),
          logger(logger),
          check_interval_ms(check_interval_ms == 0 ? 5000 : check_interval_ms)
    {
    }

    void BaseImpl::connect(const char *ssid, const char *password)
    {
        bool was_connected = false;

        for (;;)
        {
            if (connection->isConnected())
            {
                if (!was_connected)
                    logger->info(connectTag, "WiFi connected");

                was_connected = true;
            }
            else
            {
                if (was_connected)
                    logger->warn(connectTag, "WiFi disconnected");

                was_connected = false;
                logger->warn(connectTag, "Connecting to WiFi");
                connection->connect(ssid, password);
            }

            vTaskDelay(pdMS_TO_TICKS(check_interval_ms));
        }
    }

    void BaseImpl::disconnect()
    {
        logger->info(disconnectTag, "Disconnecting WiFi");
        connection->disconnect();
    }

    bool BaseImpl::isConnected() const
    {
        return connection->isConnected();
    }
}
