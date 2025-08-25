#include "middlewares/UWB/client.h"
using namespace uwb;

DW3000Client::DW3000Client(uint64_t timeout) : DW3000Base::DW3000Base()
{
    this->rangingMode = RANGING_MODE_NONE;
    this->connected = false;
    this->serverAddress = 0xFFFF;
    this->connTimeout = timeout;
    this->connTimeoutTs = 0;
}

bool DW3000Client::deviceConfig(dwt_config_t *config)
{
    return this->begin(config);
}

void DW3000Client::networkConfig(uint16_t deviceAddress, RangingMode mode)
{
    this->setDeviceAddress(deviceAddress);
    this->setNetworkAddress(0xFFFF);
    this->rangingMode = mode;
}

void DW3000Client::initBypass()
{
    this->setNetworkAddress(0xABCD);
    this->serverAddress = 0x0001;
    this->connected = true;
}

void DW3000Client::spin()
{
    this->listen();
    // this->timeoutHandle();
}

bool DW3000Client::isConnected()
{
    return this->connected;
}

void DW3000Client::listen()
{
    this->receiveHold();
    if (!this->receiveAvailable())
        return;

    if (this->receiveCollect(this->rxBuffer, 127) > 0)
    {
        switch (this->getFrameFunctionCode(this->rxBuffer))
        {
            // case FUNCTION_CODE_AUTHORIZE:
            //     this->onEventAuthorize();
            //     break;

            // case FUNCTION_CODE_NETWORK_UPDATE:
            //     this->onEventNetworkUpdate();
            //     break;

            // case FUNCTION_CODE_CLOCK_SYNC:
            //     this->onEventClockSync();
            //     break;

            // case FUNCTION_CODE_TDOA_SCHEDULE:
            //     this->onEventTDOASchedule();
            //     break;

        case FUNCTION_CODE_TWR_SCHEDULE:
            this->onEventTWRSchedule();
            break;

        case FUNCTION_CODE_TWR_ACCESS:
            this->onEventTWRAccess();
            break;
        }
    }
}

void DW3000Client::onEventAuthorize()
{
    if (this->connected)
        return;

    uint16_t netAddress = this->getFrameNetworkAddress(this->rxBuffer);
    uint16_t srcAddress = this->getFrameSourceAddress(this->rxBuffer);
    size_t frameSize = this->createFrame(
        this->txBuffer,
        127,
        srcAddress,
        FUNCTION_CODE_AUTHORIZE,
        1,
        (uint8_t *)&this->rangingMode);

    if (this->sendExpectResponse(this->txBuffer, frameSize, this->rxBuffer, 127) > 0)
    {
        if (this->getFrameFunctionCode(this->rxBuffer) == FUNCTION_CODE_AUTHORIZE &&
            this->getFrameNetworkAddress(this->rxBuffer) == netAddress &&
            this->getFrameSourceAddress(this->rxBuffer) == srcAddress &&
            this->getFrameDestinationAddress(this->rxBuffer) == this->getDeviceAddress())
        {
            this->setNetworkAddress(netAddress);
            this->serverAddress = srcAddress;
            this->connTimeoutTs = millis();
            this->connected = true;
        }
    }
}

void DW3000Client::onEventNetworkUpdate()
{
    if ((!this->connected) || (!this->validateFrame(this->rxBuffer)))
        return;

    size_t frameSize = this->createFrame(
        this->txBuffer,
        127,
        this->serverAddress,
        FUNCTION_CODE_NETWORK_UPDATE);

    this->send(this->txBuffer, frameSize);
    this->connTimeoutTs = millis();
}

void DW3000Client::onEventClockSync()
{
    if ((!this->connected) || (!this->validateFrame(this->rxBuffer)))
        return;
}

void DW3000Client::onEventTDOASchedule()
{
    if ((!this->connected) || (!this->validateFrame(this->rxBuffer)))
        return;
}

void DW3000Client::onEventTWRSchedule()
{
    static uint32_t reqTxTs;
    static uint32_t resRxTs;
    static uint32_t reqRxTs;
    static uint32_t resTxTs;
    static int32_t rtdReq;
    static int32_t rtdRes;
    static float cor;
    static double tof;
    static double dist;

    if ((!this->connected) || (!this->validateFrame(this->rxBuffer)))
        return;

    uint16_t targetAddr = this->getFrameSourceAddress(this->rxBuffer);

    size_t frameSize = this->createFrame(
        this->txBuffer,
        127,
        targetAddr,
        FUNCTION_CODE_TWR_REQUEST);

    size_t recvSize = this->sendExpectResponse(
        this->txBuffer,
        frameSize,
        this->rxBuffer,
        127,
        true,
        DW3000_RX_AFTER_TX_DELAY_UUS,
        DW3000_RX_TIMEOUT_UUS);

    if ((recvSize > 0) &&
        (this->getFrameFunctionCode(this->rxBuffer) == (uint8_t)FUNCTION_CODE_TWR_RESPONSE) &&
        (this->validateFrame(this->rxBuffer)))
    {
        reqTxTs = dwt_readtxtimestamplo32();
        resRxTs = dwt_readrxtimestamplo32();
        cor = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

        resp_msg_get_ts(this->rxBuffer + 11, &reqRxTs);
        resp_msg_get_ts(this->rxBuffer + 15, &resTxTs);

        rtdReq = resRxTs - reqTxTs;
        rtdRes = resTxTs - reqRxTs;

        tof = ((rtdReq - rtdRes * (1 - cor)) / 2.0) * DWT_TIME_UNITS;
        dist = tof * SPEED_OF_LIGHT;

        size_t frameSize = this->createFrame(
            this->txBuffer,
            127,
            targetAddr,
            FUNCTION_CODE_TWR_ACKNOWLEDGE,
            8,
            (uint8_t *)&dist);

        this->send(this->txBuffer, frameSize);
    }
}

void DW3000Client::onEventTWRAccess()
{
    if ((!this->connected) || (!this->validateFrame(this->rxBuffer)))
        return;

    bool retrieved = false;
    double dataCnt = 0.0;
    double dist = 0.0;
    uint16_t srcAddress = this->getFrameSourceAddress(this->rxBuffer);
    uint16_t targetAddress;

    this->getFramePayload((uint8_t *)&targetAddress, 2, this->rxBuffer);

    for (uint8_t j = 0; j < 5; ++j)
    {
        if (!this->twrServe(targetAddress))
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
        dist /= dataCnt;
    else
        dist = -999.0;

    size_t frameSize = this->createFrame(
        this->txBuffer,
        127,
        srcAddress,
        FUNCTION_CODE_TWR_ACKNOWLEDGE,
        8,
        (uint8_t *)&dist);

    this->send(this->txBuffer, frameSize);
}

void DW3000Client::timeoutHandle()
{
    if (!this->connected)
        return;

    if ((millis() - this->connTimeoutTs) > this->connTimeout)
    {
        this->connected = false;
        this->serverAddress = 0xFFFF;
        this->setNetworkAddress(0xFFFF);
    }
}

bool DW3000Client::twrServe(uint16_t targetAddress)
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