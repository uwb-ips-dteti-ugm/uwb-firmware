import requests

BASE_URL                        = "http://192.168.4.1"
API_URL_GET_UWB_CLIENT_INFO     = f"{BASE_URL}/api/uwb/client/info"
API_URL_GET_UWB_CLIENT_TWR      = f"{BASE_URL}/api/uwb/client/twr"
API_URL_POST_WIFI_CONNECT       = f"{BASE_URL}/api/wifi/connect"
API_URL_POST_WIFI_DISCONNECT    = f"{BASE_URL}/api/wifi/disconnect"
API_URL_POST_SERVER_CONFIG      = f"{BASE_URL}/api/wifi/config"
API_URL_POST_UWB_CONFIG         = f"{BASE_URL}/api/uwb/config"
API_URL_POST_DEVICE_RESTART     = f"{BASE_URL}/api/device/restart"

def wifiConnect():
    payload = {
        'autoconnect': False,
        'ap_ssid': 'esp32-uwb-client0',
        'ap_pass': '12345678',
        'sta_ssid': 'DhonanAP',
        'sta_pass': 'epiepiepi'
    }
    res = requests.post(API_URL_POST_WIFI_CONNECT, json=payload)
    
    if res.status_code == 200:
        print(res.json())
    elif res.status_code == 400:
        print(res.text)

def serverConfig():
    payload = {
        'port': 80,
        'mdns': 'esp32-uwb.server'
    }
    res = requests.post(API_URL_POST_SERVER_CONFIG, json=payload)

    if res.status_code == 200:
        print(res.json())
    elif res.status_code == 400:
        print(res.text)

def clientInfo():
    res = requests.get(API_URL_GET_UWB_CLIENT_INFO)

    if res.status_code == 200:
        print(res.json())
    elif res.status_code == 400:
        print(res.text)

def clientTwr():
    res = requests.get(API_URL_GET_UWB_CLIENT_TWR)

    if res.status_code == 200:
        print(res.json())
    elif res.status_code == 400:
        print(res.text)

def uwbConfig():
    # payload = {
        # 'autostart': True,
        # 'is_server': True,
        # 'client_max': 16,
        # 'mode': 2,
        # 'network_addr': 0xDEFA,
        # 'device_addr': 0x0001
    # }
    payload = {
        'autostart': True,
        'is_server': False,
        'client_max': 16,
        'mode': 2,
        'network_addr': 0xDEFA,
        'device_addr': 0x0002
    }
    res = requests.post(API_URL_POST_UWB_CONFIG, json=payload)
    
    if res.status_code == 200:
        print(res.json())
    elif res.status_code == 400:
        print(res.text)

def restartDevice():
    res = requests.post(API_URL_POST_DEVICE_RESTART)
    
    if res.status_code == 200:
        print(res.json())
    elif res.status_code == 400:
        print(res.text)

def main(args=None):
    # wifiConnect()
    # clientInfo()
    # clientTwr()
    # uwbConfig()
    # restartDevice()
    return

if __name__ == '__main__':
    main()