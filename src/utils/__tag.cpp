#include "tag.h"

void MakerfabsDW3000::UWBTag::deviceConfig(dwt_config_t *configuration)
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

void MakerfabsDW3000::UWBTag::networkConfig(uint16_t network_addr, uint16_t device_addr)
{
    this->networkAddress = network_addr;
    this->deviceAddress = device_addr;
}

void MakerfabsDW3000::UWBTag::setRangingMode(MakerfabsDW3000::UWBTag::RangingMode mode)
{
    this->rangingMode = mode;
}

bool MakerfabsDW3000::UWBTag::begin()
{
    this->networkEventQueue = xQueueCreate(8, sizeof(MakerfabsDW3000::UWBTag::UWBNetworkEventInfo));

    spiBegin(MAKERFABSDW3000_PIN_IRQ, MAKERFABSDW3000_PIN_RST);
    spiSelect(MAKERFABSDW3000_PIN_SS);
    vTaskDelay(pdMS_TO_TICKS(2));

    if (!dwt_checkidlerc())
        return false;

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
        return false;

    if (dwt_configure(this->dwConfig))
        return false;

    dwt_configuretxrf(&MakerfabsDW3000::txconfig_options);
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
    dwt_setrxantennadelay(MAKERFABSDW3000_RX_ANTENNA_DELAY_UUS);
    dwt_settxantennadelay(MAKERFABSDW3000_TX_ANTENNA_DELAY_UUS);
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

    return true;
}

void MakerfabsDW3000::UWBTag::spin()
{
    static MakerfabsDW3000::UWBTag::UWBNetworkEventInfo queueBuf;
    this->statusHandle();
    this->networkEventListen();
    if (xQueueReceive(this->networkEventQueue, &queueBuf, 0) == pdTRUE)
    {
        switch (queueBuf.event)
        {
        case MakerfabsDW3000::UWBTag::EVENT_AUTHORIZE:
            this->onEventAuthorize(queueBuf.sourceAddress);
            break;

        case MakerfabsDW3000::UWBTag::EVENT_NETWORK_UPDATE:
            this->onEventNetworkUpdate();
            break;

        case MakerfabsDW3000::UWBTag::EVENT_CLOCK_SYNC:
            this->onEventClockSync();
            break;

        case MakerfabsDW3000::UWBTag::EVENT_TDOA_ACCESS:
            this->onEventTDoAccess();
            break;

        case MakerfabsDW3000::UWBTag::EVENT_TWR_ACCESS:
            this->onEventTWRAccess();
            break;
        }
    }
}

bool MakerfabsDW3000::UWBTag::isNetworkConnected()
{
    return this->networkConnected;
}

bool MakerfabsDW3000::UWBTag::isRangeUpdated()
{
    return this->rangeUpdated;
}

bool MakerfabsDW3000::UWBTag::isPositionUpdated()
{
    return this->positionUpdated;
}

float MakerfabsDW3000::UWBTag::getRange(uint16_t *anchor_addr)
{
}

void MakerfabsDW3000::UWBTag::getPosition(float *coordinate)
{
}

uint8_t *MakerfabsDW3000::UWBTag::generateFrame(size_t frameLen, uint16_t &destination_addr, uint8_t function_code)
{
    uint8_t *retFrame = new uint8_t[frameLen];
    memset(retFrame, 0x00, frameLen);

    retFrame[0] = 0x41;
    retFrame[1] = 0x88;
    retFrame[UWB_FRAME_FUNCTION_CODE_INDEX] = function_code;

    memcpy(
        retFrame + UWB_FRAME_NETWORK_ADDRESS_INDEX,
        &this->networkAddress,
        sizeof(uint16_t));
    memcpy(
        retFrame + UWB_FRAME_DESTINATION_ADDRESS_INDEX,
        &destination_addr,
        sizeof(uint16_t));
    memcpy(
        retFrame + UWB_FRAME_SOURCE_ADDRESS_INDEX,
        &this->deviceAddress,
        sizeof(uint16_t));

    return retFrame;
}

bool MakerfabsDW3000::UWBTag::validateFrame(uint8_t *frame)
{
    uint16_t networkAddress = this->getFrameNetworkAddress(frame);
    uint16_t destinationAddress = this->getFrameDestinationAddress(frame);

    // check the frame control
    if (!(frame[0] == 0x41 && frame[1] == 0x88))
        return false;

    // check the network address
    if (networkAddress != this->networkAddress)
        return false;

    // check the destination address
    if (!((frame[5] == 0xFF && frame[6] == 0xFF) || (destinationAddress == this->deviceAddress)))
        return false;

    return true;
}

uint16_t MakerfabsDW3000::UWBTag::getFrameNetworkAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(
        &retVal,
        frame + UWB_FRAME_NETWORK_ADDRESS_INDEX,
        sizeof(uint16_t));
    return retVal;
}

uint16_t MakerfabsDW3000::UWBTag::getFrameDestinationAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(
        &retVal,
        frame + UWB_FRAME_DESTINATION_ADDRESS_INDEX,
        sizeof(uint16_t));
    return retVal;
}

uint16_t MakerfabsDW3000::UWBTag::getFrameSourceAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(
        &retVal,
        frame + UWB_FRAME_SOURCE_ADDRESS_INDEX,
        sizeof(uint16_t));
    return retVal;
}

MakerfabsDW3000::UWBTag::UWBNetworkEvent MakerfabsDW3000::UWBTag::getFrameEvent(uint8_t *frame)
{
    switch (frame[UWB_FRAME_FUNCTION_CODE_INDEX])
    {
    case UWB_FUNCTION_CODE_AUTHORIZE:
        return MakerfabsDW3000::UWBTag::EVENT_AUTHORIZE;

    case UWB_FUNCTION_CODE_NETWORK_UPDATE:
        return MakerfabsDW3000::UWBTag::EVENT_NETWORK_UPDATE;

    case UWB_FUNCTION_CODE_CLOCK_SYNC:
        return MakerfabsDW3000::UWBTag::EVENT_CLOCK_SYNC;

    case UWB_FUNCTION_CODE_TDOA_ACCESS:
        return MakerfabsDW3000::UWBTag::EVENT_TDOA_ACCESS;

    case UWB_FUNCTION_CODE_TWR_ACCESS:
        return MakerfabsDW3000::UWBTag::EVENT_TWR_ACCESS;

    default:
        return MakerfabsDW3000::UWBTag::EVENT_NONE;
    }
}

void MakerfabsDW3000::UWBTag::statusHandle()
{
}

void MakerfabsDW3000::UWBTag::networkEventListen()
{
    static bool rxEnabled = false;
    static uint32_t statusReg;
    static size_t frameLen;
    static uint8_t frameBuffer[MAKERFABSDW3000_FRAME_MAX_SIZE];

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
        if (frameLen > MAKERFABSDW3000_FRAME_MAX_SIZE)
            return;

        dwt_readrxdata(frameBuffer, frameLen, 0);
        if (!this->validateFrame(frameBuffer))
        {
            memset(frameBuffer, 0x00, frameLen);
            return;
        }

        MakerfabsDW3000::UWBTag::UWBNetworkEventInfo info;
        info.event = this->getFrameEvent(frameBuffer);
        info.sourceAddress = this->getFrameSourceAddress(frameBuffer);
        xQueueSend(this->networkEventQueue, &info, portMAX_DELAY);
        memset(frameBuffer, 0x00, frameLen);
        return;
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        return;
    }
}

void MakerfabsDW3000::UWBTag::onEventAuthorize(uint16_t &destination_addr)
{
    uint8_t *frame = this->generateFrame(13U, destination_addr, UWB_FUNCTION_CODE_AUTHORIZE);
    if (this->rangingMode == MakerfabsDW3000::UWBTag::RANGING_MODE_TDOA)
        frame[10] = UWB_AUTHORIZE_TDOA;
    else if (this->rangingMode == MakerfabsDW3000::UWBTag::RANGING_MODE_TWR)
        frame[10] = UWB_AUTHORIZE_TWR;

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(sizeof(frame), frame, 0);
    dwt_writetxfctrl(sizeof(frame), 0, 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE);

    delete[] frame;
}

void MakerfabsDW3000::UWBTag::onEventNetworkUpdate()
{
}

void MakerfabsDW3000::UWBTag::onEventClockSync()
{
}

void MakerfabsDW3000::UWBTag::onEventTDoAccess()
{
}

void MakerfabsDW3000::UWBTag::onEventTWRAccess()
{
}

double MakerfabsDW3000::UWBTag::executeTWR(uint16_t &target_anchor)
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

    uint8_t bufferFrame[20];
    uint8_t *pollFrame = this->generateFrame(12U, target_anchor, UWB_FUNCTION_CODE_TWR_REQUEST);

    dwt_setrxaftertxdelay(MAKERFABSDW3000_RX_AFTER_TX_DELAY_UUS);
    dwt_setrxtimeout(MAKERFABSDW3000_RX_TIMEOUT_UUS);
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(12U, pollFrame, 0);
    dwt_writetxfctrl(12U, 0, 1);
    delete[] pollFrame;

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
            return -1.0;

        dwt_readrxdata(bufferFrame, frameLen, 0);
        if (!this->validateFrame(bufferFrame))
            return -1.0;

        reqTxTs = dwt_readtxtimestamplo32();
        resRxTs = dwt_readrxtimestamplo32();
        clockOffsetRatio = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

        resp_msg_get_ts(bufferFrame + UWB_FRAME_RANGING_REQRXTS_INDEX, &reqRxTs);
        resp_msg_get_ts(bufferFrame + UWB_FRAME_RANGING_RESTXTS_INDEX, &resTxTs);

        rtdTag = resRxTs - reqTxTs;
        rtdAnc = resTxTs - reqRxTs;
        tof = ((rtdTag - rtdAnc * (1.0 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
        distance = tof * SPEED_OF_LIGHT;
        return distance;
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        return -1.0;
    }
}

bool MakerfabsDW3000::UWBTag::executeTDoA()
{
}
