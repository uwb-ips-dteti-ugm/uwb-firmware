#include "apps/app.h"

void app::start()
{
    Serial.begin(115200);
    LittleFS.begin(true);

    api::KernelInst *apiInst = new api::KernelInst;
    apiInst->fs = &LittleFS;
    apiInst->wifi = &WiFi;
    api::run(apiInst);
}