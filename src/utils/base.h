#ifndef __UWB_BASE_H__
#define __UWB_BASE_H__

#include <Arduino.h>
#include "defines.h"

namespace uwbsys
{
    class Base
    {
    protected:
        uint16_t networkAddress;
        uint16_t deviceAddress;

        /*
         * @brief
         * Set the 2-bytes network address.
         *
         * @param
         * networkAddress 2-bytes network address `type: uint16_t`
         *
         * @return
         * None
         */
        void setNetworkAddress(uint16_t networkAddress);
        /*
         * @brief
         * Set the 2-bytes device address.
         *
         * @param
         * networkAddress 2-bytes device address `type: uint16_t`
         *
         * @return
         * None
         */
        void setDeviceAddress(uint16_t deviceAddress);
        /*
         * @brief
         * Get the 2-bytes network address.
         *
         * @param
         * None
         *
         * @return
         * The 2-bytes network address `type: uint16_t`
         */
        uint16_t getNetworkAddress();
        /*
         * @brief
         * Get the 2-bytes device address.
         *
         * @param
         * None
         *
         * @return
         * The 2-bytes device address `type: uint16_t`
         */
        uint16_t getDeviceAddress();
        /*
         * @brief
         * Generate a UWB frame following the IEEE 802.15.4 standard.
         *
         * @param
         * ptr for referencing the resulted frame `type: uint8_t *`
         * @param
         * destinationAddress 2-bytes target address `type: uint16_t`
         * @param
         * functionCode 1-byte function code, use UWB_FUNCTION_CODE_{function} definition to help `type: uint8_t`
         * @param
         * payloadLength the length of the payload must be less than 114 (127-bytes total, 10-bytes header, 1-byte payload length, 2-bytes zero byte) `type: uint8_t`
         * @param
         * payload the payload array `type: uint8_t *`
         *
         * @return
         * The size of the generated frame `type: size_t`
         *
         * @note
         * Make sure to call `setNetworkAddress()` and `setDeviceAddress()` before calling this method.
         * @note
         * The returned frame should be deleted after use with `delete[]` operator.
         */
        size_t generateFrame(uint8_t *ptr, uint16_t &destinationAddress, uint8_t functionCode, uint8_t payloadLength = 0x00, uint8_t *payload = nullptr);
        /*
         * @brief
         * Get the network address defined inside a frame.
         *
         * @param
         * frame the pointer to the desired frame `type: uint8_t *`
         *
         * @return
         * The network address defined inside a frame `type: uint16_t`
         */
        uint16_t getFrameNetworkAddress(uint8_t *frame);
        /*
         * @brief
         * Get the destination address defined inside a frame.
         *
         * @param
         * frame the pointer to the desired frame `type: uint8_t *`
         *
         * @return
         * The destination address defined inside a frame `type: uint16_t`
         */
        uint16_t getFrameDestinationAddress(uint8_t *frame);
        /*
         * @brief
         * Get the source address defined inside a frame.
         *
         * @param
         * frame the pointer to the desired frame `type: uint8_t *`
         *
         * @return
         * The source address defined inside a frame `type: uint16_t`
         */
        uint16_t getFrameSourceAddress(uint8_t *frame);
        /*
         * @brief
         * Get the function code defined inside a frame.
         *
         * @param
         * frame the pointer to the desired frame `type: uint8_t *`
         *
         * @return
         * The function code defined inside a frame `type: uint16_t`
         */
        uint8_t getFrameFunctionCode(uint8_t *frame);
        /*
         * @brief
         * Get the payload defined inside a frame (not including the payload's length byte).
         *
         * @param
         * ptr the pointer for referencing the payload `type: uint8_t *`
         * @param
         * frame the pointer to the desired frame `type: uint8_t *`
         *
         * @return
         * The length of the payload `type: size_t`
         *
         * @note
         * The returned frame should be deleted after use with `delete[]` operator.
         */
        size_t getFramePayload(uint8_t *ptr, uint8_t *frame);
        /*
         * @brief
         * Validate the frame based on the included frame control, network address, and destination address.
         *
         * @param
         * frame the pointer to the desired frame `type: uint8_t *`
         *
         * @return
         * The function code defined inside a frame `type: uint16_t`
         */
        bool validateFrame(uint8_t *frame);
    };
}

#endif