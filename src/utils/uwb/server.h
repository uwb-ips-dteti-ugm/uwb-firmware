#ifndef __UWB_SERVER_H__
#define __UWB_SERVER_H__

#include <Arduino.h>
#include <SPI.h>
#include <DW1000.h>
#include <dw3000.h>
#include "base.h"

namespace uwbsys
{
    extern SPISettings _fastSPI;
    extern dwt_txconfig_t txconfig_options;

    class ServerDW3000 : public Base
    {
    public:
        /*
         * @brief
         * Set configuration for the UWB peripheral. If no argument passed, the default is used.
         *
         * @param
         * configuration Configuration struct `(dwt_config_t *)`
         *
         * @return
         * None
         *
         * @note
         * The passed struct can be a temporary scoped variable, since the value will be copied.
         */
        void deviceConfig(dwt_config_t *configuration = nullptr);
        /*
         * @brief
         * Set the UWB network configuration, including the network address and the device's address.
         *
         * @param
         * network_addr 2-bytes network address `(uint16_t)`
         * @param
         * device_addr 2-bytes device address `(uint16_t)`
         *
         * @return
         * None
         */
        void networkConfig(uint16_t networkAddress, uint16_t deviceAddress);
        /*
         * @brief
         * Start the UWB peripheral.
         *
         * @param
         * None
         *
         * @return
         * `true` if success, `false` otherwise `type: bool`
         */
        bool begin();
        /*
         * @brief
         * Run an iteration of the UWB server's task.
         *
         * @param
         * None
         *
         * @return
         * None
         */
        void spin();

    private:
        QueueHandle_t clientQueue;
        dwt_config_t *dwConfig;

        void appendClientQueue(uint16_t clientAddress);
        uint16_t popCyclicClientQueue();
        NetworkEvent getFrameNetworkEvent(uint8_t *frame);

        void execAuthorize();
        void execNetworkUpdate();
        void execClockSync();
        void execTDoAAccess();
        void execTWRAccess();
    };
}

#endif