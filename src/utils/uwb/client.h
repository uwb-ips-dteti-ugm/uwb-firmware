#ifndef __UWB_CLIENT_H__
#define __UWB_CLIENT_H__

#include <Arduino.h>
#include <SPI.h>
#include <DW1000.h>
#include <dw3000.h>
#include "base.h"

namespace uwbsys
{
    class DW3000Client : public DW3000Base
    {
    public:
        /*
         * @brief   The default constructor of the `uwbsys::DW3000Client` class.
         */
        DW3000Client();

        /*
         * @brief   Set configuration for the UWB IC. The default is used if left empty.
         * @param   config DW3000 configuration parameters
         * @return  `true` if success, otherwise `false`
         */
        bool deviceConfig(dwt_config_t *config = nullptr);

        /*
         * @brief   Set the client network configuration.
         * @param   deviceAddress 2-bytes device address
         * @param   mode the client ranging mode from `uwbsys::RangingMode`
         * @return  None
         */
        void networkConfig(uint16_t deviceAddress, RangingMode mode);

        /*
         * @brief   Run an iteration of the UWB client routine.
         * @param   None
         * @return  None
         */
        void spin();

        /*
         * @brief   Checks whether the client is connected to a UWB network.
         * @param   None
         * @return  true` if exists, `false` otherwise
         */
        bool isConnected();

    protected:
        /*
         */
        void listen();

        /*
         */
        void onEventAuthorize();

        /*
         */
        void onEventNetworkUpdate();

        /*
         */
        void onEventClockSync();

        /*
         */
        void onEventTDoAAccess();

        /*
         */
        void onEventTWRAccess();

    private:
        QueueHandle_t eventQueue;
        RangingMode rangingMode;
        bool connected;
    };

    struct ClientEventParam
    {
        uint8_t *frame;

        ClientEventParam();
        ~ClientEventParam();
    };
}

#endif