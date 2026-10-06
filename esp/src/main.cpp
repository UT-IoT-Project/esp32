// Topfile for the ESP32.
// connects to sensors which are polled frequently
// sends these to the MQTT broker on the gateway

// INCLUDES
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "BMEWrapper.h"
#include "pir.h"

// check this file for #define constants.
#include "config.h"

// #define BME_SDA 21
// #define BME_SCL 22

// OBJECTS
TwoWire i2cWire = TwoWire(0);  // I2C controller 0
BMEWrapper bmeWrapper(i2cWire);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

PIR pir1(PIR1_PIN, PIR_BUFFER_SIZE);
PIR pir2(PIR2_PIN, PIR_BUFFER_SIZE);
PIR pir3(PIR3_PIN, PIR_BUFFER_SIZE);


// TIMERS
unsigned long lastPublishTime = 0;
unsigned long lastSampleTime = 0;

// functions
void setupWifi();
String preparePayload(float temperature, float humidity, float pressure, float iaq, float voc, float co2Equivalent, uint8_t accuracy, uint8_t pir1Activity, uint8_t pir2Activity, uint8_t pir3Activity);
void publishMQTT(const char* building, const char* room, const String& payload);

void setup() {
  Serial.begin(115200);
  delay(1000);
  // while (!Serial) {
  //   ; // wait for serial port to connect (mostly for safety)
  // }

  Serial.println();
  Serial.println("Starting ESP32...");

  // sensors
  bmeWrapper.begin();

  setupWifi();

  mqttClient.setServer(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);
}

void setupWifi(){
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi, IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Sample sensors multiple times within the publish interval.
  if (millis() - lastSampleTime >= TIME_BETWEEN_SAMPLES) {
    lastSampleTime = millis();

    // Update PIR sensors
    pir1.update();
    pir2.update();
    pir3.update();

    bmeWrapper.update();
  }

  // might need like a pot meter to select 3 busyness levels. this can be used to label the data. (instead of doing this via the webserver or something.)

  // Publish to MQTT bus.
  if(millis() - lastPublishTime >= TIME_INTERVAL) {
    lastPublishTime = millis();

    // Read sensor data
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

    String payload = preparePayload(temperature, humidity, pressure, iaq, voc, co2Equivalent, accuracy, pir1Activity, pir2Activity, pir3Activity);
    publishMQTT(MQTT_BUILDING_NAME, MQTT_ROOM_NUMBER, payload);
  }
}

String preparePayload(float temperature, float humidity, float pressure, float iaq, float voc, float co2Equivalent, uint8_t accuracy, uint8_t pir1Activity, uint8_t pir2Activity, uint8_t pir3Activity) {
  String payload = "{";
  payload += "\"temperature\": " + String(temperature) + ",";
  payload += "\"humidity\": " + String(humidity) + ",";
  payload += "\"pressure\": " + String(pressure) + ",";
  payload += "\"iaq\": " + String(iaq) + ",";
  payload += "\"voc\": " + String(voc) + ",";
  payload += "\"co2Equivalent\": " + String(co2Equivalent) + ",";
  payload += "\"accuracy\": " + String(accuracy) + ",";
  payload += "\"pir1\": " + String(pir1Activity) + ",";
  payload += "\"pir2\": " + String(pir2Activity) + ",";
  payload += "\"pir3\": " + String(pir3Activity);
  payload += "}";
  return payload;
}

void publishMQTT(const char* building, const char* room, const String& payload) {
  // Connect to MQTT broker if not connected
  if (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    while (!mqttClient.connected()) {
      if (mqttClient.connect("ESP32Client")) {
        Serial.println("connected");
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }

  // Publish the payload
  String topic = String(building) + "/" + String(room) + "/sensors";
  if (mqttClient.publish(topic.c_str(), payload.c_str())) {
    Serial.println("MQTT message published successfully");
  } else {
    Serial.println("Failed to publish MQTT message");
  }
}