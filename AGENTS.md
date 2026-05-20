# AGENTS.md

## Project Idea

This repository contains ESP32 firmware for a UWB ranging device built around a DW3000 radio. The device keeps WiFi connected, connects to a UWB server over WebSocket, waits for ranging commands, performs stateless single-sided UWB ranging, and reports either the ranging result or an error back to the server.

The firmware follows a layered, port-and-adapter structure:

- Domain ports define what the firmware needs, independent of Arduino, DW3000, WiFi, or WebSocket libraries.
- Adapters implement those ports with concrete infrastructure libraries.
- Services coordinate domain behavior and logging.
- Controllers present services to external runtime input, currently FreeRTOS tasks and WebSocket events.
- Composition owns concrete objects, initializes hardware, wires dependencies, and starts the application.

The DW3000 adapter intentionally does not initialize the DW3000 chip. DW3000 setup belongs in the composition layer.

## Purpose

The main runtime flow is:

1. Arduino `setup()` creates `composition::App` and calls `run()`.
2. `App::setup()` initializes `Serial`, SPI, and the DW3000 chip.
3. `App::initInfrastructures()` validates required runtime configuration.
4. `App::initServices()` keeps service construction explicit.
5. `App::initControllers()` starts FreeRTOS task controllers.
6. The WiFi task keeps the ESP32 connected indefinitely.
7. The UWB stateless task connects to the WebSocket server as `/path/device_id`, handles commands, runs ranging, and sends results.
8. `App::run()` ends in an infinite delay, so Arduino `loop()` is intentionally unused.

## Structure

Important directories:

- `include/domain/models`: shared domain models such as `models::Error` and UWB frame constants.
- `include/domain/ports/driven`: infrastructure-facing ports implemented by adapters.
- `include/domain/ports/driving`: application-facing ports used by controllers.
- `include/adapters` and `src/adapters`: concrete implementations for Serial logging, ESP32 WiFi, DW3000 ranging, and arduinoWebSockets client.
- `include/services` and `src/services`: service-layer implementations. Services depend on ports, not concrete adapters.
- `include/controllers` and `src/controllers`: runtime presenters. Current controllers are FreeRTOS task controllers.
- `include/composition` and `src/composition`: composition root, configuration constants, hardware setup, object wiring.
- `scripts/load_env.py`: PlatformIO pre-script that loads local `.env` values into compile-time macros.
- `partitions`: custom flash partition tables for 8MB and 16MB ESP32 variants.
- `lib/DW3000`: local DW3000 library and examples used as the source of truth for radio setup.

## Runtime Configuration

Runtime configuration is compiled into the firmware from a local `.env` file. `.env` is ignored by git and must exist on the machine that builds/uploads the firmware.

Expected `.env` keys:

```env
WIFI_SSID=your_wifi_ssid
WIFI_PASSWORD=your_wifi_password
UWB_SERVER_SCHEME=ws
UWB_SERVER_HOST=192.168.1.10
UWB_SERVER_PORT=8080
UWB_SERVER_PATH=/uwb
```

`UWB_SERVER_SCHEME` accepts `ws`, `wss`, `ws://`, or `wss://`. The adapter normalizes it and calls:

- `WebSocketsClient::begin()` for `ws`
- `WebSocketsClient::beginSSL()` for `wss`

The server host must not include protocol, port, or path. The adapter appends the device ID to the configured path when connecting. For example:

```env
UWB_SERVER_SCHEME=ws
UWB_SERVER_HOST=192.168.1.50
UWB_SERVER_PORT=8080
UWB_SERVER_PATH=/uwb
```

connects to:

```text
ws://192.168.1.50:8080/uwb/<device_id>
```

The device ID is the ESP32 base MAC address formatted as uppercase hex without `:`.

## WebSocket Commands

The UWB task controller currently handles JSON text messages shaped like:

```json
{
  "code": 300,
  "args": {
    "pan_id": 1234,
    "destination_address": 2222,
    "source_address": 1111,
    "timeout_uus": 6000
  }
}
```

Command codes:

- `200`: initiate ranging, then send a ranging result or error.
- `300`: listen for a ranging poll, send the DW3000 response frame, then send an error only if the operation fails.

Ranging timeout values use DW3000 UWB microseconds (`uus`) because that is the unit used by the DW3000 API and examples.

## Build And Upload

Use PlatformIO.

Build:

```bash
pio run -e esp32dev-8mb
pio run -e esp32dev-16mb
```

Upload:

```bash
pio run -e esp32dev-8mb -t upload
pio run -e esp32dev-16mb -t upload
```

Monitor:

```bash
pio device monitor -e esp32dev-8mb
```

If the monitor resets the board unexpectedly, add or keep these settings in `platformio.ini`:

```ini
monitor_rts = 0
monitor_dtr = 0
```

The 8MB and 16MB environments use custom partition tables. Make sure the selected environment matches the actual flash size reported by:

```bash
python -m esptool --chip esp32 --port /dev/ttyUSB0 flash_id
```

In partition CSV files, the storage subtype should stay `spiffs`; PlatformIO controls LittleFS formatting with `board_build.filesystem = littlefs`.

## Development Guide

Follow the existing layer boundaries:

- Domain ports should stay free of Arduino, DW3000, WiFi, WebSocket, and FreeRTOS details.
- Adapters may depend on concrete libraries and should implement exactly one driven port.
- Services should depend on domain ports and logger ports, not concrete adapters.
- Controllers should handle runtime presentation details such as FreeRTOS loops and WebSocket callbacks.
- Composition should own concrete instances, perform hardware setup, wire dependencies, and start controllers.

Keep object ownership static where practical. The current app composition constructs concrete dependencies as members instead of allocating them dynamically.

Use the current C++ style:

- Keep implementations in matching `src/.../*.cpp` files.
- Use namespaces matching the directory structure.
- Use short section comments such as `// Helpers` and `// Adapter implementations`.
- Prefer small namespace-scope helper functions for file-local logic.
- Use direct early returns for invalid states.
- Keep log tags as `constexpr const char *`.
- Preserve existing snake_case member names where already used.
- Keep comments sparse and only add them where they explain non-obvious behavior.

Logging conventions:

- Adapters should log only what their current implementation pattern requires. The DW3000 ranging adapter currently uses error logs only.
- Services are the main place for operation logs.
- Controllers should avoid direct logs unless there is a specific reason; they present service behavior to runtime inputs.

When adding a new feature:

1. Add or adjust a domain model/port first if the behavior crosses a layer boundary.
2. Implement the concrete adapter only against the driven port.
3. Add service methods that coordinate ports and logging.
4. Add or update a controller if the behavior is triggered by a task, WebSocket command, HTTP request, MQTT event, or another external input.
5. Wire new concrete objects in `composition::App`.
6. Build the relevant PlatformIO environment.

## DW3000 Notes

Use `lib/DW3000/examples/range_rx/range_rx.ino` and `lib/DW3000/examples/range_tx/range_tx.ino` as references for DW3000 setup and single-sided ranging behavior.

Composition is responsible for:

- `Serial.begin`
- SPI speed and pin selection
- `spiBegin` / `spiSelect`
- `dwt_checkidlerc`
- `dwt_initialise`
- `dwt_configure`
- `dwt_configuretxrf`
- antenna delays
- LNA/PA mode

The stateless ranging adapter is responsible only for the ranging exchange itself:

- `initiate(...)`: send poll, wait for response, compute distance.
- `listen(...)`: wait for poll, send response.

Do not move DW3000 setup into the adapter.

## Safety Notes

Do not commit `.env` or secrets. The values are still embedded into the compiled firmware binary.

Do not use destructive git or filesystem commands to recover from build issues. Prefer inspecting generated files under `.pio/`, PlatformIO output, and partition tables first.

If changing partition tables, verify the actual ESP32 flash size before upload.
