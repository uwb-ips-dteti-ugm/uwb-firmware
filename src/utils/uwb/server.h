#ifndef __UWB_SERVER_H__
#define __UWB_SERVER_H__

#include <Arduino.h>
#include "base.h"

namespace uwbsys
{
    class DW3000Server : public DW3000Base
    {
    public:
        /*
         * @brief   The constructor of the `uwbsys::DW3000Server` class.
         * @param   clientMax the maximum number of client will be handled
         */
        DW3000Server(uint8_t clientMax);

        /*
         * @brief   Set configuration for the UWB IC. The default is used if left empty.
         * @param   config DW3000 configuration parameters
         * @return  `true` if success, otherwise `false`
         */
        bool deviceConfig(dwt_config_t *config = nullptr);

        /*
         * @brief   Set the UWB network configuration.
         * @param   networkAddress 2-bytes network address
         * @param   deviceAddress 2-bytes device address
         * @return  None
         */
        void networkConfig(uint16_t networkAddress, uint16_t deviceAddress);

        /*
         * @brief   Run an iteration of the UWB server routine.
         * @param   None
         * @return  None
         */
        void spin();

    protected:
        /*
         */
        bool addClient(uint16_t clientAddress);

        /*
         */
        bool deleteClient(uint16_t clientAddress);

        /*
         */
        uint8_t getClientNum();

        /*
         */
        uint16_t nextClient();

        /*
         */
        void authorizeRoutine();

    private:
        uint16_t *clients;
        uint8_t clientIter;
        uint8_t clientNum;
        uint8_t clientMax;
    };
}

#endif