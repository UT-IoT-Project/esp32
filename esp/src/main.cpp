// Topfile for the ESP32.
// connects to sensors which are polled frequently
// sends these to the MQTT broker on the gateway

// INCLUDES
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// GLOBAL PARAMS
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define TIME_INTERVAL 10000
#define SAMPLES_PER_INTERVAL 10
#define TIME_BETWEEN_SAMPLES (TIME_INTERVAL / SAMPLES_PER_INTERVAL)  // 1 second between samples

#define MQTT_BUILDING_NAME "raijn"
#define MQTT_ROOM_NUMBER "1004"

#define MQTT_BROKER_ADDRESS "gateway.local"
#define MQTT_BROKER_PORT 1883

#define PIR1_PIN 1
#define PIR2_PIN 2
#define PIR3_PIN 3
#define PIR_BUFFER_SIZE SAMPLES_PER_INTERVAL    // timeinterval / number of samples  10 per minute.

#define CO2_BUFFER_SIZE SAMPLES_PER_INTERVAL

// OBJECTS
WiFiClient espClient;
PubSubClient mqttClient(espClient);
CO2Sensor co2Sensor(10, 12);              // probably need to use the library of the sensor.
PIR pir1(PIR1_PIN, PIR_BUFFER_SIZE);
PIR pir2(PIR2_PIN, PIR_BUFFER_SIZE);
PIR pir3(PIR3_PIN, PIR_BUFFER_SIZE);


// TIMERS
unsigned long lastPublishTime = 0;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  // while (!Serial) {
  //   ; // wait for serial port to connect (mostly for safety)
  // }

  Serial.println();
  Serial.println("Starting ESP32...");

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

    // Read CO2 sensor
    co2Sensor.readCO2();  // Assuming this function reads and stores the value internally.
  }

  // might need like a pot meter to select 3 busyness levels. this can be used to label the data. (instead of doing this via the webserver or something.)

  // Publish to MQTT bus.
  if(millis() - lastPublishTime >= TIME_INTERVAL) {
    lastPublishTime = millis();

    // Read sensor data
    float co2Level = co2Sensor.readCO2();
    uint8_t pir1Activity = pir1.computeActivityRatio();
    uint8_t pir2Activity = pir2.computeActivityRatio();
    uint8_t pir3Activity = pir3.computeActivityRatio();

    String payload = preparePayload(co2Level, pir1Activity, pir2Activity, pir3Activity);
    publishMQTT(MQTT_BUILDING_NAME, MQTT_ROOM_NUMBER, payload);
  }
}

void preparePayload(float co2Level, uint8_t pir1Activity, uint8_t pir2Activity, uint8_t pir3Activity) {
  String payload = "{";
  payload += "\"co2\": " + String(co2Level) + ",";
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