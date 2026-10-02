#pragma once
#include <Arduino.h>

// THIS WILL BE A WRAPPER CLASS THAT STORES VALUES IN A BUFFER.

class CO2Sensor {
public:
    explicit CO2Sensor(uint8_t pin, uint8_t bufSize = 10);

    // Read current raw state (HIGH/LOW)
    uint8_t getValue() const;

    // Call this periodically to sample the sensor
    void update();

    // Average value
    uint8_t computeAverage() const;

    uint8_t computeMaximum() const;

    // Reset internal buffer to all false
    void resetBuffer();

private:
    const uint8_t _pin;
    const uint8_t _bufferSize;
    uint8_t _bufferIndex = 0;
    bool* _buffer;                  // dynamically allocated once in constructor

    // probably a buffer for co2 values, temp and humidity.
    // uint16_t* _co2Buffer;
    // uint16_t* _tempBuffer;
    // uint16_t* _humidityBuffer;

    // COOLCLASS co2Sensor(_pin);

    // I2C bus (WIRE)

};