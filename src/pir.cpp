#include "pir.h"

PIR::PIR(uint8_t pin, uint8_t bufSize)
    : _pin(pin),
      _bufferSize(bufSize),
      _buffer(new bool[bufSize])
{
    pinMode(_pin, INPUT);
    resetBuffer();
}

bool PIR::getState() const {
    // read PIR state.
    return digitalRead(_pin) == HIGH;
}

void PIR::update() {
    // store 1 new sample in buffer.
    _buffer[_bufferIndex] = getState();

    _bufferIndex++;
    if (_bufferIndex >= _bufferSize) {
        _bufferIndex = 0;
    }
}

void PIR::resetBuffer() {
    // set buffer to 0
    _bufferIndex = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        _buffer[i] = false;
    }
}

uint8_t PIR::computeActivityRatio() const {
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