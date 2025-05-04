#define __SERVER__
// #define __CLIENT__

// #define CLIENT_ADDR 0x1111
#define CLIENT_ADDR 0x2222

#ifdef __SERVER__
#include "utils/uwb/server.h"

uwbsys::DW3000Server server = uwbsys::DW3000Server(16);

void setup()
{
    Serial.begin(115200);
    Serial.println("UWB SERVER");

    if (!server.deviceConfig())
    {
        Serial.println("DEVICE CONFIG FAILED");
        while (1)
        {
        }
    }

    server.networkConfig(0x0001, 0xAABB);
}

void loop()
{
    server.spin();
    delay(10);
}
#endif

#ifdef __CLIENT__
#include "utils/uwb/client.h"

uwbsys::DW3000Client client = uwbsys::DW3000Client();

void setup()
{
    Serial.begin(115200);
    Serial.println("UWB CLIENT");

    if (!client.deviceConfig())
    {
        Serial.println("DEVICE CONFIG FAILED");
        while (1)
        {
        }
    }

    client.networkConfig(CLIENT_ADDR, uwbsys::RANGING_MODE_TWR);
}

void loop()
{
    client.spin();
}
#endif