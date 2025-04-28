#ifndef __UWB_BASE_H__
#define __UWB_BASE_H__

#include <Arduino.h>
#include <SPI.h>
#include <DW1000.h>
#include <dw3000.h>
#include "config.h"
#include "enum.h"

namespace uwbsys
{
    class Base
    {
    public:
        /*
         * @brief    Default constructor of the `uwbsys::Base` class.
         */
        Base();

    protected:
        /*
         * @brief   Set the 2-bytes network address.
         * @param   networkAddress 2-bytes network address
         * @return  None
         */
        void setNetworkAddress(uint16_t networkAddress);

        /*
         * @brief   Set the 2-bytes device address.
         * @param   networkAddress 2-bytes device address
         * @return  None
         */
        void setDeviceAddress(uint16_t deviceAddress);

        /*
         * @brief   Set the UWB operation mode (server/client).
         * @param   operationMode the operation mode
         * @return  None
         */
        void setOperationMode(OperationMode operationMode);

        /*
         * @brief   Get the 2-bytes network address.
         * @param   None
         * @return  The 2-bytes network address
         */
        uint16_t getNetworkAddress();

        /*
         * @brief   Get the 2-bytes device address.
         * @param   None
         * @return  The 2-bytes device address
         */
        uint16_t getDeviceAddress();

        /*
         * @brief   Get the UWB operation mode (server/client).
         * @param   None
         * @return  The operation mode
         */
        OperationMode getOperationMode();

        /*
         * @brief   Generate a UWB frame following the IEEE 802.15.4 standard.
         * @param   buffer for storing the yielded frame
         * @param   bufferSize the size of the buffer
         * @param   destinationAddress 2-bytes target address
         * @param   functionCode 1-byte function code, use UWB_FUNCTION_CODE_{function} definition to help
         * @param   payloadLength the length of the payload must be less than 114 (127-bytes total, 10-bytes header, 1-byte payload length, 2-bytes zero byte)
         * @param   payload the payload array
         * @return  The size of the created frame
         * @note    Make sure to call `setNetworkAddress()` and `setDeviceAddress()` before calling this method.
         * @note    The sequence number is still zero.
         */
        size_t createFrame(uint8_t *buffer, size_t bufferSize, uint16_t destinationAddress, uint8_t functionCode, uint8_t payloadLength = 0x00, uint8_t *payload = nullptr);

        /*
         * @brief   Get the network address defined inside a frame.
         * @param   frame the pointer to the desired frame
         * @return  The network address defined inside a frame
         */
        uint16_t getFrameNetworkAddress(uint8_t *frame);

        /*
         * @brief   Get the destination address defined inside a frame.
         * @param   frame the pointer to the desired frame
         * @return  The destination address defined inside a frame
         */
        uint16_t getFrameDestinationAddress(uint8_t *frame);

        /*
         * @brief   Get the source address defined inside a frame.
         * @param   frame the pointer to the desired frame
         * @return  The source address defined inside a frame
         */
        uint16_t getFrameSourceAddress(uint8_t *frame);

        /*
         * @brief   Get the function code defined inside a frame.
         * @param   frame the pointer to the desired frame
         * @return  The function code defined inside a frame
         */
        uint8_t getFrameFunctionCode(uint8_t *frame);

        /*
         * @brief   Get the payload defined inside a frame (not including the payload's length byte).
         * @param   buffer the buffer to contain the payload
         * @param   bufferSize the size of the buffer
         * @param   frame the pointer to the desired frame
         * @return  The length of the payload
         */
        size_t getFramePayload(uint8_t *buffer, size_t bufferSize, uint8_t *frame);

        /*
         * @brief   Validate the frame based on the included frame control, network address, and destination address.
         * @param   frame the pointer to the desired frame
         * @return  The function code defined inside a frame
         */
        bool validateFrame(uint8_t *frame);

    private:
        uint16_t networkAddress;
        uint16_t deviceAddress;
        OperationMode operationMode;
    };

    class DW3000Base : public Base
    {
    public:
        /*
         * @brief    Default constructor of the `uwbsys::DW3000Base` class.
         */
        DW3000Base();

    protected:
        /*
         * @brief   Initiate the connection to the DW3000. The default is used if left empty.
         * @param   cfg DW3000 configuration
         * @return  `true` if success, otherwise `false`
         */
        bool begin(dwt_config_t *config = nullptr);

        /*
         * @brief   Send an UWB frame directly.
         * @param   frame the frame to be transmitted
         * @param   frameSize the size of the frame
         * @param   isRanging ranging frame or not
         * @return  `true` if success, otherwise `false`
         */
        bool send(uint8_t *frame, size_t frameSize, bool isRanging = false);

        /*
         * @brief   Send an UWB frame with delay.
         * @param   frame the frame to be transmitted
         * @param   frameSize the size of the frame
         * @param   delay the delay in DW3000 time unit
         * @param   isRanging ranging frame or not
         * @return  `true` if success, otherwise `false`
         * @note    To convert microseconds to DW3000 time unit, multiply with `UUS_TO_DWT_TIME = 63898`
         */
        bool sendDelayed(uint8_t *frame, size_t frameSize, uint32_t delay, bool isRanging = false);

        /*
         * @brief   Send an UWB frame and wait for an immediate response (within 1ms).
         * @param   frame the frame to be transmitted
         * @param   frameSize the size of the frame
         * @param   buffer the buffer to contain the response
         * @oaram   bufferSize the size of the buffer
         * @param   isRanging ranging frame or not
         * @param   rxOnTime time (in microseconds) to turn on the receiver after the frame is sent
         * @param   rxTimeout time (in microseconds) until timeout
         * @return  The size of received frame
         */
        size_t sendExpectResponse(uint8_t *frame, size_t frameSize, uint8_t *buffer, size_t bufferSize, bool isRanging = false, uint32_t rxOnTime = 1, uint32_t rxTimeout = 1000);

        /*
         * @brief   Receive an UWB frame. Set the timeout to 0 to wait for frame indefinitely.
         * @param   buffer the buffer to contain the frame
         * @param   bufferSize the size of the buffer
         * @param   timeout time (in microseconds) until timeout
         * @return  The size of received frame
         */
        size_t receive(uint8_t *buffer, size_t bufferSize, uint32_t timeout = 0);

    private:
        dwt_config_t *dwConfig;
        uint8_t seqCnt;
        uint32_t statusReg;
    };

    extern SPISettings _fastSPI;
    extern dwt_txconfig_t txconfig_options;
}

#endif