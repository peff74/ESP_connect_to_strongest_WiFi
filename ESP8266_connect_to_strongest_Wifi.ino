
//IDE 2.3.3 - - esp8266 3.1.2
#include <ArduinoOTA.h>
#include <time.h>

//=======Defines

#define WIFI_SSID "Test"          
#define WIFI_PASSWORD "Password1"  
#define WIFIOFF_COUNT_VALUE 90         //Counter wie lang WiFi ausgeschaltet bleibt bis ein neuer Verbindungsversuch gestartet wird
#define WIFICONNECTING_COUNT_VALUE 10  //Counter wie lange WiFi versucht werden soll zu erreichen, bis erneut deaktiviert wird
#define OTAHOSTNAME "ESP8266-WiFi"
#define WiFiHOSTNAME "Test-ESP8266"

#define MEASUREMENT_COUNT_VALUE 60    // multiplikator von Heartbeat
#define HEARTBEATINTERVAL_VALUE 1000  // MilliSekunden für einen Heartbeat



//=======SETTINGS


//Timer für Heartbeat 1 sec.
unsigned long HeartbeatMillis = 0;                       // will store last time Heartbeat was running
const long Heartbeatinterval = HEARTBEATINTERVAL_VALUE;  // interval at which to send (milliseconds)
int Measurement_count = MEASUREMENT_COUNT_VALUE;



//WiFi reconnecting
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
bool WiFiscandone = false;                              // Schalter für Wifi scan
int WiFiconnecting_count = WIFICONNECTING_COUNT_VALUE;  // Counter für WiFi connection
bool WiFiconnected = false;                             // Schalter für Wifi verbunden
bool WiFiOff = false;                                   // Schalter für Wifi OFF
int WiFiOff_count = WIFIOFF_COUNT_VALUE;                // Counter wie lange WiFi Off bleiben soll




//Printbuffers
char printbuffer0[400];  // NFO
char printbuffer[70];    // Loop



/*
  OTA Upload via ArduinoIDE
*/

void ideOTASetup() {
  ArduinoOTA.setHostname(OTAHOSTNAME);
  ArduinoOTA.onStart([]() {
    Serial.println("[otaide] OTA started");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);

    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

/*
  End off OTA Upload via ArduinoIDE
*/


/*
  WiFi
*/

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  WiFiconnected = true;
  WiFiconnecting_count = 0;
  WiFiOff_count = 0;
  Serial.println(F(".............................WiFi Connected"));
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  WiFiconnected = false;
  Serial.println(F("............................WiFi Disconnected"));
}



void WiFisetup() {
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  WiFi.setAutoReconnect(false);
  WiFi.hostname(WiFiHOSTNAME);
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
    if (i > 30) {
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
                    (WiFi.encryptionType(i) == AUTH_OPEN) ? "open" : "encrypted",
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
  if (WiFi.scanComplete() > -1) {
    WiFiscandone = true;
  }


  if (!WiFiconnected && !WiFiOff && WiFiconnecting_count == 0) {
    Serial.println(F("Switching WiFi Off, no WiFi available"));
    WiFi.disconnect(true);   // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html?highlight=WiFi.disconnect()#disconnect
    WiFi.forceSleepBegin();  // schaltet WiFi physikalisch ab
    WiFiOff = true;
    WiFiOff_count = WIFIOFF_COUNT_VALUE;
  }

  if (WiFiOff == true) {
    --WiFiOff_count;
  }

  if (WiFiOff && WiFiOff_count == 0) {
    Serial.println(F("Try to reconnect"));
    WiFi.mode(WIFI_STA);
    scanWiFiNetwork();
    WiFiOff = false;
    WiFiconnecting_count = WIFICONNECTING_COUNT_VALUE;
  }

  if (WiFiscandone) {
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
  configTime(0, 0, "pool.ntp.org");
}

//============LOOP

void loop() {

  if (WiFiconnected) {
    ArduinoOTA.handle();
  }

  //===========Heartbeat
  unsigned long currentMillis = millis();

  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;
    --Measurement_count;
    handelWiFi();

    time_t now;
    time(&now);
    char ipBuffer[16];
    WiFi.localIP().toString().toCharArray(ipBuffer, sizeof(ipBuffer));
    snprintf(printbuffer0, sizeof(printbuffer0),
             "WiFiconnected: %i \n"
             "WiFi-OFF: %i \n"
             "WiFi-OFF_count: %i \n"
             "WiFi-Connecting_count: %i \n"
             "Local IP:  %s \n"
             "WiFi-RSSI:  %d \n"
             "Measurement_count: %i \n"
             "Unix-Time: %lld \n"
             "___________________________",
             WiFiconnected, WiFiOff, WiFiOff_count,
             WiFiconnecting_count, ipBuffer, WiFi.RSSI(),
             Measurement_count, now);
    Serial.println(printbuffer0);


    if (Measurement_count == 0) {
      Measurement_count = MEASUREMENT_COUNT_VALUE;
    }
  }
}
