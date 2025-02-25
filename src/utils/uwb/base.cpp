#include "base.h"

void uwbsys::Base::setNetworkAddress(uint16_t networkAddress)
{
    this->networkAddress = networkAddress;
}

void uwbsys::Base::setDeviceAddress(uint16_t deviceAddress)
{
    this->deviceAddress = deviceAddress;
}

uint16_t uwbsys::Base::getNetworkAddress()
{
    return this->networkAddress;
}

uint16_t uwbsys::Base::getDeviceAddress()
{
    return this->deviceAddress;
}

size_t uwbsys::Base::generateFrame(uint8_t *ptr, uint16_t &destinationAddress, uint8_t functionCode, uint8_t payloadLength, uint8_t *payload)
{
    size_t totalLength = 13U + (size_t)payloadLength;
    if (totalLength > 127U)
        return 0;

    ptr = new uint8_t[totalLength];
    memset(ptr, 0x00, totalLength);

    ptr[0] = 0x41;
    ptr[1] = 0x88;
    ptr[UWB_FRAME_INDEX_FUNCTION_CODE] = functionCode;
    ptr[UWB_FRAME_INDEX_PAYLOAD] = payloadLength;
    memcpy(ptr + UWB_FRAME_INDEX_NETWORK_ADDRESS, &this->networkAddress, sizeof(uint16_t));
    memcpy(ptr + UWB_FRAME_INDEX_DESTINATION_ADDRESS, &destinationAddress, sizeof(uint16_t));
    memcpy(ptr + UWB_FRAME_INDEX_SOURCE_ADDRESS, &this->deviceAddress, sizeof(uint16_t));

    if (payload != nullptr)
        memcpy(ptr + UWB_FRAME_INDEX_PAYLOAD + 1, payload, (size_t)payloadLength);

    return totalLength;
}

uint16_t uwbsys::Base::getFrameNetworkAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(&retVal, frame + UWB_FRAME_INDEX_NETWORK_ADDRESS, sizeof(uint16_t));
    return retVal;
}

uint16_t uwbsys::Base::getFrameDestinationAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(&retVal, frame + UWB_FRAME_INDEX_DESTINATION_ADDRESS, sizeof(uint16_t));
    return retVal;
}

uint16_t uwbsys::Base::getFrameSourceAddress(uint8_t *frame)
{
    uint16_t retVal;
    memcpy(&retVal, frame + UWB_FRAME_INDEX_SOURCE_ADDRESS, sizeof(uint16_t));
    return retVal;
}

uint8_t uwbsys::Base::getFrameFunctionCode(uint8_t *frame)
{
    return frame[UWB_FRAME_INDEX_FUNCTION_CODE];
}

size_t uwbsys::Base::getFramePayload(uint8_t *ptr, uint8_t *frame)
{
    size_t payloadLength = (size_t)frame[UWB_FRAME_INDEX_PAYLOAD];
    if (payloadLength == 0)
    {
        ptr = nullptr;
        return 0;
    }
    else
    {
        ptr = new uint8_t[payloadLength];
        memcpy(ptr, frame + UWB_FRAME_INDEX_PAYLOAD + 1, payloadLength);
        return payloadLength;
    }
}

bool uwbsys::Base::validateFrame(uint8_t *frame)
{
    if (!(frame[0] == 0x41 && frame[1] == 0x88))
        return false;

    uint16_t tempAddress = this->getFrameNetworkAddress(frame);
    if (networkAddress != this->networkAddress)
        return false;

    tempAddress = this->getFrameDestinationAddress(frame);
    if (!((tempAddress == this->deviceAddress) || (tempAddress == 0xFFFFU)))
        return false;

    return true;
}
