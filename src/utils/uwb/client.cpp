#include "client.h"
using namespace uwbsys;

void ClientDW3000::deviceConfig(dwt_config_t *configuration)
{
    this->dwConfig = new dwt_config_t;
    if (configuration != nullptr)
        memcpy(this->dwConfig, configuration, sizeof(dwt_config_t));
    else
    {
        this->dwConfig->chan = 5;
        this->dwConfig->txPreambLength = DWT_PLEN_128;
        this->dwConfig->rxPAC = DWT_PAC8;
        this->dwConfig->txCode = 9;
        this->dwConfig->rxCode = 9;
        this->dwConfig->sfdType = 0;
        this->dwConfig->dataRate = DWT_BR_6M8;
        this->dwConfig->phrMode = DWT_PHRMODE_STD;
        this->dwConfig->phrRate = DWT_PHRMODE_STD;
        this->dwConfig->sfdTO = (129 + 8 - 8);
        this->dwConfig->stsMode = DWT_STS_MODE_OFF;
        this->dwConfig->stsLength = DWT_STS_LEN_64;
        this->dwConfig->pdoaMode = DWT_PDOA_M0;
    }
}

void ClientDW3000::networkConfig(uint16_t networkAddress, uint16_t deviceAddress, uint64_t timeout)
{
    this->setNetworkAddress(networkAddress);
    this->setDeviceAddress(deviceAddress);
    this->timeout = timeout;
}

void ClientDW3000::setRangingMode(uint8_t mode)
{
    this->rangingMode = mode;
}

bool ClientDW3000::begin()
{
    this->networkEventQueue = xQueueCreate(4, sizeof(ClientDW3000::NetworkEventParams));

    _fastSPI = SPISettings(16000000L, MSBFIRST, SPI_MODE0);
    spiBegin(DW3000_PIN_IRQ, DW3000_PIN_RST);
    spiSelect(DW3000_PIN_SS);
    vTaskDelay(pdMS_TO_TICKS(2));

    if (!dwt_checkidlerc())
        return false;

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
        return false;

    if (dwt_configure(this->dwConfig))
        return false;

    dwt_configuretxrf(&txconfig_options);
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
    dwt_setrxantennadelay(DW3000_RX_ANTENNA_DELAY_UUS);
    dwt_settxantennadelay(DW3000_TX_ANTENNA_DELAY_UUS);
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

    return true;
}

void ClientDW3000::spin()
{
    ClientDW3000::NetworkEventParams paramBuf;
    this->networkEventListen();
    if (xQueueReceive(this->networkEventQueue, &paramBuf, 0) == pdTRUE)
    {
        switch (paramBuf.event)
        {
        case Base::NETWORK_EVENT_AUTHORIZE:
            this->onEventAuthorize(&paramBuf);
            break;

        case Base::NETWORK_EVENT_NETWORK_UPDATE:
            this->onEventNetworkUpdate(&paramBuf);
            break;

        case Base::NETWORK_EVENT_CLOCK_SYNC:
            this->onEventClockSync(&paramBuf);
            break;

        case Base::NETWORK_EVENT_TDOA_ACCESS:
            this->onEventTDoAAccess(&paramBuf);
            break;

        case Base::NETWORK_EVENT_TWR_ACCESS:
            this->onEventTWRAccess(&paramBuf);
            break;

        case Base::NETWORK_EVENT_TWR_GRANT:
            this->onEventTWRGrant(&paramBuf);
            break;
        }

        if (paramBuf.payloadPtr != nullptr)
            delete paramBuf.payloadPtr;
    }
}

bool ClientDW3000::isNetworkConnected()
{
    return this->networkConnected;
}

Base::NetworkEvent ClientDW3000::getFrameNetworkEvent(uint8_t *frame)
{
    switch (frame[UWB_FRAME_INDEX_FUNCTION_CODE])
    {
    case UWB_FUNCTION_CODE_AUTHORIZE:
        return Base::NETWORK_EVENT_AUTHORIZE;

    case UWB_FUNCTION_CODE_NETWORK_UPDATE:
        return Base::NETWORK_EVENT_NETWORK_UPDATE;

    case UWB_FUNCTION_CODE_CLOCK_SYNC:
        return Base::NETWORK_EVENT_CLOCK_SYNC;

    case UWB_FUNCTION_CODE_TDOA_ACCESS:
        return Base::NETWORK_EVENT_TDOA_ACCESS;

    case UWB_FUNCTION_CODE_TWR_ACCESS:
        return Base::NETWORK_EVENT_TWR_ACCESS;

    case UWB_FUNCTION_CODE_TWR_REQUEST:
        return Base::NETWORK_EVENT_TWR_GRANT;

    default:
        return Base::NETWORK_EVENT_NONE;
    }
}

void ClientDW3000::networkEventListen()
{
    static bool rxEnabled = false;
    static uint32_t statusReg;
    static size_t frameLen;
    static uint8_t frameBuffer[DW3000_FRAME_MAX_SIZE];
    static uint64_t currentTime;

    currentTime = millis();
    this->networkConnected = (currentTime - this->lastEventTimestamp >= this->timeout) ? false : this->networkConnected;

    if (!rxEnabled)
    {
        dwt_setrxtimeout(0);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        rxEnabled = true;
    }

    if (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR)))
        return;
    rxEnabled = false;

    if (statusReg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        frameLen = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        if (frameLen > DW3000_FRAME_MAX_SIZE)
            return;

        dwt_readrxdata(frameBuffer, frameLen, 0);
        if (!this->validateFrame(frameBuffer))
        {
            memset(frameBuffer, 0x00, frameLen);
            return;
        }

        ClientDW3000::NetworkEventParams params;
        params.event = this->getFrameNetworkEvent(frameBuffer);
        params.sourceAddress = this->getFrameSourceAddress(frameBuffer);
        params.payloadSize = this->getFramePayload(params.payloadPtr, frameBuffer);
        xQueueSend(this->networkEventQueue, &params, portMAX_DELAY);
        memset(frameBuffer, 0x00, frameLen);
        return;
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        return;
    }
}

void ClientDW3000::onEventAuthorize(ClientDW3000::NetworkEventParams *params)
{
    if (this->networkConnected)
        return;
    this->lastEventTimestamp = millis();

    uint8_t payload = this->rangingMode;
    uint8_t *frameBuf;
    size_t frameLen = this->generateFrame(
        frameBuf,
        params->sourceAddress,
        UWB_FUNCTION_CODE_AUTHORIZE,
        0x01,
        &payload);

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(frameLen, frameBuf, 0);
    dwt_writetxfctrl(frameLen, 0, 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE);

    delete[] frameBuf;
    this->masterAddress = params->sourceAddress;
    this->networkConnected = true;
}

void ClientDW3000::onEventNetworkUpdate(ClientDW3000::NetworkEventParams *params)
{
    if (!this->networkConnected)
        return;
    this->lastEventTimestamp = millis();
}

void ClientDW3000::onEventClockSync(ClientDW3000::NetworkEventParams *params)
{
    if (!this->networkConnected)
        return;
    this->lastEventTimestamp = millis();
}

void ClientDW3000::onEventTDoAAccess(ClientDW3000::NetworkEventParams *params)
{
    if (!this->networkConnected)
        return;
    this->lastEventTimestamp = millis();
}

void ClientDW3000::onEventTWRAccess(ClientDW3000::NetworkEventParams *params)
{
    static uint32_t statusReg;
    static uint8_t seqeunceNum = 0;
    static size_t frameLen;
    static uint32_t reqTxTs;
    static uint32_t reqRxTs;
    static uint32_t resTxTs;
    static uint32_t resRxTs;
    static int32_t rtdTag;
    static int32_t rtdAnc;
    static float clockOffsetRatio;
    static double tof;
    static double distance;

    if (!this->networkConnected)
        return;
    this->lastEventTimestamp = millis();

    uint16_t targetAddress;
    memcpy(&targetAddress, params->payloadPtr, sizeof(uint16_t));

    uint8_t bufferFrame[20];
    uint8_t *requestFrame;
    size_t requestFrameLen = this->generateFrame(
        requestFrame,
        targetAddress,
        UWB_FUNCTION_CODE_TWR_REQUEST);

    dwt_setrxaftertxdelay(DW3000_RX_AFTER_TX_DELAY_UUS);
    dwt_setrxtimeout(DW3000_RX_TIMEOUT_UUS);
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(requestFrameLen, requestFrame, 0);
    dwt_writetxfctrl(requestFrameLen, 0, 1);
    delete[] requestFrame;

    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    }
    seqeunceNum++;

    if (statusReg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        frameLen = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        if (frameLen > 20U)
            return;

        dwt_readrxdata(bufferFrame, frameLen, 0);
        if (!this->validateFrame(bufferFrame))
            return;

        reqTxTs = dwt_readtxtimestamplo32();
        resRxTs = dwt_readrxtimestamplo32();
        clockOffsetRatio = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

        resp_msg_get_ts(bufferFrame + 10, &reqRxTs);
        resp_msg_get_ts(bufferFrame + 14, &resTxTs);

        rtdTag = resRxTs - reqTxTs;
        rtdAnc = resTxTs - reqRxTs;
        tof = ((rtdTag - rtdAnc * (1.0 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
        distance = tof * SPEED_OF_LIGHT;
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    }
}

void ClientDW3000::onEventTWRGrant(ClientDW3000::NetworkEventParams *params)
{
    static uint32_t statusReg;
    static uint8_t seqeunceNum = 0;
    static size_t frameLen;
    static uint32_t responseTxDWTime;
    static uint64_t requestRxTime;
    static uint64_t responseTxTime;
    static uint8_t responseFrame[20] = {0x41, 0x88, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, UWB_FUNCTION_CODE_TWR_RESPONSE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static int txRet;

    memcpy(responseFrame + UWB_FRAME_INDEX_NETWORK_ADDRESS, &this->networkAddress, sizeof(uint16_t));
    memcpy(responseFrame + UWB_FRAME_INDEX_DESTINATION_ADDRESS, &params->sourceAddress, sizeof(uint16_t));
    memcpy(responseFrame + UWB_FRAME_INDEX_SOURCE_ADDRESS, &this->deviceAddress, sizeof(uint16_t));

    requestRxTime = get_rx_timestamp_u64();
    responseTxDWTime = (requestRxTime + (DW3000_TX_AFTER_RX_DELAY_UUS * UUS_TO_DWT_TIME)) >> 8;
    dwt_setdelayedtrxtime(responseTxDWTime);

    responseTxTime = (((uint64_t)(responseTxDWTime & 0xFFFFFFFEUL)) << 8) + DW3000_TX_ANTENNA_DELAY_UUS;
    resp_msg_set_ts(responseFrame + 10, requestRxTime);
    resp_msg_set_ts(responseFrame + 14, responseTxTime);

    dwt_writetxdata(sizeof(responseFrame), responseFrame, 0);
    dwt_writetxfctrl(sizeof(responseFrame), 0, 1);
    txRet = dwt_starttx(DWT_START_TX_DELAYED);
    if (txRet == DWT_SUCCESS)
    {
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
        {
        };
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        seqeunceNum++;
    }
}
