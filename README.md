# ESP8266 / ESP32 connect to the strongest WiFi  Script
![ESP Boards](https://img.shields.io/badge/Supports-ESP32%20%7C%20ESP8266-green) 


This lightweight Arduino script helps your ESP automatically connect to the strongest available WiFi and seamlessly handle reconnections when needed.

## Features

- **Automatically scans and connects to the strongest WiFi signal** matching a predefined SSID.
- **Non-blocking WiFi management** to ensure smooth operation.
- **Dynamic Reconnect Handling**: If the connection drops, it intelligently attempts to reconnect.
- **Power-Saving Mechanism / wifi pollution stopping**: WiFi is temporarily disabled if no connection is possible.
- **OTA-OverTheAir**: as example to use WiFi
- **Simple and clearly written** so that even beginners (like me) can understand how it works.

## How it Works

### WiFi Connection

1. The ESP **scans for available networks**.
<details>
<summary>📝 code</summary>
   
```
void scanWiFiNetwork() {
  Serial.println(F(""));
  Serial.printf("Start scanning for SSID %s\n", WIFI_SSID);
  WiFi.scanNetworks(true);  // WiFi.scanNetworks will return the number of networks found
}
```

</details>

   
2. It will then select the strongest **RSSI (signal strength)** for the given SSID and connetet to it
<details>
<summary>📝 code</summary>
   
```
void connectToStrongestWiFi() {
  int i_strongest = -1;
  int32_t rssi_strongest = -100;
  int16_t WiFiScanResult = WiFi.scanComplete();
  Serial.println(F(""));
  if (WiFiScanResult < 0) {
    Serial.println(F("No networks found!"));
  } else {
    Serial.printf("%d networks found:\n", WiFiScanResult);
    for (int i = 0; i < WiFiScanResult; ++i) {
      Serial.printf("%d: BSSID: %s  %2ddBm, %3d%%  %9s  %s\n",
                    i,
                    WiFi.BSSIDstr(i).c_str(),
                    WiFi.RSSI(i),
                    constrain(2 * (WiFi.RSSI(i) + 100), 0, 100),
                    (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "encrypted",
                    WiFi.SSID(i).c_str());
      if (strcmp(WIFI_SSID, WiFi.SSID(i).c_str()) == 0 && (WiFi.RSSI(i)) > rssi_strongest) {
        rssi_strongest = WiFi.RSSI(i);
        i_strongest = i;
      }
    }
  }

  if (i_strongest > -1) {
    Serial.printf("Connecting to strongest WiFi signal at No. %d. \n", i_strongest);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, WiFi.BSSID(i_strongest));
  } else {
    Serial.printf("No network with SSID %s found!\n", WIFI_SSID);
  }
}
```

</details>

3. If the connection is lost, it **switches off the WiFi temporarily** and **tries to reconnect** within a retry window.
<details>
<summary>📝 code</summary>
   
```
void handelWiFi() {
  if (WiFiconnected && WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    WiFiconnected = false;
    Serial.println(F("............................WiFi Disconnected"));
  }


  if (!WiFiconnected && !WiFiOff && WiFiconnecting_count == 0) {
    Serial.println(F("Switching WiFi Off, no WiFi available"));
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1);
    WiFiOff = true;
    WiFiOff_count = WIFIOFF_COUNT_VALUE;
  }

  if (WiFiOff) {
    --WiFiOff_count;
  }

  if (WiFiOff && WiFiOff_count == 0) {
    Serial.println(F("Try to reconnect"));
    WiFi.mode(WIFI_STA);
    scanWiFiNetwork();
    WiFiOff = false;
    WiFiconnecting_count = WIFICONNECTING_COUNT_VALUE;
  }

  if (WiFiscandone) {  // WiFiscandone kommt über WiFiEvent
    connectToStrongestWiFi();
    WiFi.scanDelete();
    WiFiscandone = false;
  }

  if (!WiFiconnected && WiFiconnecting_count > 0) {
    --WiFiconnecting_count;
  }
}
```

</details>

4.  It uses **asynchronous callbacks (WiFi.onEvent/WiFiEventHandler** that work **non-blocking** to improve efficiency.
<details>
<summary>📝 ESP32 code</summary>

```
WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
// WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED); /does not work propper with ESP32
WiFi.onEvent(onWifiScandone, WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
```

</details>

<details>
<summary>📝 ESP8266 code</summary>

```
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
.
.
.
wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
.
.
.
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  }

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
 }

```

</details>
 



   
### Heartbeat Monitoring

- Periodically logs **WiFi state, signal strength, and IP address**.
- Helps with **debugging and monitoring**.
- Easy to **integrate custom functions**.
<details>
<summary>📝 example</summary>

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
</details>


## How to Use

1. Just change the "defines" from one of the scripts to suit your needs.
<details>
<summary>📝 code</summary>
   
```
//=======Defines

#define WIFI_SSID "Test"
#define WIFI_PASSWORD "Password1"
#define WIFIOFF_COUNT_VALUE 90         // Counter for how long WiFi remains off before attempting a reconnection
#define WIFICONNECTING_COUNT_VALUE 10  // Counter for how long WiFi should attempt to connect before being disabled again
#define OTAHOSTNAME "ESP32-WiFi"
#define WiFiHOSTNAME "Test-ESP32"

#define MEASUREMENT_COUNT_VALUE 60    // Multiplier for Heartbeat
#define HEARTBEATINTERVAL_VALUE 1000  // Milliseconds for a Heartbeat
```

</details>

2. Flash the script to an ESP8266 / ESP32 device.

[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fpeff74%2FESP_connect_to_strongest_WiFi%2F&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://hits.seeyoufarm.com)

