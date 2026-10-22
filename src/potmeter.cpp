#include "potmeter.h"

Potmeter::Potmeter(uint8_t pin, uint8_t bufSize)
    : _pin(pin),
        _bufferSize(bufSize),
        _buffer(new uint8_t[bufSize])
{
    pinMode(_pin, INPUT);
    resetBuffer();
}

int Potmeter::getState() const {
    // read Potmeter state, map from 0-3
    const int reading = analogRead(_pin);
    Serial.print("Analog read: ");
    Serial.println(reading);
    return map(reading, 0, 4095, 0, 3);
}

void Potmeter::update() {
    // store 1 new sample in buffer.
    _buffer[_bufferIndex] = getState();

    _bufferIndex++;

    if (_bufferIndex >= _bufferSize) {
        _bufferIndex = 0;
    }
}

void Potmeter::resetBuffer() {
    // set buffer to 0
    _bufferIndex = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        _buffer[i] = false;
    }
}

uint8_t Potmeter::computeActivityRatio() const {
    // count true values and divide by the buffer size to get a percentage.
    uint8_t average = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        if (_buffer[i]) {
            average += _buffer[i];
        }
    }
    // avoid divide by zero.
    if (_bufferSize == 0) {
        return 0;
    }
    // Percentage (0–100)
    return static_cast<uint8_t>(average / _bufferSize);
}