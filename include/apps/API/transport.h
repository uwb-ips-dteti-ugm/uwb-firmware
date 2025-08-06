#ifndef __API_TRANSPORT_H__
#define __API_TRANSPORT_H__

#include <Arduino.h>
#include "config.h"

namespace api
{
    namespace trp
    {
        namespace types
        {
            enum TransportType : uint8_t
            {
                NONE,
                WIFI_CONNECT,
                WIFI_DISCONNECT,
                SERVER_CONFIG,
                UWB_CONFIG,
                UWB_CLIENT_INFO,
                UWB_CLIENT_TWR
            };
        }

        namespace models
        {
            struct Generic
            {
                types::TransportType type = types::NONE;
                void *dataPtr = nullptr;
            };
        }

        extern QueueHandle_t queue;
    }
}

#endif