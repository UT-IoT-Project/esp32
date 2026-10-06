#pragma once
#include <Arduino.h>
#include <Wire.h>

#include <array>
#include "config.h"

#include "bsec.h"

// THIS WILL BE A WRAPPER CLASS THAT STORES VALUES IN A BUFFER.
class BMEWrapper {
public:
    explicit BMEWrapper(TwoWire& wire);

    bool begin();

    // Call this periodically to sample the sensor
    void update();

    // Average value
    float computeAverageTemperatureC() const;
    float computeAverageHumidityPercent() const;
    float computeAveragePressureHpa() const;
    float computeAverageIAQ() const;
    float computeAverageBreathVocPpm() const;
    float computeAverageCo2EquivalentPpm() const;

    // Reset internal buffer to all false
    void resetBuffer();

    // BME680/BSEC getters
    float getTemperatureC() const;
    float getHumidityPercent() const;
    float getPressureHpa() const;
    float getIAQ() const;
    uint8_t getIAQAccuracy() const;
    float getBreathVocPpm() const;
    float getCo2EquivalentPpm() const;
    float getGasResistanceKohm() const;

private:
    // refrence to the I2C bus.
    TwoWire& _wire;

    // Sensor object
    Bsec _bsec;

    // Buffers.
    uint8_t _bufferIndex = 0;
    uint8_t _sampleCount = 0;

    std::array<float, CO2_BUFFER_SIZE> _temperatureBuffer;
    std::array<float, CO2_BUFFER_SIZE> _humidityBuffer;
    std::array<float, CO2_BUFFER_SIZE> _pressureBuffer;
    std::array<float, CO2_BUFFER_SIZE> _iaqBuffer;
    std::array<float, CO2_BUFFER_SIZE> _breathVocBuffer;
    std::array<float, CO2_BUFFER_SIZE> _co2EquivalentBuffer;

    // Current readings
    float _temperatureC = NAN;
    float _humidityPercent = NAN;
    float _pressureHpa = NAN;
    float _iaq = NAN;
    uint8_t _iaqAccuracy = 0;
    float _breathVocPpm = NAN;
    float _co2EquivalentPpm = NAN;
    float _gasResistanceKohm = NAN;
};