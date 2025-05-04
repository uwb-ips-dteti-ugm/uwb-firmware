#ifndef __UWB_SERVER_H__
#define __UWB_SERVER_H__

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
        DW3000Server(uint8_t clientMax, uint64_t timeout = 5000, uint16_t queueSize = 16);

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
        struct ClientInfo
        {
            uint16_t addr;
            RangingMode mode;
            uint64_t lastUpdate;

            /*
             */
            ClientInfo();
        };

        struct ClientTWRData
        {
            uint32_t timestamp;
            uint16_t addr1;
            uint16_t addr2;
            double distance;

            /*
             */
            ClientTWRData();
        };

        /*
         */
        bool addClient(uint16_t clientAddress, RangingMode mode);

        /*
         */
        bool deleteClient(uint16_t clientAddress);

        /*
         */
        bool deleteClientByIndex(uint8_t index);

        /*
         */
        bool existClient(uint16_t clientAddress);

        /*
         */
        ClientInfo nextClient();

        /*
         */
        uint8_t getClientNum();

        /*
         */
        void authorizeRoutine();

        /*
         */
        void networkUpdateRoutine();

        /*
         */
        void clockSyncRoutine();

        /*
         */
        void tdoaScheduleRoutine();

        /*
         */
        void twrScheduleRoutine();

    private:
        ClientInfo *clients;
        uint8_t clientIter;
        uint8_t clientNum;
        uint8_t clientMax;
        uint64_t clientTimeout;

        QueueHandle_t clientTWRQueue;
        uint16_t queueSize;
        uint16_t queueCnt;

        uint8_t txBuffer[127];
        uint8_t rxBuffer[127];

        /*
         * @brief Perform TWR to a target address. This method is used because TWR is a time critical task.
         * @param targetAddress address to target
         * @return None
         */
        bool twrServe(uint16_t targetAddress);

        /*
         */
        void appendTWRData(ClientTWRData *data);
    };
}

#endif