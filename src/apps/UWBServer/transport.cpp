#include "apps/UWBServer/transport.h"

namespace uws
{
    namespace trp
    {
        QueueHandle_t queue = xQueueCreate(UWBSERVER_KERNEL_TRANSPORT_QUEUE_SIZE, sizeof(models::Generic));
    }
}