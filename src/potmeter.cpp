#include "potmeter.h"
#include "config.h"

Potmeter::Potmeter(uint8_t pin, uint8_t led1, uint8_t led2, uint8_t bufSize)
    : _pin(pin),
        _led1(led1),
        _led2(led2),
        _bufferSize(bufSize),
        _buffer(new uint8_t[bufSize])
{
    pinMode(_pin, INPUT);
    pinMode(_led1, OUTPUT);
    pinMode(_led2, OUTPUT);
    resetBuffer();
}

uint8_t Potmeter::getState() const {
    // read Potmeter state, map from 0-3
    const int reading = analogRead(_pin);
    const uint8_t mappedValue = map(reading, 0, 4095, 0, NUM_OF_LABELS); // we want 4 labels. 0 - 3, as the 4 is excluded.
    switch (mappedValue) {
        case 0:
            digitalWrite(_led1, LOW);
            digitalWrite(_led2, LOW);
            break;
        case 1:
            digitalWrite(_led1, HIGH);
            digitalWrite(_led2, LOW);
            break;
        case 2:
            digitalWrite(_led1, LOW);
            digitalWrite(_led2, HIGH);
            break;
        case 3:
            digitalWrite(_led1, HIGH);
            digitalWrite(_led2, HIGH);
            break;
    }
    return mappedValue;
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

uint8_t Potmeter::computeAverage() const {
    // get the integer label 0-3 of the busyness level.
    uint32_t sum = 0;
    for (uint8_t i = 0; i < _bufferSize; i++) {
        sum += _buffer[i];
    }

    return static_cast<uint8_t>(sum / _bufferSize);
}