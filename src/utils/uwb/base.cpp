#include "base.h"

SPISettings uwbsys::_fastSPI;
dwt_txconfig_t uwbsys::txconfig_options;

uwbsys::Base::Base()
{
    this->networkAddress = 0xFFFF;
    this->deviceAddress = 0xFFFF;
    this->operationMode = uwbsys::OPERATION_MODE_NONE;
}

void uwbsys::Base::setNetworkAddress(uint16_t networkAddress)
{
    this->networkAddress = networkAddress;
}

void uwbsys::Base::setDeviceAddress(uint16_t deviceAddress)
{
    this->deviceAddress = deviceAddress;
}

void uwbsys::Base::setOperationMode(uwbsys::OperationMode operationMode)
{
    this->operationMode = operationMode;
}

uint16_t uwbsys::Base::getNetworkAddress()
{
    return this->networkAddress;
}

uint16_t uwbsys::Base::getDeviceAddress()
{
    return this->deviceAddress;
}

uwbsys::OperationMode uwbsys::Base::getOperationMode()
{
    return this->operationMode;
}

size_t uwbsys::Base::createFrame(uint8_t *buffer, size_t bufferSize, uint16_t destinationAddress, uint8_t functionCode, uint8_t payloadLength, uint8_t *payload)
{
    size_t totalLength = 13 + (size_t)payloadLength;
    if (totalLength > 127 || totalLength > bufferSize)
        return 0;

    buffer[0] = 0x41;
    buffer[1] = 0x88;
    buffer[totalLength - 1] = 0x00;
    buffer[totalLength - 2] = 0x00;
    buffer[uwbsys::FRAME_INDEX_FUNCTION_CODE] = functionCode;
    buffer[uwbsys::FRAME_INDEX_PAYLOAD] = payloadLength;
    memcpy(buffer + uwbsys::FRAME_INDEX_NETWORK_ADDRESS, &this->networkAddress, sizeof(uint16_t));
    memcpy(buffer + uwbsys::FRAME_INDEX_DESTINATION_ADDRESS, &destinationAddress, sizeof(uint16_t));
    memcpy(buffer + uwbsys::FRAME_INDEX_SOURCE_ADDRESS, &this->deviceAddress, sizeof(uint16_t));

    if (payload != nullptr)
        memcpy(buffer + uwbsys::FRAME_INDEX_PAYLOAD + 1, payload, (size_t)payloadLength);

    return totalLength;
}

uint16_t uwbsys::Base::getFrameNetworkAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(&retVal, frame + uwbsys::FRAME_INDEX_NETWORK_ADDRESS, sizeof(uint16_t));
    return retVal;
}

uint16_t uwbsys::Base::getFrameDestinationAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(&retVal, frame + uwbsys::FRAME_INDEX_DESTINATION_ADDRESS, sizeof(uint16_t));
    return retVal;
}

uint16_t uwbsys::Base::getFrameSourceAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(&retVal, frame + uwbsys::FRAME_INDEX_SOURCE_ADDRESS, sizeof(uint16_t));
    return retVal;
}

uint8_t uwbsys::Base::getFrameFunctionCode(uint8_t *frame)
{
    return frame[uwbsys::FRAME_INDEX_FUNCTION_CODE];
}

size_t uwbsys::Base::getFramePayload(uint8_t *buffer, size_t bufferSize, uint8_t *frame)
{
    size_t payloadLength = (size_t)frame[uwbsys::FRAME_INDEX_PAYLOAD];
    if (payloadLength == 0 || payloadLength > bufferSize)
        return 0;
    else
    {
        memcpy(buffer, frame + uwbsys::FRAME_INDEX_PAYLOAD + 1, payloadLength);
        return payloadLength;
    }
}

bool uwbsys::Base::validateFrame(uint8_t *frame)
{
    if (!(frame[0] == 0x41 && frame[1] == 0x88))
        return false;

    uint16_t tempAddress = this->getFrameNetworkAddress(frame);
    if (tempAddress != this->networkAddress)
        return false;

    tempAddress = this->getFrameDestinationAddress(frame);
    if ((tempAddress != this->deviceAddress) && (tempAddress != 0xFFFF))
        return false;

    return true;
}

uwbsys::DW3000Base::DW3000Base() : uwbsys::Base()
{
    this->dwConfig = new dwt_config_t;
    this->dwConfig->chan = DW3000_CHANNEL;
    this->dwConfig->txPreambLength = DW3000_TX_PREAMBLE_LENGTH;
    this->dwConfig->rxPAC = DW3000_RX_PAC;
    this->dwConfig->txCode = DW3000_TX_CODE;
    this->dwConfig->rxCode = DW3000_RX_CODE;
    this->dwConfig->sfdType = DW3000_SFD_TYPE;
    this->dwConfig->dataRate = DW3000_DATA_RATE;
    this->dwConfig->phrMode = DW3000_PHR_MODE;
    this->dwConfig->phrRate = DW3000_PHR_RATE;
    this->dwConfig->sfdTO = (129 + 8 - 8);
    this->dwConfig->stsMode = DW3000_STS_MODE;
    this->dwConfig->stsLength = DW3000_STS_LENGTH;
    this->dwConfig->pdoaMode = DW3000_PDOA_MODE;

    this->seqCnt = 0;
    this->statusReg = 0;
    this->rxHeld = false;
}

bool uwbsys::DW3000Base::begin(dwt_config_t *config)
{
    if (config != nullptr)
        memcpy(this->dwConfig, config, sizeof(dwt_config_t));

    uwbsys::_fastSPI = SPISettings(16000000L, MSBFIRST, SPI_MODE0);
    spiBegin(DW3000_PIN_IRQ, DW3000_PIN_RST);
    spiSelect(DW3000_PIN_SS);
    vTaskDelay(pdMS_TO_TICKS(2));

    if (!dwt_checkidlerc())
        return false;

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
        return false;

    if (dwt_configure(this->dwConfig) == DWT_ERROR)
        return false;

    dwt_configuretxrf(&uwbsys::txconfig_options);
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
    dwt_setrxantennadelay(DW3000_RX_ANTENNA_DELAY_UUS);
    dwt_settxantennadelay(DW3000_TX_ANTENNA_DELAY_UUS);
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
    return true;
}

bool uwbsys::DW3000Base::send(uint8_t *frame, size_t frameSize, bool isRanging)
{
    frame[uwbsys::FRAME_INDEX_SEQUENCE_NUMBER] = this->seqCnt++;

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(frameSize, frame, 0);
    dwt_writetxfctrl(frameSize, 0, (isRanging ? 1 : 0));

    if (dwt_starttx(DWT_START_TX_IMMEDIATE) == DWT_SUCCESS)
    {
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
        {
        }
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        return true;
    }
    else
    {
        return false;
    }
}

bool uwbsys::DW3000Base::sendDelayed(uint8_t *frame, size_t frameSize, uint32_t delay, bool isRanging)
{
    frame[uwbsys::FRAME_INDEX_SEQUENCE_NUMBER] = this->seqCnt++;

    dwt_setdelayedtrxtime(delay);
    dwt_writetxdata(frameSize, frame, 0);
    dwt_writetxfctrl(frameSize, 0, (isRanging ? 1 : 0));

    if (dwt_starttx(DWT_START_TX_DELAYED) == DWT_SUCCESS)
    {
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
        {
        };
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        return true;
    }
    else
    {
        return false;
    }
}

size_t uwbsys::DW3000Base::sendExpectResponse(uint8_t *frame, size_t frameSize, uint8_t *buffer, size_t bufferSize, bool isRanging, uint32_t rxOnTime, uint32_t rxTimeout)
{
    frame[uwbsys::FRAME_INDEX_SEQUENCE_NUMBER] = this->seqCnt++;

    dwt_setrxaftertxdelay(rxOnTime);
    dwt_setrxtimeout(rxTimeout);
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(frameSize, frame, 0);
    dwt_writetxfctrl(frameSize, 0, (isRanging ? 1 : 0));
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    while (!((this->statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    }

    if (this->statusReg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        size_t frameLength = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

        if (frameLength <= bufferSize)
        {
            dwt_readrxdata(buffer, frameLength, 0);
            return frameLength;
        }
    }
    else
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);

    return 0;
}

size_t uwbsys::DW3000Base::receive(uint8_t *buffer, size_t bufferSize, uint32_t timeout)
{
    dwt_setrxtimeout(timeout);
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    while (!((this->statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    };

    if (this->statusReg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        size_t frameLength = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

        if (frameLength <= bufferSize)
        {
            dwt_readrxdata(buffer, frameLength, 0);
            return frameLength;
        }
    }
    else
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);

    return 0;
}

void uwbsys::DW3000Base::receiveHold()
{
    if (!this->rxHeld)
    {
        dwt_setrxtimeout(0);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        this->rxHeld = true;
    }
}

bool uwbsys::DW3000Base::receiveAvailable()
{
    if (!((this->statusReg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR)))
    {
        return false;
    }
    else if (this->statusReg & SYS_STATUS_RXFCG_BIT_MASK)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
        this->rxHeld = false;
        return true;
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        this->rxHeld = false;
        return false;
    }
}

size_t uwbsys::DW3000Base::receiveCollect(uint8_t *buffer, size_t bufferSize)
{
    size_t frameLength = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

    if (frameLength <= bufferSize)
    {
        dwt_readrxdata(buffer, frameLength, 0);
        return frameLength;
    }
    else
        return 0;
}