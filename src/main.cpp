#include <Arduino.h>
#include "composition/app.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);
    // static composition::App app;
    // app.run();
}

void loop()
{
    Serial.println("Hello, world!");
    delay(1000);
}
