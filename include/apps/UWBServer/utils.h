#ifndef __UWBSERVER_UTILS_H__
#define __UWBSERVER_UTILS_H__

#include <Arduino.h>
#include "config.h"
#include "registry.h"
#include "transport.h"
#include "middlewares/UWB/server.h"

namespace uws
{
    namespace utl
    {
        KernelState rGetKernelState();
        void rSetKernelState(KernelState state);

        uint8_t rGetClientNum();
        void rGetClient(uwb::DW3000Server::ClientInfo *clients);
        uwb::DW3000Server::TWRData rGetTWRData();
        void rUpdateNetworkInfo(uwb::DW3000Server *server);
    }
}

#endif