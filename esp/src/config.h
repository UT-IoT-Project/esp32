#pragma once

// GLOBAL PARAMS
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define TIME_INTERVAL 10000
#define SAMPLES_PER_INTERVAL 10
#define TIME_BETWEEN_SAMPLES (TIME_INTERVAL / SAMPLES_PER_INTERVAL)  // 1 second between samples

#define PIR_BUFFER_SIZE SAMPLES_PER_INTERVAL    // timeinterval / number of samples  10 per minute.
#define CO2_BUFFER_SIZE SAMPLES_PER_INTERVAL

#define MQTT_BUILDING_NAME "raijn"
#define MQTT_ROOM_NUMBER "1004"

#define MQTT_BROKER_ADDRESS "gateway.local"
#define MQTT_BROKER_PORT 1883

#define CO2_PIN 4
#define PIR1_PIN 1
#define PIR2_PIN 2
#define PIR3_PIN 3