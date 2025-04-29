#include "utils/uwb/server.h"

uwbsys::DW3000Server server = uwbsys::DW3000Server(16);

void setup()
{
    Serial.begin(115200);

    if (!server.deviceConfig())
    {
        Serial.println("DEVICE CONFIG FAILED");
        while (1)
        {
        }
    }

    server.networkConfig(0xDEAB, 0x0001);
}

void loop()
{
    server.spin();
    delay(1000);
}

#include "utils/uwb/client.h"

uwbsys::DW3000Client client = uwbsys::DW3000Client();

void setup()
{
    Serial.begin(115200);

    if (!client.deviceConfig())
    {
        Serial.println("DEVICE CONFIG FAILED");
        while (1)
        {
        }
    }

    client.networkConfig(0x00AB, uwbsys::RANGING_MODE_TWR);
}

void loop()
{
    client.spin();
}