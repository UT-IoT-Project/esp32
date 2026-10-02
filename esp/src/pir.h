#pragma once
#include <Arduino.h>

class PIR {
public:
    explicit PIR(uint8_t pin, uint8_t bufSize = 10);

    // Read current raw state (HIGH/LOW)
    bool getState() const;

    // Call this periodically to sample the sensor
    void update();

    // Activity ratio as percentage (0–100)
    uint8_t computeActivityRatio() const;

    // Reset internal buffer to all false
    void resetBuffer();

private:
    const uint8_t _pin;
    const uint8_t _bufferSize;
    uint8_t _bufferIndex = 0;
    bool* _buffer;                  // dynamically allocated once in constructor
};