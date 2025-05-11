#include "middlewares/UWB/server.h"
using namespace uwb;

DW3000Server::ClientInfo::ClientInfo()
{
    this->addr = 0xFFFF;
    this->mode = RANGING_MODE_NONE;
    this->lastUpdate = 0;
}

DW3000Server::TWRData::TWRData()
{
    this->timestamp = millis();
    this->addr1 = 0xFFFF;
    this->addr2 = 0xFFFF;
    this->distance = 0.0;
}

DW3000Server::DW3000Server(uint8_t clientMax, uint16_t twrQueueSize, uint64_t timeout) : DW3000Base::DW3000Base()
{
    this->clients = new DW3000Server::ClientInfo[clientMax];
    this->clientNum = 0;
    this->clientMax = clientMax;
    this->clientTimeout = timeout;

    this->clientTWRQueue = xQueueCreate(twrQueueSize, sizeof(DW3000Server::TWRData));
    this->clientTWRQueueSize = twrQueueSize;
}

bool DW3000Server::deviceConfig(dwt_config_t *config)
{
    return this->begin(config);
}

void DW3000Server::networkConfig(uint16_t networkAddress, uint16_t deviceAddress)
{
    this->setNetworkAddress(networkAddress);
    this->setDeviceAddress(deviceAddress);
}

void DW3000Server::spin()
{
    this->authorizeRoutine();
    this->networkUpdateRoutine();
    // this->clockSyncRoutine();
    // this->tdoaScheduleRoutine();
    this->twrScheduleRoutine();
}

uint8_t DW3000Server::getClientNum()
{
    return this->clientNum;
}

uint8_t DW3000Server::getClients(DW3000Server::ClientInfo *buffer, size_t bufferLen)
{
    if ((bufferLen == 0xFFFFFFFF) || (bufferLen >= this->clientNum))
    {
        memcpy(buffer, this->clients, sizeof(DW3000Server::ClientInfo) * this->clientNum);
        return this->clientNum;
    }
    else
    {
        memcpy(buffer, this->clients, sizeof(DW3000Server::ClientInfo) * bufferLen);
        return (uint8_t)(bufferLen & 0xFF);
    }
}

uint16_t DW3000Server::isTWRDataAvailable()
{
    return (uint16_t)(uxQueueMessagesWaiting(this->clientTWRQueue));
}

bool DW3000Server::getTWRData(DW3000Server::TWRData *buffer)
{
    return (bool)xQueueReceive(this->clientTWRQueue, buffer, 0);
}

bool DW3000Server::addClient(uint16_t clientAddress, RangingMode mode)
{
    if (this->clientNum < this->clientMax)
    {
        this->clients[this->clientNum].addr = clientAddress;
        this->clients[this->clientNum].mode = mode;
        this->clients[this->clientNum].lastUpdate = millis();
        this->clientNum++;
        return true;
    }
    else
        return false;
}

bool DW3000Server::deleteClient(uint16_t clientAddress)
{
    for (uint8_t i = 0; i < this->clientNum; ++i)
    {
        if (this->clients[i].addr == clientAddress)
        {
            if (i != this->clientNum - 1)
            {
                memmove(
                    this->clients + i,
                    this->clients + i + 1,
                    sizeof(uint16_t) * (this->clientNum - 1 - i));
            }
            this->clientNum--;

            return true;
        }
    }
    return false;
}

bool DW3000Server::deleteClientByIndex(uint8_t index)
{
    if (index >= this->clientNum)
        return false;

    if (index != this->clientNum - 1)
    {
        memmove(
            this->clients + index,
            this->clients + index + 1,
            sizeof(uint16_t) * (this->clientNum - 1 - index));
    }
    this->clientNum--;

    return true;
}

bool DW3000Server::existClient(uint16_t clientAddress)
{
    for (uint8_t i = 0; i < this->clientNum; ++i)
    {
        if (this->clients[i].addr == clientAddress)
            return true;
    }
    return false;
}

void DW3000Server::authorizeRoutine()
{
    size_t frameSize = this->createFrame(
        this->txBuffer,
        127,
        0xFFFF,
        FUNCTION_CODE_AUTHORIZE);

    size_t recvSize = this->sendExpectResponse(
        this->txBuffer,
        frameSize,
        this->rxBuffer,
        127);

    uint16_t srcAddress;
    RangingMode rangingMode[1];

    if ((recvSize > 0) &&
        (this->getFrameFunctionCode(this->rxBuffer) == (uint8_t)FUNCTION_CODE_AUTHORIZE) &&
        (this->getFrameDestinationAddress(this->rxBuffer) == this->getDeviceAddress()) &&
        (this->getFramePayload((uint8_t *)rangingMode, 1, this->rxBuffer) == 1))
    {
        srcAddress = this->getFrameSourceAddress(this->rxBuffer);
        frameSize = this->createFrame(this->txBuffer, 127, srcAddress, FUNCTION_CODE_AUTHORIZE);
        this->send(this->txBuffer, frameSize);

        if (!this->existClient(srcAddress))
            this->addClient(srcAddress, rangingMode[0]);
    }
}

void DW3000Server::networkUpdateRoutine()
{
    for (uint8_t i = 0; i < this->clientNum; ++i)
    {
        size_t frameSize = this->createFrame(
            this->txBuffer,
            127,
            this->clients[i].addr,
            FUNCTION_CODE_NETWORK_UPDATE);

        size_t recvSize = this->sendExpectResponse(
            this->txBuffer,
            frameSize,
            this->rxBuffer,
            127);

        if ((recvSize > 0) &&
            (this->getFrameFunctionCode(this->rxBuffer) == (uint8_t)FUNCTION_CODE_NETWORK_UPDATE) &&
            (this->validateFrame(this->rxBuffer)))
        {
            this->clients[i].lastUpdate = millis();
        }

        if ((millis() - this->clients[i].lastUpdate) > this->clientTimeout)
            this->deleteClientByIndex(i);
    }
}

void DW3000Server::clockSyncRoutine()
{
}

void DW3000Server::tdoaScheduleRoutine()
{
}

void DW3000Server::twrScheduleRoutine()
{
    static uint8_t clientNumBuf = 0;
    static uint16_t totalIter = 0;
    static uint16_t twrIter = 0;
    static uint16_t idx = 0;
    static uint16_t jdx = 1;

    if (twrIter == totalIter)
    {
        twrIter = 0;
        idx = 0;
        jdx = 1;

        if (clientNumBuf != this->clientNum)
        {
            clientNumBuf = this->clientNum;
            totalIter = this->clientNum + ((this->clientNum < 2) ? 0 : ((this->clientNum * (this->clientNum - 1)) / 2));
        }
    }

    for (uint16_t i = 0; i < this->clientTWRQueueSize; ++i)
    {
        if (twrIter < clientNumBuf)
        {
            bool retrieved = false;
            double dataCnt = 0.0;
            double dist = 0.0;

            for (uint8_t j = 0; j < 5; ++j)
            {
                if (!this->twrServe(this->clients[twrIter].addr))
                    continue;

                if (this->receive(this->rxBuffer, 127, 10000) == 0)
                    continue;

                if ((this->validateFrame(this->rxBuffer)) &&
                    this->getFrameFunctionCode(this->rxBuffer) == (uint8_t)FUNCTION_CODE_TWR_ACKNOWLEDGE)
                {
                    double recvDist;
                    this->getFramePayload(
                        (uint8_t *)&recvDist,
                        sizeof(double),
                        this->rxBuffer);

                    dataCnt += 1.0;
                    dist += recvDist;
                    retrieved = true;
                }
            }

            if (retrieved)
            {
                dist /= dataCnt;
                DW3000Server::TWRData data;
                data.addr1 = this->clients[i].addr;
                data.addr2 = this->getDeviceAddress();
                data.distance = dist;

                this->appendTWRData(&data);
            }
        }

        else if (twrIter < totalIter)
        {
            size_t frameSize = this->createFrame(
                this->txBuffer,
                127,
                this->clients[idx].addr,
                FUNCTION_CODE_TWR_ACCESS,
                2,
                (uint8_t *)&this->clients[jdx].addr);

            this->send(this->txBuffer, frameSize);

            uint8_t sigCnt = 0;
            while (sigCnt < 21)
            {
                this->receive(this->rxBuffer, 127, 1000);
                if ((this->validateFrame(this->rxBuffer)) &&
                    (this->getFrameFunctionCode(this->rxBuffer) == FUNCTION_CODE_TWR_ACKNOWLEDGE))
                {
                    double recvDist;
                    this->getFramePayload(
                        (uint8_t *)&recvDist,
                        sizeof(double),
                        this->rxBuffer);

                    DW3000Server::TWRData data;
                    data.addr1 = this->clients[jdx].addr;
                    data.addr2 = this->clients[idx].addr;
                    data.distance = recvDist;

                    this->appendTWRData(&data);
                    break;
                }
                else
                    sigCnt++;
            }

            jdx++;
            if (jdx == clientNumBuf)
            {
                idx++;
                jdx = idx + 1;
            }
        }

        else
            break;

        twrIter++;
    }
}

bool DW3000Server::twrServe(uint16_t targetAddress)
{
    static uint8_t twrScheduleFrame[] = {0x41, 0x88, 0x00, 0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, FUNCTION_CODE_TWR_SCHEDULE, 0x00, 0x00, 0x00};
    static uint8_t twrResponseFrame[] = {0x41, 0x88, 0x00, 0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, FUNCTION_CODE_TWR_RESPONSE, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static uint8_t twrRequestMask[] = {0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, FUNCTION_CODE_TWR_REQUEST};
    static uint16_t networkAddress = this->getNetworkAddress();
    static uint16_t deviceAddress = this->getDeviceAddress();
    static uint64_t reqRxTs;
    static uint64_t resTxTs;
    static uint32_t resTxDelay;
    static uint32_t statusReg;
    static uint32_t frameLen;
    static int ret;

    memcpy(twrScheduleFrame + FRAME_INDEX_NETWORK_ADDRESS, &networkAddress, 2);
    memcpy(twrScheduleFrame + FRAME_INDEX_DESTINATION_ADDRESS, &targetAddress, 2);
    memcpy(twrScheduleFrame + FRAME_INDEX_SOURCE_ADDRESS, &deviceAddress, 2);

    memcpy(twrResponseFrame + FRAME_INDEX_NETWORK_ADDRESS, &networkAddress, 2);
    memcpy(twrResponseFrame + FRAME_INDEX_DESTINATION_ADDRESS, &targetAddress, 2);
    memcpy(twrResponseFrame + FRAME_INDEX_SOURCE_ADDRESS, &deviceAddress, 2);

    memcpy(twrRequestMask, &networkAddress, 2);
    memcpy(twrRequestMask + 2, &deviceAddress, 2);
    memcpy(twrRequestMask + 4, &targetAddress, 2);

    this->send(
        twrScheduleFrame,
        sizeof(twrScheduleFrame));

    dwt_setrxtimeout(10000);
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    };

    if (statusReg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        frameLen = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        dwt_readrxdata(this->rxBuffer, frameLen, 0);

        if (memcmp(this->rxBuffer + 3, twrRequestMask, sizeof(twrRequestMask)) == 0)
        {
            reqRxTs = get_rx_timestamp_u64();
            resTxDelay = (reqRxTs + (DW3000_TX_AFTER_RX_DELAY_UUS * UUS_TO_DWT_TIME)) >> 8;
            dwt_setdelayedtrxtime(resTxDelay);
            resTxTs = (((uint64_t)(resTxDelay & 0xFFFFFFFEUL)) << 8) + DW3000_TX_ANTENNA_DELAY_UUS;

            resp_msg_set_ts(twrResponseFrame + 11, reqRxTs);
            resp_msg_set_ts(twrResponseFrame + 15, resTxTs);

            dwt_writetxdata(sizeof(twrResponseFrame), twrResponseFrame, 0);
            dwt_writetxfctrl(sizeof(twrResponseFrame), 0, 1);
            ret = dwt_starttx(DWT_START_TX_DELAYED);

            if (ret == DWT_SUCCESS)
            {
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
                {
                };
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        return false;
    }
}

void DW3000Server::appendTWRData(DW3000Server::TWRData *data)
{
    static DW3000Server::TWRData dump;

    if (uxQueueMessagesWaiting(this->clientTWRQueue) < this->clientTWRQueueSize)
    {
        xQueueSend(this->clientTWRQueue, data, 0);
    }
    else
    {
        xQueueReceive(this->clientTWRQueue, &dump, 0);
        xQueueSend(this->clientTWRQueue, data, 0);
    }
}