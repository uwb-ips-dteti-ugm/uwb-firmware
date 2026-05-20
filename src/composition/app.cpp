#include "composition/app.h"

#include <Arduino.h>
#include <SPI.h>

#include "dw3000.h"

extern SPISettings _fastSPI;
extern dwt_txconfig_t txconfig_options;

namespace composition
{
    constexpr const char *setupTag = "composition::App::setup";
    constexpr const char *infrastructureTag = "composition::App::initInfrastructures";
    constexpr const char *serviceTag = "composition::App::initServices";
    constexpr const char *controllerTag = "composition::App::initControllers";

    dwt_config_t uwb_config = {
        5,
        DWT_PLEN_128,
        DWT_PAC8,
        9,
        9,
        1,
        DWT_BR_6M8,
        DWT_PHRMODE_STD,
        DWT_PHRRATE_STD,
        (129 + 8 - 8),
        DWT_STS_MODE_OFF,
        DWT_STS_LEN_64,
        DWT_PDOA_M0};

    // Helpers

    void halt()
    {
        while (true)
            vTaskDelay(0xFFFFFFFF);
    }

    void haltOnFailure(ports::driven::logger::Leveled *logger, const char *tag, const char *message)
    {
        logger->error(tag, "%s", message);
        halt();
    }

    bool isEmpty(const char *value)
    {
        return value == nullptr || value[0] == '\0';
    }

    // App implementations

    App::App()
        : logger(&Serial, adapters::logger::leveled::LOG_LEVEL_INFO),
          websocket_client(),
          wifi_connection(),
          ranging(
              &logger,
              config::uwbTxAntennaDelay,
              config::uwbPollTxToRespRxDelayUus,
              config::uwbPollRxToRespTxDelayUus),
          uwb_server(
              &logger,
              &websocket_client,
              config::uwbServerScheme,
              config::uwbServerHost,
              config::uwbServerPort,
              config::uwbServerPath,
              config::uwbServerConnectTimeoutMs),
          wifi_service(&wifi_connection, &logger),
          uwb_service(&logger, &uwb_server, &ranging, &wifi_connection),
          wifi_controller(
              &wifi_service,
              config::wifiSsid,
              config::wifiPassword,
              config::wifiTaskCheckIntervalMs,
              "wifi_connection",
              config::wifiTaskStackDepth,
              config::wifiTaskPriority),
          uwb_controller(
              &uwb_service,
              &websocket_client,
              config::uwbTaskCheckIntervalMs,
              "uwb_stateless",
              config::uwbTaskStackDepth,
              config::uwbTaskPriority)
    {
    }

    void App::setup()
    {
        Serial.begin(config::serialBaud);
        delay(100);

        _fastSPI = SPISettings(config::uwbSpiFastClockHz, MSBFIRST, SPI_MODE0);
        spiBegin(config::uwbPinIrq, config::uwbPinReset);
        spiSelect(config::uwbPinSelect);
        delay(2);

        if (!dwt_checkidlerc())
            haltOnFailure(&logger, setupTag, "DW3000 IDLE_RC check failed");

        if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
            haltOnFailure(&logger, setupTag, "DW3000 init failed");

        dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

        if (dwt_configure(&uwb_config))
            haltOnFailure(&logger, setupTag, "DW3000 configure failed");

        dwt_configuretxrf(&txconfig_options);
        dwt_setrxantennadelay(config::uwbRxAntennaDelay);
        dwt_settxantennadelay(config::uwbTxAntennaDelay);
        dwt_setrxaftertxdelay(config::uwbPollTxToRespRxDelayUus);
        dwt_setrxtimeout(config::uwbDefaultResponseRxTimeoutUus);
        dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

        logger.info(setupTag, "DW3000 ready");
    }

    void App::initInfrastructures()
    {
        if (isEmpty(config::wifiSsid))
            haltOnFailure(&logger, infrastructureTag, "WiFi SSID is empty");

        if (isEmpty(config::uwbServerHost) || config::uwbServerPort == 0)
            haltOnFailure(&logger, infrastructureTag, "UWB server endpoint is invalid");

        logger.info(infrastructureTag, "Infrastructures ready");
    }

    void App::initServices()
    {
        logger.info(serviceTag, "Services ready");
    }

    void App::initControllers()
    {
        if (!wifi_controller.start())
            haltOnFailure(&logger, controllerTag, "Failed to start WiFi connection task");

        if (!uwb_controller.start())
            haltOnFailure(&logger, controllerTag, "Failed to start UWB stateless task");

        logger.info(controllerTag, "Controllers ready");
    }

    void App::run()
    {
        setup();
        initInfrastructures();
        initServices();
        initControllers();

        halt();
    }
}
