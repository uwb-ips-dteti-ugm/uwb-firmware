import requests

BASE_URL                        = "http://192.168.4.1"
API_URL_GET_UWB_CLIENT_INFO     = f"{BASE_URL}/api/uwb/client/info"
API_URL_GET_UWB_CLIENT_TWR      = f"{BASE_URL}/api/uwb/client/twr"
API_URL_POST_WIFI_CONNECT       = f"{BASE_URL}/api/wifi/connect"
API_URL_POST_WIFI_DISCONNECT    = f"{BASE_URL}/api/wifi/disconnect"
API_URL_POST_SERVER_CONFIG      = f"{BASE_URL}/api/wifi/config"
API_URL_POST_UWB_CONFIG         = f"{BASE_URL}/api/uwb/config"
API_URL_POST_DEVICE_RESTART     = f"{BASE_URL}/api/device/restart"

def restartDevice():
    res = requests.post(API_URL_POST_DEVICE_RESTART)
    print(res.json())

def main(args=None):
    restartDevice()

if __name__ == '__main__':
    main()