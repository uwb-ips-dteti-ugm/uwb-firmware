#include "services/task/uwb_stateless/base_impl.h"

namespace services::task::uwb_stateless
{
    constexpr const char *deviceIdTag = "task::uwb_stateless::BaseImpl::getDeviceId";
    constexpr const char *connectTag = "task::uwb_stateless::BaseImpl::sendConnectRequest";
    constexpr const char *initiateTag = "task::uwb_stateless::BaseImpl::initiate";
    constexpr const char *listenTag = "task::uwb_stateless::BaseImpl::listen";
    constexpr const char *rangingResultTag = "task::uwb_stateless::BaseImpl::sendRangingResult";
    constexpr const char *errorTag = "task::uwb_stateless::BaseImpl::sendError";

    // Helpers

    bool isEmpty(const char *value)
    {
        return value == nullptr || value[0] == '\0';
    }

    models::Error requireWiFi(
        ports::driven::logger::Leveled *logger,
        ports::driven::wifi::Connection *wifi,
        const char *tag)
    {
        if (!wifi->isConnected())
        {
            logger->error(tag, "WiFi is not connected");
            return models::Error::BadState;
        }

        return models::Error::Ok;
    }

    void logOperationError(
        ports::driven::logger::Leveled *logger,
        const char *tag,
        const char *operation,
        models::Error error)
    {
        logger->error(tag, "%s failed: %s", operation, models::errorToString(error));
    }

    // Service implementations

    BaseImpl::BaseImpl(
        ports::driven::logger::Leveled *logger,
        ports::driven::client::UWBServer *client,
        ports::driven::ranging::Stateless *ranging,
        ports::driven::wifi::Connection *wifi)
        : logger(logger),
          client(client),
          ranging(ranging),
          wifi(wifi)
    {
    }

    bool BaseImpl::getDeviceId(char *device_id, std::size_t length) const
    {
        const bool result = wifi->getDeviceId(device_id, length);
        if (!result)
        {
            logger->error(deviceIdTag, "Failed to get device ID");
            return false;
        }

        logger->info(deviceIdTag, "Device ID: %s", device_id);
        return true;
    }

    bool BaseImpl::isWiFiConnected() const
    {
        return wifi->isConnected();
    }

    models::Error BaseImpl::sendConnectRequest(const char *device_id)
    {
        if (isEmpty(device_id))
        {
            logger->error(connectTag, "Invalid argument: device_id is empty");
            return models::Error::InvalidArgument;
        }

        const models::Error wifi_error = requireWiFi(logger, wifi, connectTag);
        if (wifi_error != models::Error::Ok)
            return wifi_error;

        logger->info(connectTag, "Sending connect request (device_id=%s)", device_id);
        const models::Error error = client->sendConnectRequest(device_id);
        if (error != models::Error::Ok)
        {
            logOperationError(logger, connectTag, "Connect request", error);
            return error;
        }

        logger->info(connectTag, "Connect request accepted (device_id=%s)", device_id);
        return models::Error::Ok;
    }

    models::Error BaseImpl::initiate(
        uint16_t pan_id,
        uint16_t source_address,
        uint16_t destination_address,
        uint32_t timeout_uus,
        float *distance)
    {
        if (distance == nullptr)
        {
            logger->error(initiateTag, "Invalid argument: distance is null");
            return models::Error::InvalidArgument;
        }

        const models::Error wifi_error = requireWiFi(logger, wifi, initiateTag);
        if (wifi_error != models::Error::Ok)
            return wifi_error;

        logger->info(
            initiateTag,
            "Initiating ranging (pan_id=0x%04X source=0x%04X destination=0x%04X timeout_uus=%lu)",
            static_cast<unsigned int>(pan_id),
            static_cast<unsigned int>(source_address),
            static_cast<unsigned int>(destination_address),
            static_cast<unsigned long>(timeout_uus));

        const models::Error error = ranging->initiate(pan_id, source_address, destination_address, timeout_uus, distance);
        if (error != models::Error::Ok)
        {
            logOperationError(logger, initiateTag, "Ranging initiate", error);
            return error;
        }

        logger->info(
            initiateTag,
            "Ranging initiated (pan_id=0x%04X source=0x%04X destination=0x%04X distance=%.3f)",
            static_cast<unsigned int>(pan_id),
            static_cast<unsigned int>(source_address),
            static_cast<unsigned int>(destination_address),
            static_cast<double>(*distance));
        return models::Error::Ok;
    }

    models::Error BaseImpl::listen(
        uint16_t pan_id,
        uint16_t destination_address,
        uint16_t source_address,
        uint32_t timeout_uus)
    {
        const models::Error wifi_error = requireWiFi(logger, wifi, listenTag);
        if (wifi_error != models::Error::Ok)
            return wifi_error;

        logger->info(
            listenTag,
            "Listening for ranging (pan_id=0x%04X destination=0x%04X source=0x%04X timeout_uus=%lu)",
            static_cast<unsigned int>(pan_id),
            static_cast<unsigned int>(destination_address),
            static_cast<unsigned int>(source_address),
            static_cast<unsigned long>(timeout_uus));

        const models::Error error = ranging->listen(pan_id, destination_address, source_address, timeout_uus);
        if (error != models::Error::Ok)
        {
            logOperationError(logger, listenTag, "Ranging listen", error);
            return error;
        }

        logger->info(
            listenTag,
            "Ranging response sent (pan_id=0x%04X destination=0x%04X source=0x%04X)",
            static_cast<unsigned int>(pan_id),
            static_cast<unsigned int>(destination_address),
            static_cast<unsigned int>(source_address));
        return models::Error::Ok;
    }

    models::Error BaseImpl::sendRangingResult(
        const char *device_id,
        uint16_t pan_id,
        uint16_t source_address,
        uint16_t destination_address,
        float distance)
    {
        if (isEmpty(device_id))
        {
            logger->error(rangingResultTag, "Invalid argument: device_id is empty");
            return models::Error::InvalidArgument;
        }

        const models::Error wifi_error = requireWiFi(logger, wifi, rangingResultTag);
        if (wifi_error != models::Error::Ok)
            return wifi_error;

        logger->info(
            rangingResultTag,
            "Sending ranging result (device_id=%s pan_id=0x%04X source=0x%04X destination=0x%04X distance=%.3f)",
            device_id,
            static_cast<unsigned int>(pan_id),
            static_cast<unsigned int>(source_address),
            static_cast<unsigned int>(destination_address),
            static_cast<double>(distance));

        const models::Error error = client->sendRangingResult(device_id, pan_id, source_address, destination_address, distance);
        if (error != models::Error::Ok)
        {
            logOperationError(logger, rangingResultTag, "Send ranging result", error);
            return error;
        }

        logger->info(rangingResultTag, "Ranging result sent (device_id=%s)", device_id);
        return models::Error::Ok;
    }

    models::Error BaseImpl::sendError(
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

        const models::Error wifi_error = requireWiFi(logger, wifi, errorTag);
        if (wifi_error != models::Error::Ok)
            return wifi_error;

        logger->info(
            errorTag,
            "Sending error (device_id=%s pan_id=0x%04X source=0x%04X destination=0x%04X message=%s)",
            device_id,
            static_cast<unsigned int>(pan_id),
            static_cast<unsigned int>(source_address),
            static_cast<unsigned int>(destination_address),
            message);

        const models::Error error = client->sendError(device_id, pan_id, source_address, destination_address, message);
        if (error != models::Error::Ok)
        {
            logOperationError(logger, errorTag, "Send error", error);
            return error;
        }

        logger->info(errorTag, "Error sent (device_id=%s)", device_id);
        return models::Error::Ok;
    }
}
