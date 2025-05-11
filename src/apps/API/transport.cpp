#include "apps/API/transport.h"

namespace api
{
    namespace trp
    {
        QueueHandle_t queue = xQueueCreate(API_KERNEL_TRANSPORT_QUEUE_SIZE, sizeof(models::Generic));
    }
}