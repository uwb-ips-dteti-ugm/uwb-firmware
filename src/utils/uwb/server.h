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
         * @brief   Client info including address, ranging mode, and its latest interaction timestamp with the server.
         */
        struct ClientInfo
        {
            uint16_t addr;
            RangingMode mode;
            uint64_t lastUpdate;

            /*
             * @brief Default constructor of `DW3000Server::ClientInfo`.
             */
            ClientInfo();
        };

        /*
         * @brief   Struct of TWR data to contain. Including timestamp, two addresses, and the distance.
         */
        struct ClientTWRData
        {
            uint32_t timestamp;
            uint16_t addr1;
            uint16_t addr2;
            double distance;

            /*
             * @brief Default constructor of `DW3000Server::ClientTWRData`.
             */
            ClientTWRData();
        };

        /*
         * @brief   Adds new client whenever the maximum has yet reached.
         * @param   clientAddress the client's address
         * @param   mode the client's ranging mode
         * @return  `true` if success
         */
        bool addClient(uint16_t clientAddress, RangingMode mode);

        /*
         * @brief   Deletes existing client.
         * @oaram   clientAddress the client's address
         * @return  `true` if success
         */
        bool deleteClient(uint16_t clientAddress);

        /*
         * @brief   Deletes existing client from its index in the queue.
         * @param   index its index
         * @return  `true` if success
         */
        bool deleteClientByIndex(uint8_t index);

        /*
         * @brief   Checks whether the client address exists or not.
         * @param   clientAddress the client's address
         * @return  `true` if the client does exist
         */
        bool existClient(uint16_t clientAddress);

        /*
         * @brief   Retrieves current number of clients.
         * @param   None
         * @return  number of clients
         */
        uint8_t getClientNum();

        /*
         * @brief   Routine in authorization period.
         * @param   None
         * @return  None
         */
        void authorizeRoutine();

        /*
         * @brief   Routine in network update period.
         * @param   None
         * @return  None
         */
        void networkUpdateRoutine();

        /*
         * @brief   Routine in clock synchronization period.
         * @param   None
         * @return  None
         */
        void clockSyncRoutine();

        /*
         * @brief   Routine in TDOA scheduling period.
         * @param   None
         * @return  None
         */
        void tdoaScheduleRoutine();

        /*
         * @brief   Routine in TWR scheduling period.
         * @param   None
         * @return  None
         */
        void twrScheduleRoutine();

    private:
        ClientInfo *clients;
        uint8_t clientNum;
        uint8_t clientMax;
        uint64_t clientTimeout;

        QueueHandle_t clientTWRQueue;
        uint16_t queueSize;
        uint16_t queueCnt;

        uint8_t txBuffer[127];
        uint8_t rxBuffer[127];

        /*
         * @brief   Performs TWR to a target address. This method is used because TWR is a time critical task.
         * @param   targetAddress address to serve the TWR from
         * @return  None
         */
        bool twrServe(uint16_t targetAddress);

        /*
         * @brief   Add new TWR data in the queue. The latest data will be replaced after the queue is full.
         * @param   data the data to append
         * @return  None
         */
        void appendTWRData(ClientTWRData *data);
    };
}

#endif