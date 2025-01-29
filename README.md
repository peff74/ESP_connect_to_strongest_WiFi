# ESP8266 / ESP32 connect to the strongest WiFi  Script

This lightweight Arduino script helps your ESP automatically connect to the strongest available WiFi and seamlessly handle reconnections when needed.

## Features

- **Automatically scans and connects to the strongest WiFi signal** matching a predefined SSID.
- **Non-blocking WiFi management** to ensure smooth operation.
- **Dynamic Reconnect Handling**: If the connection drops, it intelligently attempts to reconnect.
- **Power-Saving Mechanism / wifi pollution stopping**: WiFi is temporarily disabled if no connection is possible.
- **Simple and clearly written** so that even beginners (like me) can understand how it works.

## How it Works

### WiFi Connection

1. The ESP **scans for available networks**.
2. It selects the strongest **RSSI (signal strength)** for the given SSID.
3. If no network is found at startup, it restarts after a timeout.
4. If the connection is lost, it **tries to reconnect** within a retry window.
5. If repeated attempts fail, WiFi is temporarily disabled before retrying later.

   
### Heartbeat Monitoring

- Periodically logs **WiFi state, signal strength, and IP address**.
- Helps with **debugging and monitoring**.

```
Start scanning for SSID Test
Scanning WiFi......
15 networks found:
0: BSSID: 00:11:6B:74:94:61  -77dBm,  46%  encrypted  Test
1: BSSID: 80:1F:02:AE:43:D8  -93dBm,  14%  encrypted  Test
2: BSSID: 80:1F:02:AE:41:48  -78dBm,  44%  encrypted  Test
3: BSSID: 80:1F:02:36:31:98  -80dBm,  40%  encrypted  Test
4: BSSID: 9C:A2:F4:63:65:60  -45dBm, 100%  encrypted  PG-WLAN
5: BSSID: 06:0B:BD:5A:DB:5D  -17dBm, 100%  encrypted  Test
6: BSSID: 00:11:6B:74:94:2B  -91dBm,  18%  encrypted  Test
7: BSSID: 80:1F:02:36:3A:98  -63dBm,  74%  encrypted  Test
8: BSSID: 00:11:6B:74:94:79  -82dBm,  36%  encrypted  der-hotspot
9: BSSID: 80:1F:02:36:31:C8  -71dBm,  58%  encrypted  Test
10: BSSID: 32:E5:EE:AD:C4:E4  -88dBm,  24%  encrypted  AndroidAP
11: BSSID: 80:1F:02:36:39:E0  -86dBm,  28%  encrypted  Test
12: BSSID: 80:1F:02:AE:41:30  -80dBm,  40%  encrypted  Test
13: BSSID: 80:1F:02:AE:43:F0  -81dBm,  38%  encrypted  Test
14: BSSID: 00:11:6B:74:94:55  -80dBm,  40%  encrypted  Test
Connecting to strongest WiFi signal at No. 5. 
Connecting WiFi....................................WiFi Connected
.192.168.206.105
WiFiconnected: 1 
WiFi-OFF: 0 
WiFi-OFF_count: 0 
WiFi-Connecting_count: 0 
Local IP:  192.168.206.105 
WiFi-RSSI:  -22 
Measurement_count: 59 
Unix-Time: 6 
___________________________
WiFiconnected: 1 
WiFi-OFF: 0 
WiFi-OFF_count: 0 
WiFi-Connecting_count: 0 
Local IP:  192.168.206.105 
WiFi-RSSI:  -24 
Measurement_count: 58 
Unix-Time: 1738189502 
___________________________
WiFiconnected: 1 
WiFi-OFF: 0 
WiFi-OFF_count: 0 
WiFi-Connecting_count: 0 
Local IP:  192.168.206.105 
WiFi-RSSI:  -23 
Measurement_count: 57 
Unix-Time: 1738189503 
___________________________
```

- **Hardware:** ESP8266 /ESP32
- **Libraries:**
  - `WiFi.h` for ESP32
  - `ArduinoOTA.h`
  - `time.h`

## How to Use

1. Change `WIFI_SSID` and `WIFI_PASSWORD`.
2. Flash the script to an ESP8266 / ESP32 device.

[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fpeff74%2FESP_connect_to_strongest_WiFi%2F&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://hits.seeyoufarm.com)

