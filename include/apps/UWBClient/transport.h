#ifndef __UWBCLIENT_TRANSPORT_H__
#define __UWBCLIENT_TRANSPORT_H__

#include <Arduino.h>
#include "config.h"

namespace uwc
{
    namespace trp
    {
        namespace types
        {
            enum TransportType : uint8_t
            {
                NONE
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