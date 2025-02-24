#ifndef __UTILS_TAG_H__
#define __UTILS_TAG_H__

#include <Arduino.h>
#include <SPI.h>
#include <dw3000.h>
#include "defines.h"

#define MAKERFABSDW3000_PIN_RST 27
#define MAKERFABSDW3000_PIN_IRQ 34
#define MAKERFABSDW3000_PIN_SS 4
#define MAKERFABSDW3000_RX_ANTENNA_DELAY_UUS 16385
#define MAKERFABSDW3000_TX_ANTENNA_DELAY_UUS 16385
#define MAKERFABSDW3000_RX_AFTER_TX_DELAY_UUS 240
#define MAKERFABSDW3000_RX_TIMEOUT_UUS 400
#define MAKERFABSDW3000_FRAME_MAX_SIZE 127

namespace MakerfabsDW3000
{
    class UWBTag
    {
    public:
        enum RangingMode : uint8_t
        {
            RANGING_MODE_UNSET = 0x00,
            RANGING_MODE_TDOA = 0x01,
            RANGING_MODE_TWR = 0x02
        };

        /*
         * @brief
         * Set configuration for the UWB peripheral. If no argument passed, the default is used.
         *
         * @param
         * configuration Configuration struct `(dwt_config_t *)`
         *
         * @return
         * None.
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
         * None.
         */
        void networkConfig(uint16_t network_addr, uint16_t device_addr);
        /*
         * @brief
         * Set the desired ranging mode for the tag.
         *
         * @param
         * mode Ranging mode types: `MakerfabsDW3000::UWBTag::RANGING_MODE_TDOA` or `MakerfabsDW3000::UWBTag::RANGING_MODE_TWR`
         *
         * @return
         * None.
         */
        void setRangingMode(RangingMode mode);
        /*
         * @brief
         * Start the UWB peripheral.
         *
         * @param
         * None
         *
         * @return
         * A boolean value, `true` if success, `false` otherwise.
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
         * None.
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
         * A boolean value, `true` if exists, `false` otherwise.
         */
        bool isNetworkConnected();
        /*
         * @brief
         * Checks whether new ranging result exists (only for TWR).
         *
         * @param
         * None
         *
         * @return
         * A boolean value, `true` if exists, `false` otherwise.
         */
        bool isRangeUpdated();
        /*
         * @brief
         * Checks whether new position result exists (only for TDoA).
         *
         * @param
         * None
         *
         * @return
         * A boolean value, `true` if exists, `false` otherwise.
         */
        bool isPositionUpdated();
        /*
         * @brief
         * Get the incoming ranging value and the anchor's address.
         *
         * @param
         * anchor_addr Buffer for receiving anchor address `(uint16_t *)`
         *
         * @return
         * A float value in meters.
         *
         * @note
         * Please check with isRangeUpdated() before using this method.
         */
        float getRange(uint16_t *anchor_addr);
        /*
         * @brief
         * Get the position values.
         *
         * @param
         * coordinate Buffer for receiving the position coordinates `(float[3])`
         *
         * @return
         * None.
         *
         * @note
         * Please check with isPositionUpdated() before using this method.
         */
        void getPosition(float *coordinate);

    private:
        enum UWBNetworkEvent : uint8_t
        {
            EVENT_NONE,
            EVENT_AUTHORIZE,
            EVENT_NETWORK_UPDATE,
            EVENT_CLOCK_SYNC,
            EVENT_TDOA_ACCESS,
            EVENT_TWR_ACCESS
        };

        struct UWBNetworkEventInfo
        {
            UWBNetworkEvent event;
            uint16_t sourceAddress;
            uint8_t query[4];
        };

        QueueHandle_t networkEventQueue;
        dwt_config_t *dwConfig;
        RangingMode rangingMode = RANGING_MODE_UNSET;
        uint16_t networkAddress;
        uint16_t deviceAddress;
        uint16_t masterAddress;

        bool networkConnected = false;
        bool rangeUpdated = false;
        bool positionUpdated = false;

        uint16_t rangingFrom;
        double rangingResult = 0.0;
        double positionResult[3] = {0.0, 0.0, 0.0};

        uint8_t *generateFrame(size_t frameLen, uint16_t &destination_addr, uint8_t function_code);
        bool validateFrame(uint8_t *frame);
        uint16_t getFrameNetworkAddress(uint8_t *frame);
        uint16_t getFrameDestinationAddress(uint8_t *frame);
        uint16_t getFrameSourceAddress(uint8_t *frame);
        UWBNetworkEvent getFrameEvent(uint8_t *frame);

        void statusHandle();
        void networkEventListen();
        void onEventAuthorize(uint16_t &destination_addr);
        void onEventNetworkUpdate();
        void onEventClockSync();
        void onEventTDoAccess();
        void onEventTWRAccess();

        double executeTWR(uint16_t &target_anchor);
        bool executeTDoA();
    };

    extern dwt_txconfig_t txconfig_options;
}

#endif