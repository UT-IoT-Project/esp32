#pragma once
#include <Arduino.h>

class Potmeter {
public:
    Potmeter(uint8_t pin, uint8_t bufSize);

    int getState() const;
    uint8_t computeActivityRatio() const;
    void update();
    void resetBuffer();

private:
    const uint8_t _pin;
    const uint8_t _bufferSize;
    uint8_t _bufferIndex = 0;
    uint8_t* _buffer;                  // dynamically allocated once in constructor
};