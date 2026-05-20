#include "controllers/task/wifi_connection.h"

namespace controllers::task
{
    // Controller implementations

    WiFiConnection::WiFiConnection(
        ports::driving::task::WiFiConnection *service,
        const char *ssid,
        const char *password,
        uint32_t check_interval_ms,
        const char *task_name,
        uint32_t stack_depth,
        UBaseType_t priority)
        : service(service),
          ssid(ssid ? ssid : ""),
          password(password ? password : ""),
          check_interval_ms(check_interval_ms == 0 ? 5000 : check_interval_ms),
          task_name(task_name ? task_name : "wifi_connection"),
          stack_depth(stack_depth),
          priority(priority),
          task_handle(nullptr),
          running(false)
    {
    }

    WiFiConnection::~WiFiConnection()
    {
        stop();
    }

    bool WiFiConnection::start()
    {
        if (task_handle != nullptr)
            return true;

        if (service == nullptr || ssid.length() == 0 || task_name.length() == 0 || stack_depth == 0)
            return false;

        running = true;
        const BaseType_t result = xTaskCreate(
            WiFiConnection::task,
            task_name.c_str(),
            stack_depth,
            this,
            priority,
            &task_handle);

        if (result != pdPASS)
        {
            running = false;
            task_handle = nullptr;
            return false;
        }

        return true;
    }

    void WiFiConnection::stop()
    {
        if (task_handle == nullptr)
            return;

        running = false;

        if (task_handle == xTaskGetCurrentTaskHandle())
            return;

        while (task_handle != nullptr)
            vTaskDelay(pdMS_TO_TICKS(check_interval_ms));
    }

    void WiFiConnection::task(void *argument)
    {
        WiFiConnection *self = static_cast<WiFiConnection *>(argument);

        while (self->running)
        {
            if (!self->service->isConnected())
                self->service->connect(self->ssid.c_str(), self->password.c_str());

            vTaskDelay(pdMS_TO_TICKS(self->check_interval_ms));
        }

        self->service->disconnect();
        self->task_handle = nullptr;
        self->running = false;
        vTaskDelete(nullptr);
    }
}
