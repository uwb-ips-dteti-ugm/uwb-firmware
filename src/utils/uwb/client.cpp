#include "client.h"
using namespace uwbsys;

DW3000Client::DW3000Client() : DW3000Base::DW3000Base()
{
    this->eventQueue = xQueueCreate(16, sizeof(uint8_t));
    this->rangingMode = RANGING_MODE_NONE;
    this->connected = false;
}

bool DW3000Client::deviceConfig(dwt_config_t *config)
{
    return this->begin(config);
}

void DW3000Client::networkConfig(uint16_t deviceAddress, RangingMode mode)
{
    this->setDeviceAddress(deviceAddress);
    this->rangingMode = mode;
}

void DW3000Client::spin()
{
    this->listen();
}

bool DW3000Client::isConnected()
{
}

void DW3000Client::listen()
{
    static uint8_t buffer[127];
    size_t recvSize = this->receive(buffer, 127);

    if (recvSize > 0)
    {
        for (size_t i = 0; i < recvSize; ++i)
        {
            Serial.printf("%02X ", buffer[i]);
        }
        Serial.println();
    }
}

void DW3000Client::onEventAuthorize()
{
}

void DW3000Client::onEventNetworkUpdate()
{
}

void DW3000Client::onEventClockSync()
{
}

void DW3000Client::onEventTDoAAccess()
{
}

void DW3000Client::onEventTWRAccess()
{
}