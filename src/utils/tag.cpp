#include "tag.h"
using namespace uwbsys;

void TagDW3000::deviceConfig(dwt_config_t *configuration)
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

void TagDW3000::networkConfig(uint16_t networkAddress, uint16_t deviceAddress)
{
    this->setNetworkAddress(networkAddress);
    this->setDeviceAddress(deviceAddress);
}

void TagDW3000::setRangingMode(uint8_t mode)
{
    this->rangingMode = mode;
}

bool TagDW3000::begin()
{
    this->networkEventQueue = xQueueCreate(8, sizeof(TagDW3000::NetworkEventParams));

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

void TagDW3000::spin()
{
    TagDW3000::NetworkEventParams paramBuf;
    this->networkEventListen();
    if (xQueueReceive(this->networkEventQueue, &paramBuf, 0) == pdTRUE)
    {
        switch (paramBuf.event)
        {
        case TagDW3000::NETWORK_EVENT_AUTHORIZE:
            this->onEventAuthorize(&paramBuf);
            break;

        case TagDW3000::NETWORK_EVENT_NETWORK_UPDATE:
            this->onEventNetworkUpdate(&paramBuf);
            break;

        case TagDW3000::NETWORK_EVENT_CLOCK_SYNC:
            this->onEventClockSync(&paramBuf);
            break;

        case TagDW3000::NETWORK_EVENT_TDOA_ACCESS:
            this->onEventTDoAccess(&paramBuf);
            break;

        case TagDW3000::NETWORK_EVENT_TWR_ACCESS:
            this->onEventTWRAccess(&paramBuf);
            break;
        }

        if (paramBuf.payloadPtr != nullptr)
            delete paramBuf.payloadPtr;
    }
}

TagDW3000::NetworkEvent TagDW3000::getFrameNetworkEvent(uint8_t *frame)
{
    switch (frame[UWB_FRAME_INDEX_FUNCTION_CODE])
    {
    case UWB_FUNCTION_CODE_AUTHORIZE:
        return TagDW3000::NETWORK_EVENT_AUTHORIZE;

    case UWB_FUNCTION_CODE_NETWORK_UPDATE:
        return TagDW3000::NETWORK_EVENT_NETWORK_UPDATE;

    case UWB_FUNCTION_CODE_CLOCK_SYNC:
        return TagDW3000::NETWORK_EVENT_CLOCK_SYNC;

    case UWB_FUNCTION_CODE_TDOA_ACCESS:
        return TagDW3000::NETWORK_EVENT_TDOA_ACCESS;

    case UWB_FUNCTION_CODE_TWR_ACCESS:
        return TagDW3000::NETWORK_EVENT_TWR_ACCESS;

    default:
        return TagDW3000::NETWORK_EVENT_NONE;
    }
}

void TagDW3000::networkEventListen()
{
    static bool rxEnabled = false;
    static uint32_t statusReg;
    static size_t frameLen;
    static uint8_t frameBuffer[DW3000_FRAME_MAX_SIZE];

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

        TagDW3000::NetworkEventParams params;
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

void TagDW3000::onEventAuthorize(TagDW3000::NetworkEventParams *params)
{
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

void TagDW3000::onEventNetworkUpdate(TagDW3000::NetworkEventParams *params)
{
}

void TagDW3000::onEventClockSync(TagDW3000::NetworkEventParams *params)
{
}

void TagDW3000::onEventTDoAccess(TagDW3000::NetworkEventParams *params)
{
}

void TagDW3000::onEventTWRAccess(TagDW3000::NetworkEventParams *params)
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
    seqeunceNum++;

    while (!((statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    }

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
