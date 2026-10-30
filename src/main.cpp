// Topfile for the ESP32.
// connects to sensors which are polled frequently
// sends these to the MQTT broker on the gateway

// INCLUDES
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <PubSubClient.h>
#include "BMEWrapper.h"
// check this file for #define constants.
#include "config.h"

#include "pir.h"
#include "microphone.h"
#include "secrets.h"
#include "potmeter.h"
#include "wirelessDetection.h"



// OBJECTS
// TwoWire Wire = TwoWire(0);  // I2C controller 0
BMEWrapper bmeWrapper(Wire);

Microphone mic;         // create a microphone object

WiFiClient espClient;
PubSubClient mqttClient(espClient);

PIR pir1(PIR1_PIN, PIR_BUFFER_SIZE);
PIR pir2(PIR2_PIN, PIR_BUFFER_SIZE);
PIR pir3(PIR3_PIN, PIR_BUFFER_SIZE);

Potmeter potmeter(POTMETER_PIN, LED1_PIN, LED2_PIN, POTMETER_BUFFER_SIZE);


// TIMERS
unsigned long lastPublishTime = 0;
unsigned long lastSampleTime = 0;

// vars
String clientId;

// functions
bool setupWifi();
void readMacAddress();
String preparePayload(float temperature, float humidity, float pressure, float iaq, float voc, float co2Equivalent, uint8_t accuracy, uint8_t pir1Activity, uint8_t pir2Activity, uint8_t pir3Activity, float soundLevel, uint8_t busynessLabel, uint32_t wifiPackets, uint32_t bleAdverts);
bool mqttEnsureConnected();
bool publishMQTT(const char* building, const char* room, const char* payload);

void setup() {
  Serial.begin(115200);

  delay(500);
  // while (!Serial) {
  //   ; // wait for serial port to connect (mostly for safety)
  // }

  Serial.println();
  Serial.println("Starting ESP32...");

  Serial.print(WiFi.macAddress());

  // sensors
  // BME680
  if (!bmeWrapper.begin()) {
    Serial.println("BME680 initialization failed");
  } else {
    Serial.println("I2C/BME680 initialized");
  }

  // mic
  if (!mic.begin(I2S_SCK, I2S_WS, I2S_SD)) {
      Serial.println("Microphone initialization failed, continuing anyway (for testing)");
  }

  if(!setupWifi()) {
    Serial.println("Failed to setup WiFi");
    while (!setupWifi()) {
      Serial.println("Retrying WiFi setup...");
      delay(1000);
    }
  }

  // Init wirelessDetection
  if (!WirelessDetection::begin()){
    Serial.println("Wireless detection init failed :_(\nContinuing without wireless detecting");
  }

  mqttClient.setServer("192.168.0.181", MQTT_BROKER_PORT);
  mqttClient.setBufferSize(512);  
}

bool setupWifi() {
  // Read the real factory Wi-Fi STA MAC.
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.print("WiFi MAC: ");
  Serial.println(macStr);

  // Start Wi-Fi station mode.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(200);

  // unique id.
  clientId = "ESP32-" + String(macStr);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // Do not hang forever; report the failure reason.
    if (millis() - start > 20000) {
      Serial.println();
      Serial.printf("WiFi connection failed, status=%d\n", WiFi.status());
      return false;
    }
  }

  Serial.println();
  Serial.print("Connected to WiFi, IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void loop() {
  mqttClient.loop();

  // get microphone samples
  mic.update();

  //Sample sensors multiple times within the publish interval.
  if (millis() - lastSampleTime >= TIME_BETWEEN_SAMPLES) {
    lastSampleTime = millis();

    // Update PIR sensors
    pir1.update();
    pir2.update();
    pir3.update();

    potmeter.update();
    // Serial.print("Potmeter value: ");
    // Serial.println(potmeter.computeAverage());

    bmeWrapper.update();
  }



  // Publish to MQTT bus.
  if(millis() - lastPublishTime >= TIME_INTERVAL) {
    lastPublishTime = millis();

    float soundlevel = mic.getAverageSoundLevelDbFS();

    uint8_t label = potmeter.computeAverage();

    // // Read sensor data
    float temperature = bmeWrapper.computeAverageTemperatureC();
    float humidity = bmeWrapper.computeAverageHumidityPercent();
    float pressure = bmeWrapper.computeAveragePressureHpa();
    float iaq = bmeWrapper.computeAverageIAQ();
    float voc = bmeWrapper.computeAverageBreathVocPpm();
    float co2Equivalent = bmeWrapper.computeAverageCo2EquivalentPpm();
    uint8_t accuracy = bmeWrapper.getIAQAccuracy();

    uint8_t pir1Activity = pir1.computeActivityRatio();
    uint8_t pir2Activity = pir2.computeActivityRatio();
    uint8_t pir3Activity = pir3.computeActivityRatio();
    

    // String payload = preparePayload(temperature, humidity, pressure, iaq, voc, co2Equivalent, accuracy, pir1Activity, pir2Activity, pir3Activity);
    WirelessStats wirelessStats = WirelessDetection::getAndResetStats();
    String payload = preparePayload(
      temperature, 
      humidity, 
      pressure, 
      iaq, 
      voc, 
      co2Equivalent, 
      accuracy, 
      pir1Activity, 
      pir2Activity, 
      pir3Activity, 
      soundlevel, 
      label, 
      wirelessStats.wifiPackets, 
      wirelessStats.bleAdverts
    );
    // String payload = preparePayload(20.0, 50.0, 1013.25, 100.0, 0.5, 400.0, 3, 1, 2, 3, soundlevel);
    publishMQTT(MQTT_BUILDING_NAME, MQTT_ROOM_NUMBER, payload.c_str());
  }
}

String preparePayload(float temperature, float humidity, float pressure, float iaq, float voc, float co2Equivalent, uint8_t accuracy, uint8_t pir1Activity, uint8_t pir2Activity, uint8_t pir3Activity, float soundLevel, uint8_t busynessLabel, uint32_t wifiPackets, uint32_t bleAdverts) {
  String payload = "{";
  payload += "\"temperature\": " + String(temperature) + ",";
  payload += "\"humidity\": " + String(humidity) + ",";
  payload += "\"pressure\": " + String(pressure) + ",";
  payload += "\"iaq\": " + String(iaq) + ",";
  payload += "\"voc\": " + String(voc) + ",";
  payload += "\"co2Equivalent\": " + String(co2Equivalent) + ",";
  // payload += "\"accuracy\": " + String(accuracy) + ",";

  // pirs are a list, with id.
  payload += "\"pirs\": [";
  payload += "{\"ID\": 1, \"activity\": " + String(pir1Activity) + "},";
  payload += "{\"ID\": 2, \"activity\": " + String(pir2Activity) + "},";
  payload += "{\"ID\": 3, \"activity\": " + String(pir3Activity) + "}";
  payload += "],";
  
  payload += "\"mic_vol\": " + String(soundLevel, 2) + ",";
  
  payload += "\"wifiPackets\": " + String(wifiPackets) + ",";
  payload += "\"bleAdverts\": " + String(bleAdverts) + ",";
  payload += "\"busynessLabel\": " + String(busynessLabel);

  payload += "}";
  return payload;
}

bool mqttEnsureConnected() {
  if (mqttClient.connected()) return true;
 
  static bool attempted = false;
  static unsigned long lastAttempt = 0;
  if (attempted && millis() - lastAttempt < 5000) return false;
  attempted = true;
  lastAttempt = millis();
 
  Serial.print("Connecting to MQTT broker...");
  if (mqttClient.connect(clientId.c_str())) {
    Serial.println("connected");
    return true;
  }
  Serial.printf("failed, rc=%d\n", mqttClient.state());  // -2 = could not reach broker
  return false;
}
 
bool publishMQTT(const char* building, const char* room, const char* payload) {
  if (!mqttEnsureConnected()) return false;
 
  char topic[64];
  snprintf(topic, sizeof(topic), "sensors/%s/%s", building, room);
 
  bool ok = mqttClient.publish(topic, payload);  // QoS 0, not retained
  Serial.printf("%s %s: %s\n", ok ? "Published" : "Publish FAILED", topic, payload);
  return ok;
}
