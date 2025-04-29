#include "server.h"
using namespace uwbsys;

DW3000Server::DW3000Server(uint8_t clientMax) : DW3000Base::DW3000Base()
{
    this->clients = new uint16_t[clientMax];
    this->clientIter = 0;
    this->clientNum = 0;
    this->clientMax = clientMax;
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
}

bool DW3000Server::addClient(uint16_t clientAddress)
{
    if (this->clientNum < this->clientMax)
    {
        this->clients[this->clientNum] = clientAddress;
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
        if (this->clients[i] == clientAddress)
        {
            if (i != this->clientNum - 1)
            {
                memmove(
                    this->clients + i,
                    this->clients + i + 1,
                    sizeof(uint16_t) * (this->clientNum - 1 - i));
            }
            this->clientNum--;

            if (this->clientIter > i || this->clientIter == this->clientNum)
                this->clientIter--;

            return true;
        }
    }
    return false;
}

uint8_t DW3000Server::getClientNum()
{
    return this->clientNum;
}

uint16_t DW3000Server::nextClient()
{
    uint16_t addr = this->clients[this->clientIter];
    this->clientIter = (this->clientIter + 1) % this->clientNum;
    return addr;
}

void DW3000Server::authorizeRoutine()
{
    uint8_t frame[13];
    uint8_t buffer[13];

    this->createFrame(frame, 13, 0xFFFF, FUNCTION_CODE_AUTHORIZE);
    size_t recvSize = this->sendExpectResponse(frame, 13, buffer, 13);

    if (recvSize > 0)
    {
        for (size_t i = 0; i < recvSize; ++i)
        {
            Serial.printf("%02X ", buffer[i]);
        }
        Serial.println();
    }
}
