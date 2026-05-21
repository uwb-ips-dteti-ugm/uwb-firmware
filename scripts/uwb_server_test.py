#!/usr/bin/env python3

import asyncio
import json
from contextlib import asynccontextmanager, suppress
from dataclasses import dataclass
from datetime import datetime
from typing import Any

try:
    import uvicorn
    from fastapi import FastAPI, WebSocket, WebSocketDisconnect
except ImportError as error:
    raise SystemExit("Missing dependency: install with `python3 -m pip install -r scripts/requirements.txt`") from error


# Configuration

HOST = "0.0.0.0"
PORT = 8080
ADDRESS = "/uwb"

PAN_ID = 0x1234
LISTEN_TIMEOUT_UUS = 120000
RANGING_TIMEOUT_UUS = 12000

DEVICE_ADDRESSES = {
    "E05A1B1FAF98": 0x1111,
    "A0A3B31FC848": 0x2222,
    "A0A3B31F3994": 0x3333,
}

LISTEN_COMMAND_CODE = 300
INITIATE_COMMAND_CODE = 200

LISTEN_LEAD_TIME_S = 0.08
PAIR_DELAY_S = 0.18
SEQUENCE_DELAY_S = 0.10
REPEAT_SEQUENCE = True


@dataclass
class DeviceConnection:
    device_id: str
    address: int
    websocket: WebSocket


connections: dict[str, DeviceConnection] = {}
connections_lock = asyncio.Lock()
stop_event: asyncio.Event | None = None
sequence_task: asyncio.Task | None = None


# Helpers


def timestamp() -> str:
    return datetime.now().strftime("%H:%M:%S.%f")[:-3]


def log(message: str) -> None:
    print(f"{timestamp()} {message}", flush=True)


def normalize_address(address: str) -> str:
    normalized = address.strip()
    if not normalized:
        return "/"

    if not normalized.startswith("/"):
        normalized = "/" + normalized

    if len(normalized) > 1 and normalized.endswith("/"):
        normalized = normalized[:-1]

    return normalized


def websocket_route(address: str) -> str:
    normalized = normalize_address(address)
    if normalized == "/":
        return "/{device_id}"

    return f"{normalized}/{{device_id}}"


def build_command(code: int, source_id: str, target_id: str, timeout_uus: int) -> dict[str, Any]:
    return {
        "code": code,
        "args": {
            "pan_id": PAN_ID,
            "destination_address": DEVICE_ADDRESSES[target_id],
            "source_address": DEVICE_ADDRESSES[source_id],
            "timeout_uus": timeout_uus,
        },
    }


def format_payload(payload: Any) -> str:
    if isinstance(payload, (dict, list)):
        return json.dumps(payload, separators=(",", ":"))

    return str(payload)


async def connected_device_ids() -> list[str]:
    async with connections_lock:
        return [device_id for device_id in DEVICE_ADDRESSES if device_id in connections]


async def get_connection(device_id: str) -> DeviceConnection | None:
    async with connections_lock:
        return connections.get(device_id)


async def send_json(connection: DeviceConnection, payload: dict[str, Any]) -> None:
    await connection.websocket.send_json(payload)


async def run_pair(source_id: str, target_id: str) -> None:
    source = await get_connection(source_id)
    target = await get_connection(target_id)
    if source is None or target is None:
        return

    listen_command = build_command(LISTEN_COMMAND_CODE, source_id, target_id, LISTEN_TIMEOUT_UUS)
    initiate_command = build_command(INITIATE_COMMAND_CODE, source_id, target_id, RANGING_TIMEOUT_UUS)

    await send_json(target, listen_command)

    await asyncio.sleep(LISTEN_LEAD_TIME_S)

    await send_json(source, initiate_command)


async def run_sequence() -> None:
    device_ids = await connected_device_ids()
    if len(device_ids) < 2:
        return

    for source_id in device_ids:
        for target_id in device_ids:
            if source_id == target_id:
                continue

            await run_pair(source_id, target_id)
            await asyncio.sleep(PAIR_DELAY_S)


async def sequence_loop() -> None:
    while stop_event is not None and not stop_event.is_set():
        device_ids = await connected_device_ids()
        if len(device_ids) < 2:
            await asyncio.sleep(0.25)
            continue

        await run_sequence()

        if not REPEAT_SEQUENCE:
            return

        try:
            await asyncio.wait_for(stop_event.wait(), timeout=SEQUENCE_DELAY_S)
        except asyncio.TimeoutError:
            pass


async def register_connection(device_id: str, websocket: WebSocket) -> DeviceConnection:
    connection = DeviceConnection(
        device_id=device_id,
        address=DEVICE_ADDRESSES[device_id],
        websocket=websocket,
    )

    async with connections_lock:
        previous = connections.get(device_id)
        connections[device_id] = connection

    if previous is not None:
        with suppress(Exception):
            await previous.websocket.close(code=4000, reason="device reconnected")

    return connection


async def unregister_connection(connection: DeviceConnection) -> None:
    async with connections_lock:
        current = connections.get(connection.device_id)
        if current is connection:
            del connections[connection.device_id]


async def handle_message(connection: DeviceConnection, message: str) -> None:
    try:
        payload = json.loads(message)
    except json.JSONDecodeError:
        payload = message

    log(f"[RECV] device={connection.device_id} payload={format_payload(payload)}")


@asynccontextmanager
async def lifespan(_: FastAPI):
    global stop_event, sequence_task

    stop_event = asyncio.Event()
    sequence_task = asyncio.create_task(sequence_loop())

    yield

    if stop_event is not None:
        stop_event.set()

    if sequence_task is not None:
        sequence_task.cancel()
        with suppress(asyncio.CancelledError):
            await sequence_task


# FastAPI app


app = FastAPI(title="UWB Server Test", lifespan=lifespan)
route = websocket_route(ADDRESS)


@app.websocket(route)
async def handle_connection(websocket: WebSocket, device_id: str) -> None:
    if device_id not in DEVICE_ADDRESSES:
        await websocket.close(code=1008, reason="unknown device")
        return

    await websocket.accept()
    connection = await register_connection(device_id, websocket)

    try:
        while True:
            message = await websocket.receive_text()
            await handle_message(connection, message)
    except WebSocketDisconnect:
        pass
    finally:
        await unregister_connection(connection)


if __name__ == "__main__":
    uvicorn.run(app, host=HOST, port=PORT)
