#!/usr/bin/env python3

import asyncio
import json
import signal
from dataclasses import dataclass
from datetime import datetime
from typing import Any

try:
    import websockets
except ImportError as error:
    raise SystemExit("Missing dependency: install with `python3 -m pip install websockets`") from error


# Configuration

HOST = "0.0.0.0"
PORT = 8080
ADDRESS = "/uwb"

PAN_ID = 0x1234
TIMEOUT_UUS = 6000

DEVICE_ADDRESSES = {
    "A": 0x1111,
    "B": 0x2222,
    "C": 0x3333,
}

LISTEN_COMMAND_CODE = 300
INITIATE_COMMAND_CODE = 200

LISTEN_LEAD_TIME_S = 0.10
PAIR_RESPONSE_TIMEOUT_S = 10.0
PAIR_DELAY_S = 0.50
SEQUENCE_DELAY_S = 2.00
REPEAT_SEQUENCE = True


@dataclass
class DeviceConnection:
    device_id: str
    address: int
    websocket: Any
    inbox: asyncio.Queue


connections: dict[str, DeviceConnection] = {}
connections_lock = asyncio.Lock()


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


def request_path(websocket: Any, path: str | None) -> str:
    if path is not None:
        return path

    legacy_path = getattr(websocket, "path", None)
    if legacy_path is not None:
        return legacy_path

    request = getattr(websocket, "request", None)
    if request is not None:
        return getattr(request, "path", "/")

    return "/"


def parse_device_id(path: str) -> str | None:
    path = path.split("?", 1)[0]
    address = normalize_address(ADDRESS)

    if address == "/":
        device_id = path.strip("/")
    elif path == address:
        device_id = ""
    elif path.startswith(address + "/"):
        device_id = path[len(address) + 1 :]
    else:
        return None

    if not device_id or "/" in device_id:
        return None

    return device_id


def build_command(code: int, source_id: str, target_id: str) -> dict[str, Any]:
    return {
        "code": code,
        "args": {
            "pan_id": PAN_ID,
            "destination_address": DEVICE_ADDRESSES[target_id],
            "source_address": DEVICE_ADDRESSES[source_id],
            "timeout_uus": TIMEOUT_UUS,
        },
    }


def format_payload(payload: Any) -> str:
    if isinstance(payload, (dict, list)):
        return json.dumps(payload, separators=(",", ":"))

    return str(payload)


def drain_queue(queue: asyncio.Queue) -> None:
    while True:
        try:
            queue.get_nowait()
        except asyncio.QueueEmpty:
            return


async def connected_device_ids() -> list[str]:
    async with connections_lock:
        return [device_id for device_id in DEVICE_ADDRESSES if device_id in connections]


async def get_connection(device_id: str) -> DeviceConnection | None:
    async with connections_lock:
        return connections.get(device_id)


async def send_json(connection: DeviceConnection, payload: dict[str, Any]) -> None:
    await connection.websocket.send(json.dumps(payload, separators=(",", ":")))


async def wait_for_pair_response(source: DeviceConnection, target: DeviceConnection) -> None:
    try:
        payload = await asyncio.wait_for(source.inbox.get(), timeout=PAIR_RESPONSE_TIMEOUT_S)
    except asyncio.TimeoutError:
        log(f"[TIMEOUT] no source response source={source.device_id} target={target.device_id}")
        return

    log(f"[PAIR] source response source={source.device_id} target={target.device_id} payload={format_payload(payload)}")


async def run_pair(source_id: str, target_id: str) -> None:
    source = await get_connection(source_id)
    target = await get_connection(target_id)
    if source is None or target is None:
        log(f"[SKIP] source={source_id} target={target_id} disconnected")
        return

    drain_queue(source.inbox)
    drain_queue(target.inbox)

    listen_command = build_command(LISTEN_COMMAND_CODE, source_id, target_id)
    initiate_command = build_command(INITIATE_COMMAND_CODE, source_id, target_id)

    log(
        "[PAIR] "
        f"source={source_id}(0x{source.address:04X}) "
        f"target={target_id}(0x{target.address:04X})"
    )
    log(f"[SEND] device={target_id} command=listen payload={format_payload(listen_command)}")
    await send_json(target, listen_command)

    await asyncio.sleep(LISTEN_LEAD_TIME_S)

    log(f"[SEND] device={source_id} command=initiate payload={format_payload(initiate_command)}")
    await send_json(source, initiate_command)

    await wait_for_pair_response(source, target)


async def run_sequence() -> None:
    device_ids = await connected_device_ids()
    if len(device_ids) < 2:
        return

    log(f"[SEQUENCE] devices={','.join(device_ids)}")

    for source_id in device_ids:
        for target_id in device_ids:
            if source_id == target_id:
                continue

            await run_pair(source_id, target_id)
            await asyncio.sleep(PAIR_DELAY_S)


async def sequence_loop(stop_event: asyncio.Event) -> None:
    while not stop_event.is_set():
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


async def register_connection(device_id: str, websocket: Any) -> DeviceConnection:
    connection = DeviceConnection(
        device_id=device_id,
        address=DEVICE_ADDRESSES[device_id],
        websocket=websocket,
        inbox=asyncio.Queue(),
    )

    async with connections_lock:
        previous = connections.get(device_id)
        connections[device_id] = connection

    if previous is not None:
        await previous.websocket.close(code=4000, reason="device reconnected")

    return connection


async def unregister_connection(connection: DeviceConnection) -> None:
    async with connections_lock:
        current = connections.get(connection.device_id)
        if current is connection:
            del connections[connection.device_id]


async def handle_message(connection: DeviceConnection, message: Any) -> None:
    try:
        payload = json.loads(message)
    except (TypeError, json.JSONDecodeError):
        payload = message

    log(f"[RECV] device={connection.device_id} payload={format_payload(payload)}")
    await connection.inbox.put(payload)


async def handle_connection(websocket: Any, path: str | None = None) -> None:
    path = request_path(websocket, path)
    device_id = parse_device_id(path)

    if device_id is None:
        log(f"[REJECT] path={path} reason=invalid_path")
        await websocket.close(code=1008, reason="invalid path")
        return

    if device_id not in DEVICE_ADDRESSES:
        log(f"[REJECT] device={device_id} reason=unknown_device")
        await websocket.close(code=1008, reason="unknown device")
        return

    connection = await register_connection(device_id, websocket)
    log(f"[CONNECT] device={device_id} address=0x{connection.address:04X} path={path}")

    try:
        async for message in websocket:
            await handle_message(connection, message)
    finally:
        await unregister_connection(connection)
        log(f"[DISCONNECT] device={device_id}")


async def main() -> None:
    stop_event = asyncio.Event()
    loop = asyncio.get_running_loop()

    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, stop_event.set)
        except NotImplementedError:
            pass

    address = normalize_address(ADDRESS)
    log(f"[START] ws://{HOST}:{PORT}{address}/<device_id>")
    log(f"[CONFIG] pan_id=0x{PAN_ID:04X} timeout_uus={TIMEOUT_UUS}")
    for device_id, address_value in DEVICE_ADDRESSES.items():
        log(f"[CONFIG] device={device_id} address=0x{address_value:04X}")

    async with websockets.serve(handle_connection, HOST, PORT):
        sequence_task = asyncio.create_task(sequence_loop(stop_event))
        await stop_event.wait()
        sequence_task.cancel()

        try:
            await sequence_task
        except asyncio.CancelledError:
            pass


if __name__ == "__main__":
    asyncio.run(main())
