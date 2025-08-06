#ifndef __APPS_APP_H__
#define __APPS_APP_H__

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>

#include "API/kernel.h"
#include "UWBClient/kernel.h"
#include "UWBServer/kernel.h"

namespace app
{
    void start();
}

#endif