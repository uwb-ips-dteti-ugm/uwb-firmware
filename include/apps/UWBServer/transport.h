#ifndef __UWBSERVER_TRANSPORT_H__
#define __UWBSERVER_TRANSPORT_H__

#include <Arduino.h>
#include "config.h"

namespace uws
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