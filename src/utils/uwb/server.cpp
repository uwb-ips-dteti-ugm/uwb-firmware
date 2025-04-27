#include "server.h"
using namespace uwbsys;

void ServerDW3000::deviceConfig(dwt_config_t *configuration)
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

void ServerDW3000::networkConfig(uint16_t networkAddress, uint16_t deviceAddress)
{
    this->setNetworkAddress(networkAddress);
    this->setDeviceAddress(deviceAddress);
}

bool ServerDW3000::begin()
{
    this->clientQueue = xQueueCreate(UWB_NETWORK_MAX_CLIENT_NUM, sizeof(uint16_t));

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

void ServerDW3000::spin()
{
    this->execAuthorize();
    this->execNetworkUpdate();
    this->execClockSync();
    this->execTDoAAccess();
    this->execTWRAccess();
}

void ServerDW3000::appendClientQueue(uint16_t clientAddress)
{
    uint16_t clientAddr = clientAddress;
    xQueueSend(this->clientQueue, &clientAddr, portMAX_DELAY);
}

uint16_t ServerDW3000::popCyclicClientQueue()
{
    uint16_t clientAddr;
    xQueueReceive(this->clientQueue, &clientAddr, portMAX_DELAY);
    xQueueSend(this->clientQueue, &clientAddr, portMAX_DELAY);
    return clientAddr;
}

Base::NetworkEvent ServerDW3000::getFrameNetworkEvent(uint8_t *frame)
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

void ServerDW3000::execAuthorize()
{
}

void ServerDW3000::execNetworkUpdate()
{
}

void ServerDW3000::execClockSync()
{
}

void ServerDW3000::execTDoAAccess()
{
}

void ServerDW3000::execTWRAccess()
{
}
