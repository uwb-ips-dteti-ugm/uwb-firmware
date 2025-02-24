#ifndef __UWB_TAG_H__
#define __UWB_TAG_H__

#include <Arduino.h>
#include <DW1000.h>
#include <dw3000.h>
#include "base.h"

namespace uwbsys
{
    extern dwt_txconfig_t txconfig_options;

    class TagDW3000 : public Base
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
         * Set the desired ranging mode for the tag.
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
         * Run an iteration of the UWB tag's task.
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
         * Checks whether the tag is connected to a UWB network.
         *
         * @param
         * None
         *
         * @return
         * `true` if exists, `false` otherwise `type: bool`
         */
        bool isNetworkConnected();

    private:
        enum NetworkEvent : uint8_t
        {
            NETWORK_EVENT_NONE,
            NETWORK_EVENT_AUTHORIZE,
            NETWORK_EVENT_NETWORK_UPDATE,
            NETWORK_EVENT_CLOCK_SYNC,
            NETWORK_EVENT_TDOA_ACCESS,
            NETWORK_EVENT_TWR_ACCESS
        };

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
        uint16_t networkAddress;
        uint16_t deviceAddress;
        uint16_t masterAddress;

        bool networkConnected = false;

        NetworkEvent getFrameNetworkEvent(uint8_t *frame);

        void networkEventListen();
        void onEventAuthorize(NetworkEventParams *params);
        void onEventNetworkUpdate(NetworkEventParams *params);
        void onEventClockSync(NetworkEventParams *params);
        void onEventTDoAccess(NetworkEventParams *params);
        void onEventTWRAccess(NetworkEventParams *params);
    };
}

#endif