/**
 * This sketch connects an AirGradient DIY sensor to a WiFi network publishing to an MQTT broker
 */

/*
  PM1 and PM10 reporting for Plantower PMS5003 PM2.5 sensor enabled.
  Workaround for glitchy CO2 and PM sensors reporting included.
  For using this .ino you have to install improved AirGradient libraries, which supports PM1 and PM10 reporting: 
  https://github.com/d3vilh/airgradient-improved
  Also needed the async-mqtt-client library:
  https://github.com/marvinroger/async-mqtt-client
*/


#include <AirGradient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <Wire.h>
#include "SSD1306Wire.h"

AirGradient ag = AirGradient();

//MQTT adds
#include <Ticker.h>
#include <AsyncMqttClient.h>
#define WIFI_SSID "Add_your_SSID"
#define WIFI_PASSWORD "Add_your_pwd"

#define MQTT_HOST IPAddress(192, 168, 3, 100) //MQTT broker address
#define MQTT_PORT 1883

// Definition of topics (use what makes more sense for your setup)
#define MQTT_PUB_PM1 "Lab/PMS5003/PM1"
#define MQTT_PUB_PM25 "Lab/PMS5003/PM25"
#define MQTT_PUB_PM10 "Lab/PMS5003/PM10"
#define MQTT_PUB_TEMP "Lab/SHT3x/TEMP"
#define MQTT_PUB_HUM "Lab/SHT3x/HUM"
#define MQTT_PUB_CO2 "Lab/Senseair_S8/CO2"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
// MQTT


// Config ----------------------------------------------------------------------

// Optional.
const char* deviceId = "Lab";

// set to 'F' to switch display from Celcius to Fahrenheit
char temp_display = 'C';

// Hardware options for AirGradient DIY sensor.
const bool hasPM1 = true;
const bool hasPM2 = true;
const bool hasPM10 = true;
const bool hasCO2 = true;
const bool hasSHT = true;

// The frequency of measurement updates, milliseconds.
const int updateFrequency = 5000;

// For housekeeping.
long lastUpdate;
int counter = 0;
int stat_prev_pm1 = 0;
int stat_prev_pm2 = 0;
int stat_prev_pm10 = 0;
int stat_prev_co = 0;


// Config End ------------------------------------------------------------------

SSD1306Wire display(0x3c, SDA, SCL);

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
// If your broker requires authentication (username and password), set them below
  mqttClient.setCredentials("mqtt_device", "mqtt_ae019nk*");
  connectToWifi();


  // Init Display.
  display.init();
  display.flipScreenVertically();
  showTextRectangle("Init", String(ESP.getChipId(),HEX),true);

  // Enable enabled sensors.
  if (hasPM1) ag.PMS_Init();
  if (hasCO2) ag.CO2_Init();
  if (hasSHT) ag.TMP_RH_Init(0x44);
}

void loop() {
  long t = millis();
  updateScreen(t);
}

// DISPLAY
void showTextRectangle(String ln1, String ln2, boolean small) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  if (small) {
    display.setFont(ArialMT_Plain_16);
  } else {
    display.setFont(ArialMT_Plain_24);
  }
  display.drawString(32, 16, ln1);
  display.drawString(32, 36, ln2);
  display.display();
}

void updateScreen(long now) {
  if ((now - lastUpdate) > updateFrequency) {
    // Take a measurement at a fixed interval.
    switch (counter) {
      case 0:
        if (hasPM1) {
          int statf_pm1 = 0;
          int stat = ag.getPM1_Raw();
          if (stat > 0 && stat <= 10000) {
           statf_pm1 = stat;
           stat_prev_pm1 = statf_pm1; // saving not glitchy value
          } else {
            statf_pm1 = stat_prev_pm1; // using previous not glitchy value if curent value is glitchy
          }
          showTextRectangle("PM1",String(statf_pm1),false);
          //Publish to MQTT
          uint16_t packetIdPM1 = mqttClient.publish(MQTT_PUB_PM1, 1, true, String(statf_pm1).c_str());
        }
        break;
      case 1:
        if (hasPM2) {
          int statf_pm2 = 0;
          int stat = ag.getPM2_Raw();
          if (stat > 0 && stat <= 10000) {
           statf_pm2 = stat;
           stat_prev_pm2 = statf_pm2; // saving not glitchy value
          } else {
            statf_pm2 = stat_prev_pm2; // using previous not glitchy value if curent value is glitchy
          }
          showTextRectangle("PM2",String(statf_pm2),false);
          //Publish to MQTT
          uint16_t packetIdPM1 = mqttClient.publish(MQTT_PUB_PM25, 1, true, String(statf_pm2).c_str());
        }
        break;
      case 2:
        if (hasPM10) {
          int statf_pm10 = 0;
          int stat = ag.getPM10_Raw();
          if (stat > 0 && stat <= 10000) {
           statf_pm10 = stat;
           stat_prev_pm10 = statf_pm10; // saving not glitchy value
          } else {
            statf_pm10 = stat_prev_pm10; // using previous not glitchy value if curent value is glitchy
          }
          showTextRectangle("PM10",String(statf_pm10),false);
          //Publish to MQTT
          uint16_t packetIdPM1 = mqttClient.publish(MQTT_PUB_PM10, 1, true, String(statf_pm10).c_str());
        }
        break;
      case 3:
        if (hasCO2) {
          int statf_co = 0;
          int stat = ag.getCO2_Raw();
          if (stat >= 0 && stat <= 10000) {
           statf_co = stat;
           stat_prev_co = statf_co; // saving not glitchy value
          } else {
            statf_co = stat_prev_co; // using previous not glitchy value if curent value is glitchy
          }
          showTextRectangle("CO2", String(statf_co), false);
          //Publish to MQTT
          uint16_t packetIdPM1 = mqttClient.publish(MQTT_PUB_CO2, 1, true, String(statf_co).c_str());
        }
        break;
      case 4:
        if (hasSHT) {
          TMP_RH stat = ag.periodicFetchData();
          if (temp_display == 'F' || temp_display == 'f') {
            showTextRectangle("TMP", String((stat.t * 9 / 5) + 32, 1) + "F", false);
          } else {
            showTextRectangle("TMP", String(stat.t - 3, 1) + "C", false);
          }
          //Publish to MQTT
          uint16_t packetIdPM1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(stat.t).c_str());
        }
        break;
      case 5:
        if (hasSHT) {
          TMP_RH stat = ag.periodicFetchData();
          showTextRectangle("HUM", String(stat.rh) + "%", false);
          //Publish to MQTT
          uint16_t packetIdPM1 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(stat.rh).c_str());
        }
        break;
    }
    counter++;
    if (counter > 5) counter = 0;
    lastUpdate = millis();
  }
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

