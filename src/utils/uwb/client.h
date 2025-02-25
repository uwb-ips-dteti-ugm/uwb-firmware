#ifndef __UWB_CLIENT_H__
#define __UWB_CLIENT_H__

#include <Arduino.h>
#include <SPI.h>
#include <DW1000.h>
#include <dw3000.h>
#include "base.h"

namespace uwbsys
{
    extern SPISettings _fastSPI;
    extern dwt_txconfig_t txconfig_options;

    class ClientDW3000 : public Base
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
        void networkConfig(uint16_t networkAddress, uint16_t deviceAddress, uint64_t timeout = 5000UL);
        /*
         * @brief
         * Set the desired ranging mode for the client.
         *
         * @param
         * mode Ranging modes: `UWB_RANGING_MODE_TDOA` or `UWB_RANGING_MODE_TWR`
         *
         * @return
         * None
         */
        void setRangingMode(uint8_t mode);
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
         * Run an iteration of the UWB client's task.
         *
         * @param
         * None
         *
         * @return
         * None
         */
        void spin();
        /*
         * @brief
         * Checks whether the client is connected to a UWB network.
         *
         * @param
         * None
         *
         * @return
         * `true` if exists, `false` otherwise `type: bool`
         */
        bool isNetworkConnected();

    private:
        struct NetworkEventParams
        {
            NetworkEvent event;
            uint16_t sourceAddress;
            size_t payloadSize;
            uint8_t *payloadPtr = nullptr;
        };

        QueueHandle_t networkEventQueue;
        dwt_config_t *dwConfig;
        uint8_t rangingMode = RANGING_MODE_NONE;
        uint16_t masterAddress;
        uint64_t timeout;
        uint64_t lastEventTimestamp = 0;
        bool networkConnected = false;

        NetworkEvent getFrameNetworkEvent(uint8_t *frame);

        void networkEventListen();
        void onEventAuthorize(NetworkEventParams *params);
        void onEventNetworkUpdate(NetworkEventParams *params);
        void onEventClockSync(NetworkEventParams *params);
        void onEventTDoAAccess(NetworkEventParams *params);
        void onEventTWRAccess(NetworkEventParams *params);
        void onEventTWRGrant(NetworkEventParams *params);
    };
}

#endif