#pragma once

// GLOBAL PARAMS
#define TIME_INTERVAL 10000
#define SAMPLES_PER_INTERVAL 10
#define TIME_BETWEEN_SAMPLES (TIME_INTERVAL / SAMPLES_PER_INTERVAL)  // 1 second between samples

#define PIR_BUFFER_SIZE SAMPLES_PER_INTERVAL    // timeinterval / number of samples  10 per minute.
#define CO2_BUFFER_SIZE SAMPLES_PER_INTERVAL

#define MQTT_BUILDING_NAME "ravelijn"
#define MQTT_ROOM_NUMBER "1004"

#define MQTT_BROKER_ADDRESS "busy-raspi"
#define MQTT_BROKER_PORT 1883

#define CO2_PIN 4
#define PIR1_PIN 16
#define PIR2_PIN 17
#define PIR3_PIN 4

// I2C pins of the BME688 sensor
#define BME_SCL 22
#define BME_SDA 21

// I2S pins of the micrphone (INMP441)
#define I2S_SCK 18
#define I2S_SD 23
#define I2S_WS 19

