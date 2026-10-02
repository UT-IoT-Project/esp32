#include "co2sensor.h"
// include real CO2 sensor library here.


CO2Sensor::CO2Sensor(uint8_t pin, uint8_t bufSize)
    : _pin(pin),
      _bufferSize(bufSize),
      _buffer(new bool[bufSize])
{
    pinMode(_pin, INPUT);

    // init co2 sensor object here.
    // COOLCLASS co2Sensor(_pin);

    resetBuffer();
}

uint8_t CO2Sensor::getValue() const {
    // read CO2 sensor value.

    // co2sensor.readCO2(); 
    return 1;
}

void CO2Sensor::update() {
    // store 1 new sample in buffer.
    _buffer[_bufferIndex] = getValue();

    _bufferIndex++;
    if (_bufferIndex >= _bufferSize) {
        _bufferIndex = 0;
    }
}

void CO2Sensor::resetBuffer() {
    // set buffer to 0
    _bufferIndex = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        _buffer[i] = false;
    }
}

uint8_t CO2Sensor::computeAverage() const {
    // count true values and divide by the buffer size to get a percentage.
    uint8_t count = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        if (_buffer[i]) {
            count++;
        }
    }
    // avoid divide by zero.
    if (_bufferSize == 0) {
        return 0;
    }
    // Percentage (0–100)
    return static_cast<uint8_t>((static_cast<uint16_t>  (count) * 100) / _bufferSize);
}

uint8_t CO2Sensor::computeMaximum() const {
    // find maximum value in buffer.
    uint8_t maxValue = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        if (_buffer[i] > maxValue) {
            maxValue = _buffer[i];
        }
    }
    return maxValue;
}