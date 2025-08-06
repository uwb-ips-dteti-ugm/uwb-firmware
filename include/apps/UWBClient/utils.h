#ifndef __UWBCLIENT_UTILS_H__
#define __UWBCLIENT_UTILS_H__

#include <Arduino.h>
#include "config.h"
#include "registry.h"
#include "transport.h"
#include "middlewares/UWB/client.h"

namespace uwc
{
    namespace utl
    {
        KernelState rGetKernelState();
        void rSetKernelState(KernelState state);
    }
}

#endif