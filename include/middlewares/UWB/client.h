#ifndef __UWB_CLIENT_H__
#define __UWB_CLIENT_H__

#include "base.h"

namespace uwb
{
    class DW3000Client : public DW3000Base
    {
    public:
        /*
         * @brief   The default constructor of the `uwbsys::DW3000Client` class.
         * @param   timeout the time (in milliseconds) until disconnected from network
         */
        DW3000Client(uint64_t timeout = 5000);

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
         * @brief   Listens to any incoming UWB frame.
         * @param   None
         * @return  None
         */
        void listen();

        /*
         * @brief   Callback on authorization event.
         * @param   None
         * @return  None
         */
        void onEventAuthorize();

        /*
         * @brief   Callback on network update event.
         * @param   None
         * @return  None
         */
        void onEventNetworkUpdate();

        /*
         * @brief   Callback on clock synchronization event.
         * @param   None
         * @return  None
         */
        void onEventClockSync();

        /*
         * @brief   Callback on TDOA schedule event.
         * @param   None
         * @return  None
         */
        void onEventTDOASchedule();

        /*
         * @brief   Callback on TWR schedule event.
         * @param   None
         * @return  None
         */
        void onEventTWRSchedule();

        /*
         * @brief   Callback on TWR access event.
         * @param   None
         * @return  None
         */
        void onEventTWRAccess();

        /*
         * @brief   Handles timeout.
         * @param   None
         * @return  None
         */
        void timeoutHandle();

    private:
        RangingMode rangingMode;
        bool connected;
        uint16_t serverAddress;
        uint64_t connTimeout;
        uint64_t connTimeoutTs;
        uint8_t txBuffer[127];
        uint8_t rxBuffer[127];

        /*
         * @brief   Performs TWR to a target address. This method is used because TWR is a time critical task.
         * @param targetAddress address to target
         * @return None
         */
        bool twrServe(uint16_t targetAddress);
    };
}

#endif