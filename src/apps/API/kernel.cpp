#include "apps/API/kernel.h"

void api::run(api::KernelInst *inst)
{
    xTaskCreatePinnedToCore(
        api::kernelTask,
        API_KERNEL_NAME,
        API_KERNEL_STACK_SIZE,
        (void *)inst,
        API_KERNEL_PRIORITY,
        &api::kerReg.taskHandle,
        API_KERNEL_CORE);
}

void api::kernelTask(void *pvParameters)
{
    api::KernelInst *inst = (api::KernelInst *)pvParameters;
    api::kernelSetup(inst);

    while (1)
    {
        api::kernelLoop(inst);
        vTaskDelay(API_KERNEL_DELAY_TICKS);
    }
}

void api::kernelSetup(api::KernelInst *inst)
{
    if (api::utl::rWiFiConfigExists(inst->fs))
        api::utl::rLoadWiFiConfig(inst->fs);

    if (api::utl::rServerConfigExists(inst->fs))
        api::utl::rLoadServerConfig(inst->fs);

    if (api::utl::rUWBConfigExists(inst->fs))
        api::utl::rLoadUWBConfig(inst->fs);

    if (api::utl::rIsUWBAutoStart())
    {
        if (api::utl::rIsUWBServer())
        {
            uws::KernelInst *uwsInst = new uws::KernelInst;
            uwsInst->clientMax = api::uwbCfg.clientMax;
            uwsInst->networkAddress = api::uwbCfg.networkAddr;
            uwsInst->deviceAddress = api::uwbCfg.deviceAddr;
            uws::run(uwsInst);
            inst->uwbRunning = true;
        }
    }
}

void api::kernelLoop(api::KernelInst *inst)
{
    api::kernelEventRoutine(inst);
    api::kernelMainRoutine(inst);
}

void api::kernelEventRoutine(api::KernelInst *inst)
{
    static api::trp::models::Generic buf;

    if (xQueueReceive(api::trp::queue, &buf, 0) == pdTRUE)
    {
        switch (buf.type)
        {
        case api::trp::types::WIFI_CONNECT:
        {
            api::utl::rSaveWiFiConfig(inst->fs);
            api::utl::rSetKernelState(api::KERNEL_STATE_WIFI_ESTABLISH);
            break;
        }

        case api::trp::types::WIFI_DISCONNECT:
        {
            api::utl::rDisableWiFiAutoConnect();
            api::utl::rSetKernelState(api::KERNEL_STATE_WIFI_INIT);
            break;
        }

        case api::trp::types::SERVER_CONFIG:
        {
            api::utl::rSaveServerConfig(inst->fs);
            break;
        }

        case api::trp::types::UWB_CONFIG:
        {
            api::utl::rSaveUWBConfig(inst->fs);
            if (!inst->uwbRunning)
            {
                uws::KernelInst *uwsInst = new uws::KernelInst;
                uwsInst->clientMax = api::uwbCfg.clientMax;
                uwsInst->networkAddress = api::uwbCfg.networkAddr;
                uwsInst->deviceAddress = api::uwbCfg.deviceAddr;
                uws::run(uwsInst);
                inst->uwbRunning = true;
            }
            break;
        }

        case api::trp::types::UWB_CLIENT_INFO:
        {
            break;
        }

        case api::trp::types::UWB_CLIENT_TWR:
        {
            break;
        }
        }
    }
}

void api::kernelMainRoutine(api::KernelInst *inst)
{
    static uint32_t wifiConnToTs;

    switch (api::utl::rGetKernelState())
    {
    case api::KERNEL_STATE_WIFI_INIT:
    {
        inst->wifi->disconnect();

        if (api::utl::rIsWiFiAutoConnect())
        {
            char ssid[32], pass[32];
            api::utl::rGetWiFiSTACredentials(ssid, pass);

            inst->wifi->mode(WIFI_STA);
            inst->wifi->begin(ssid, pass);
            wifiConnToTs = millis();

            api::utl::rSetKernelState(api::KERNEL_STATE_WIFI_CONNECTING);
        }
        else
        {
            char ssid[32], pass[32];
            api::utl::rGetWiFiAPCredentials(ssid, pass);

            inst->wifi->mode(WIFI_AP);
            inst->wifi->softAP(ssid, pass);

            api::utl::rSetKernelState(api::KERNEL_STATE_SERVER_INIT);
        }
        break;
    }

    case api::KERNEL_STATE_WIFI_ESTABLISH:
    {
        inst->wifi->disconnect();

        char ssid[32], pass[32];
        api::utl::rGetWiFiSTACredentials(ssid, pass);

        inst->wifi->mode(WIFI_STA);
        inst->wifi->begin(ssid, pass);
        wifiConnToTs = millis();

        api::utl::rSetKernelState(api::KERNEL_STATE_WIFI_CONNECTING);
        break;
    }

    case api::KERNEL_STATE_WIFI_CONNECTING:
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            char mdns[32];
            api::utl::rGetMDNS(mdns);
            MDNS.begin(mdns);

            api::utl::rSetKernelState(api::KERNEL_STATE_SERVER_INIT);
        }
        else if (millis() - wifiConnToTs > 10000)
        {
            api::utl::rDisableWiFiAutoConnect();
            api::utl::rSetKernelState(api::KERNEL_STATE_WIFI_INIT);
        }
        break;
    }

    case api::KERNEL_STATE_SERVER_INIT:
    {
        inst->srv = new WebServer(api::utl::rGetServerPort());
        api::route(inst->srv);
        inst->srv->begin();

        api::utl::rSetKernelState(api::KERNEL_STATE_SERVER_LISTENING);
        break;
    }

    case api::KERNEL_STATE_SERVER_LISTENING:
    {
        inst->srv->handleClient();

        if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED)
        {
            inst->srv->stop();
            delete inst->srv;
            inst->srv = nullptr;

            api::utl::rSetKernelState(api::KERNEL_STATE_WIFI_INIT);
        }
        break;
    }
    }
}