#include "apps/UWBServer/utils.h"

uws::KernelState uws::utl::rGetKernelState()
{
    uws::KernelState state = uws::kerReg.kernelState;
}

void uws::utl::rSetKernelState(uws::KernelState state)
{
    uws::kerReg.kernelState = state;
}

uint8_t uws::utl::rGetClientNum()
{
    if (xSemaphoreTake(uws::netInfo.mtx, portMAX_DELAY) == pdTRUE)
    {
        uint8_t n = uws::netInfo.clientNum;
        xSemaphoreGive(uws::netInfo.mtx);
        return n;
    }
}

void uws::utl::rGetClient(uwb::DW3000Server::ClientInfo *clients)
{
    if (xSemaphoreTake(uws::netInfo.mtx, portMAX_DELAY) == pdTRUE)
    {
        memcpy(clients, uws::netInfo.clientInfo, uws::netInfo.clientNum);
        xSemaphoreGive(uws::netInfo.mtx);
    }
}

uwb::DW3000Server::TWRData uws::utl::rGetTWRData()
{
    if (xSemaphoreTake(uws::netInfo.mtx, portMAX_DELAY) == pdTRUE)
    {
        uwb::DW3000Server::TWRData data = uws::netInfo.twrData.front();
        uws::netInfo.twrData.pop();
        xSemaphoreGive(uws::netInfo.mtx);
    }
}

void uws::utl::rUpdateNetworkInfo(uwb::DW3000Server *server)
{
    if (xSemaphoreTake(uws::netInfo.mtx, portMAX_DELAY) == pdTRUE)
    {
        if (uws::netInfo.clientNum != server->getClientNum())
        {
            uws::netInfo.clientNum = server->getClientNum();
            delete[] uws::netInfo.clientInfo;
            uws::netInfo.clientInfo = new uwb::DW3000Server::ClientInfo[uws::netInfo.clientNum];
            server->getClients(uws::netInfo.clientInfo, uws::netInfo.clientNum);
        }

        while (server->isTWRDataAvailable() > 0)
        {
            if (uws::netInfo.twrData.size() == UWBSERVER_KERNEL_TWR_UPDATE_QUEUE_SIZE)
                uws::netInfo.twrData.pop();

            uwb::DW3000Server::TWRData data;
            server->getTWRData(&data);
            uws::netInfo.twrData.push(data);
        }

        xSemaphoreGive(uws::netInfo.mtx);
    }
}
