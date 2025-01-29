# ESP8266 connect to the strongest WiFi  Script

This lightweight Arduino script helps your ESP8266 automatically connect to the strongest available WiFi and seamlessly handle reconnections when needed.

## Features

- **Automatically scans and connects to the strongest WiFi signal** matching a predefined SSID.
- **Non-blocking WiFi management** to ensure smooth operation.
- **Dynamic Reconnect Handling**: If the connection drops, it intelligently attempts to reconnect.
- **Power-Saving Mechanism / wifi pollution stopping**: WiFi is temporarily disabled if no connection is possible.
- **Simple and clearly written** so that even beginners (like me) can understand how it works.

## How it Works

### WiFi Connection

1. The ESP8266 **scans for available networks**.
2. It selects the strongest **RSSI (signal strength)** for the given SSID.
3. If no network is found, it restarts after a timeout.
4. If the connection is lost, it **tries to reconnect** within a retry window.
5. If repeated attempts fail, WiFi is temporarily disabled before retrying later.

   
### Heartbeat Monitoring

- Periodically logs **WiFi state, signal strength, and IP address**.
- Helps with **debugging and monitoring**.

## Code Snippets

### Scanning for the Strongest WiFi Network
```cpp
void scanWiFiNetwork() {
  Serial.printf("Scanning for SSID %s\n", WIFI_SSID);
  WiFi.scanNetworks(true);
}
```

### Connecting to the Strongest WiFi Signal
```cpp
void connectToStrongestWiFi() {
  int i_strongest = -1;
  int32_t rssi_strongest = -100;
  int16_t WiFiScanResult = WiFi.scanComplete();

  if (WiFiScanResult > 0) {
    for (int i = 0; i < WiFiScanResult; ++i) {
      if (strcmp(WIFI_SSID, WiFi.SSID(i).c_str()) == 0 && WiFi.RSSI(i) > rssi_strongest) {
        rssi_strongest = WiFi.RSSI(i);
        i_strongest = i;
      }
    }
  }

  if (i_strongest > -1) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, WiFi.BSSID(i_strongest));
  } else {
    Serial.printf("No network with SSID %s found!\n", WIFI_SSID);
  }
}
```

### Handling WiFi Reconnection
```cpp
void handelWiFi() {
  if (!WiFiconnected && !WiFiOff && WiFiconnecting_count == 0) {
    WiFi.disconnect(true);
    WiFi.forceSleepBegin();
    WiFiOff = true;
    WiFiOff_count = WIFIOFF_COUNT_VALUE;
  }
}
```

## Requirements

- **Hardware:** ESP8266
- **Libraries:**
  - `ArduinoOTA.h`
  - `time.h`

## How to Use

1. Change `WIFI_SSID` and `WIFI_PASSWORD`.
2. Flash the script to an ESP8266 device.



