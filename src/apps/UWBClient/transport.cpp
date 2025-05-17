#include "apps/UWBClient/transport.h"

namespace uwc
{
    namespace trp
    {
        QueueHandle_t queue = xQueueCreate(UWBCLIENT_KERNEL_TRANSPORT_QUEUE_SIZE, sizeof(models::Generic));
    }
}