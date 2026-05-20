from pathlib import Path

Import("env")


def read_dotenv(path):
    values = {}
    if not path.exists():
        return values

    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        key, value = line.split("=", 1)
        values[key.strip()] = value.strip().strip('"').strip("'")

    return values


def read_port(values):
    port = values.get("UWB_SERVER_PORT", "80").strip()
    if not port.isdigit():
        raise ValueError("UWB_SERVER_PORT must be numeric")

    parsed = int(port)
    if parsed < 1 or parsed > 65535:
        raise ValueError("UWB_SERVER_PORT must be between 1 and 65535")

    return str(parsed)


def read_scheme(values):
    scheme = values.get("UWB_SERVER_SCHEME", "ws").strip().lower()
    if scheme.endswith("://"):
        scheme = scheme[:-3]

    if scheme not in ("ws", "wss"):
        raise ValueError("UWB_SERVER_SCHEME must be ws or wss")

    return scheme


project_dir = Path(env["PROJECT_DIR"])
dotenv = read_dotenv(project_dir / ".env")

env.Append(
    CPPDEFINES=[
        ("UWB_FIRMWARE_WIFI_SSID", env.StringifyMacro(dotenv.get("WIFI_SSID", ""))),
        ("UWB_FIRMWARE_WIFI_PASSWORD", env.StringifyMacro(dotenv.get("WIFI_PASSWORD", ""))),
        ("UWB_FIRMWARE_SERVER_SCHEME", env.StringifyMacro(read_scheme(dotenv))),
        ("UWB_FIRMWARE_SERVER_HOST", env.StringifyMacro(dotenv.get("UWB_SERVER_HOST", ""))),
        ("UWB_FIRMWARE_SERVER_PORT", read_port(dotenv)),
        ("UWB_FIRMWARE_SERVER_PATH", env.StringifyMacro(dotenv.get("UWB_SERVER_PATH", "/"))),
    ]
)
