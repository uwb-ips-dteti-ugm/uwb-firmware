#include "controllers/task/uwb_stateless.h"

#include <ArduinoJson.h>

#include "domain/models/error.h"

namespace controllers::task
{
    constexpr int initiateCommandCode = 200;
    constexpr int listenCommandCode = 300;

    // Helpers

    bool readUint16(JsonObjectConst args, const char *key, uint16_t *value)
    {
        JsonVariantConst field = args[key];
        if (field.isNull() || !field.is<long>())
            return false;

        const long parsed = field.as<long>();
        if (parsed < 0 || parsed > 0xFFFF)
            return false;

        *value = static_cast<uint16_t>(parsed);
        return true;
    }

    bool readUint32(JsonObjectConst args, const char *key, uint32_t *value)
    {
        JsonVariantConst field = args[key];
        if (field.isNull() || !field.is<long>())
            return false;

        const long parsed = field.as<long>();
        if (parsed < 0)
            return false;

        *value = static_cast<uint32_t>(parsed);
        return true;
    }

    bool readRangingArgs(
        JsonObjectConst args,
        uint16_t *pan_id,
        uint16_t *destination_address,
        uint16_t *source_address,
        uint32_t *timeout_uus)
    {
        return readUint16(args, "pan_id", pan_id) &&
               readUint16(args, "destination_address", destination_address) &&
               readUint16(args, "source_address", source_address) &&
               readUint32(args, "timeout_uus", timeout_uus);
    }

    // Controller implementations

    UWBStateless::UWBStateless(
        ports::driving::task::UWBStateless *service,
        WebSocketsClient *client,
        uint32_t check_interval_ms,
        const char *task_name,
        uint32_t stack_depth,
        UBaseType_t priority)
        : service(service),
          client(client),
          check_interval_ms(check_interval_ms == 0 ? 100 : check_interval_ms),
          task_name(task_name ? task_name : "uwb_stateless"),
          stack_depth(stack_depth),
          priority(priority),
          task_handle(nullptr),
          running(false),
          device_id{}
    {
    }

    UWBStateless::~UWBStateless()
    {
        stop();
    }

    bool UWBStateless::start()
    {
        if (task_handle != nullptr)
            return true;

        if (service == nullptr || client == nullptr || task_name.length() == 0 || stack_depth == 0)
            return false;

        if (!service->getDeviceId(device_id, sizeof(device_id)))
            return false;

        client->onEvent([this](WStype_t type, uint8_t *payload, std::size_t length)
                        { handleEvent(type, payload, length); });

        running = true;
        const BaseType_t result = xTaskCreate(
            UWBStateless::task,
            task_name.c_str(),
            stack_depth,
            this,
            priority,
            &task_handle);

        if (result != pdPASS)
        {
            client->onEvent([](WStype_t, uint8_t *, std::size_t) {});
            running = false;
            task_handle = nullptr;
            return false;
        }

        return true;
    }

    void UWBStateless::stop()
    {
        if (task_handle == nullptr)
            return;

        running = false;

        if (task_handle == xTaskGetCurrentTaskHandle())
            return;

        while (task_handle != nullptr)
            vTaskDelay(pdMS_TO_TICKS(check_interval_ms));

        if (client != nullptr)
            client->onEvent([](WStype_t, uint8_t *, std::size_t) {});
    }

    void UWBStateless::task(void *argument)
    {
        UWBStateless *self = static_cast<UWBStateless *>(argument);

        while (self->running)
        {
            if (!self->service->isWiFiConnected())
            {
                vTaskDelay(pdMS_TO_TICKS(self->check_interval_ms));
                continue;
            }

            if (!self->client->isConnected())
            {
                self->service->sendConnectRequest(self->device_id);
                vTaskDelay(pdMS_TO_TICKS(self->check_interval_ms));
                continue;
            }

            self->client->loop();
            vTaskDelay(pdMS_TO_TICKS(self->check_interval_ms));
        }

        self->client->disconnect();
        self->task_handle = nullptr;
        self->running = false;
        vTaskDelete(nullptr);
    }

    void UWBStateless::handleEvent(WStype_t type, uint8_t *payload, std::size_t length)
    {
        if (!running || type != WStype_TEXT || payload == nullptr || length == 0)
            return;

        handleCommand(payload, length);
    }

    void UWBStateless::handleCommand(const uint8_t *payload, std::size_t length)
    {
        JsonDocument document;
        DeserializationError parse_error = deserializeJson(document, payload, length);
        if (parse_error)
            return;

        JsonVariantConst code_field = document["code"];
        JsonObjectConst args = document["args"].as<JsonObjectConst>();
        if (code_field.isNull() || !code_field.is<long>() || args.isNull())
            return;

        const long code = code_field.as<long>();
        uint16_t pan_id = 0;
        uint16_t destination_address = 0;
        uint16_t source_address = 0;
        uint32_t timeout_uus = 0;
        if (!readRangingArgs(args, &pan_id, &destination_address, &source_address, &timeout_uus))
            return;

        if (code == listenCommandCode)
        {
            const models::Error error = service->listen(pan_id, destination_address, source_address, timeout_uus);
            if (error != models::Error::Ok)
                service->sendError(device_id, pan_id, source_address, destination_address, models::errorToString(error));
            return;
        }

        if (code == initiateCommandCode)
        {
            float distance = 0.0F;
            const models::Error error = service->initiate(pan_id, source_address, destination_address, timeout_uus, &distance);
            if (error == models::Error::Ok)
                service->sendRangingResult(device_id, pan_id, source_address, destination_address, distance);
            else
                service->sendError(device_id, pan_id, source_address, destination_address, models::errorToString(error));
        }
    }
}
