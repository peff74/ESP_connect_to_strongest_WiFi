//IDE 2.3.3 - - esp32 3.1.1
#include <WiFi.h>

//=======Defines

#define WIFI_SSID "Test"
#define WIFI_PASSWORD "Password1"

#define WiFiHOSTNAME "Test-ESP32"

#define WIFIOFF_COUNT_VALUE 90         // Counter for how long WiFi remains off before attempting a reconnection
#define WIFICONNECTING_COUNT_VALUE 10  // Counter for how long WiFi should attempt to connect before being disabled again

#define HEARTBEAT_COUNT_VALUE 60    // Multiplier for Heartbeat
#define HEARTBEATINTERVAL_VALUE 1000  // Milliseconds for a Heartbeat


//=======SETTINGS


// Timer for Heartbeat 1 sec.
unsigned long HeartbeatMillis = 0;                       // Stores the last time the Heartbeat was running
const long Heartbeatinterval = HEARTBEATINTERVAL_VALUE;  // Interval at which to send (milliseconds)
int Heartbeat_count = HEARTBEAT_COUNT_VALUE;


// WiFi reconnecting
bool WiFiscandone = false;                              // Flag for WiFi scan
bool WiFiconnected = false;                             // Flag for WiFi connected
bool WiFiOff = false;                                   // Flag for WiFi OFF
int WiFiOff_count = WIFIOFF_COUNT_VALUE;                // Counter for how long WiFi should remain off
int WiFiconnecting_count = WIFICONNECTING_COUNT_VALUE;  // Counter for WiFi connection


// Print buffers
char printbuffer0[400];  // NFO
char printbuffer[70];    // Loop



/*
  WiFi
*/

void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
  WiFiconnected = true;
  WiFiconnecting_count = 0;
  WiFiOff_count = 0;
  Serial.println(F(".............................WiFi Connected"));
}

// void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
//   WiFiconnected = false;
//   Serial.println(F("............................WiFi Disconnected"));
// }

void onWifiScandone(WiFiEvent_t event, WiFiEventInfo_t info) {
  WiFiscandone = true;
}

void WiFisetup() {
  WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  // WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED); /does not work propper
  WiFi.onEvent(onWifiScandone, WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
  WiFi.setAutoReconnect(false);
  WiFi.setHostname(WiFiHOSTNAME);
}

void WiFistartup() {
  WiFi.mode(WIFI_STA);
  scanWiFiNetwork();
  byte i = 0;
  Serial.print(F("Scanning WiFi."));
  while (WiFi.scanComplete() < 0) {
    delay(500);
    ++i;
    Serial.print(F("."));
    if (i > 20) {
      ESP.restart();
    }
  }
  connectToStrongestWiFi();
  i = 0;
  Serial.print(F("Connecting WiFi."));
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(500);
    ++i;
    Serial.print(F("."));
    if (i > 20) {
      ESP.restart();
    }
  }
  WiFi.scanDelete();
  Serial.print(F(""));
  Serial.println(WiFi.localIP());
  WiFiscandone = false;
}


void scanWiFiNetwork() {
  Serial.println(F(""));
  Serial.printf("Start scanning for SSID %s\n", WIFI_SSID);
  WiFi.scanNetworks(true);  // WiFi.scanNetworks will return the number of networks found
}


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

/*
  End of WiFi
*/


//============SETUP

void setup() {
  Serial.begin(115200);
  WiFisetup();
  WiFistartup();
}

//============LOOP

void loop() {

  if (WiFiconnected) {
       // code that need WiFi
  }

  //===========Heartbeat
  unsigned long currentMillis = millis();

  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;
    --HEARTBEAT_COUNT;

    handelWiFi();

    char ipBuffer[16];
    WiFi.localIP().toString().toCharArray(ipBuffer, sizeof(ipBuffer));
    snprintf(printbuffer0, sizeof(printbuffer0),
             "WiFiconnected: %i \n"
             "WiFi-OFF: %i \n"
             "WiFi-OFF_count: %i \n"
             "WiFi-Connecting_count: %i \n"
             "Local IP:  %s \n"
             "WiFi-RSSI:  %d \n"
             "HEARTBEAT_COUNT: %i \n"
             "___________________________",
             WiFiconnected, WiFiOff, WiFiOff_count,
             WiFiconnecting_count, ipBuffer, WiFi.RSSI(),
             HEARTBEAT_COUNT);
    Serial.println(printbuffer0);


    if (Heartbeat_count == 0) {
      Heartbeat_count = HEARTBEAT_COUNT_VALUE;
    }
  }
}
