#include "BMEWrapper.h"

BMEWrapper::BMEWrapper(TwoWire& wire)
    : _wire(wire)
{
    resetBuffer();
}

bool BMEWrapper::begin() {
    // Use BME68X_I2C_ADDR_LOW for 0x76,
    // or BME68X_I2C_ADDR_HIGH for 0x77.
    _bsec.begin(BME68X_I2C_ADDR_HIGH, _wire);

    if (_bsec.bsecStatus != BSEC_OK || _bsec.bme68xStatus != BME68X_OK) {
        Serial.printf("BSEC init failed: bsec=%d bme68x=%d\n",
                      _bsec.bsecStatus, _bsec.bme68xStatus);
        return false;
    }

    bsec_virtual_sensor_t sensorList[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY
    };

    _bsec.updateSubscription(
        sensorList,
        sizeof(sensorList) / sizeof(sensorList[0]),
        BSEC_SAMPLE_RATE_LP
    );

    if (_bsec.bsecStatus != BSEC_OK) {
        Serial.printf("BSEC subscription failed: %d\n", _bsec.bsecStatus);
        return false;
    }

    return true;
}

void BMEWrapper::update() {
    // store 1 new sample in buffer.
    // BSEC returns false when no new measurement is ready yet.
    if (!_bsec.run()) {
        return;
    }

    _temperatureC = _bsec.temperature;
    _humidityPercent = _bsec.humidity;
    _pressureHpa = _bsec.pressure / 100.0f;
    _iaq = _bsec.iaq;
    _iaqAccuracy = _bsec.iaqAccuracy;
    _breathVocPpm = _bsec.breathVocEquivalent;
    _co2EquivalentPpm = _bsec.co2Equivalent;
    _gasResistanceKohm = _bsec.gasResistance / 1000.0f;

    _temperatureBuffer[_bufferIndex] = _temperatureC;
    _humidityBuffer[_bufferIndex] = _humidityPercent;
    _pressureBuffer[_bufferIndex] = _pressureHpa;
    _iaqBuffer[_bufferIndex] = _iaq;
    _breathVocBuffer[_bufferIndex] = _breathVocPpm;
    _co2EquivalentBuffer[_bufferIndex] = _co2EquivalentPpm;

    _bufferIndex++;
    if (_bufferIndex >= CO2_BUFFER_SIZE) {
        _bufferIndex = 0;
    }

    if (_sampleCount < CO2_BUFFER_SIZE) {
        _sampleCount++;
    }
}

float BMEWrapper::getTemperatureC() const {
    return _temperatureC;
}

float BMEWrapper::getHumidityPercent() const {
    return _humidityPercent;
}

float BMEWrapper::getPressureHpa() const {
    return _pressureHpa;
}

float BMEWrapper::getIAQ() const {
    return _iaq;
}

uint8_t BMEWrapper::getIAQAccuracy() const {
    return _iaqAccuracy;
}

float BMEWrapper::getBreathVocPpm() const {
    return _breathVocPpm;
}

float BMEWrapper::getCo2EquivalentPpm() const {
    return _co2EquivalentPpm;
}

float BMEWrapper::getGasResistanceKohm() const {
    return _gasResistanceKohm;
}

float BMEWrapper::computeAverageTemperatureC() const {
    if (_sampleCount == 0) return NAN;

    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _temperatureBuffer[i];
    }
    return sum / _sampleCount;
}

float BMEWrapper::computeAverageHumidityPercent() const {
    if (_sampleCount == 0) return NAN;

    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _humidityBuffer[i];
    }
    return sum / _sampleCount;
}

float BMEWrapper::computeAveragePressureHpa() const {
    if (_sampleCount == 0) return NAN;

    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _pressureBuffer[i];
    }
    return sum / _sampleCount;
}

float BMEWrapper::computeAverageIAQ() const {
    if (_sampleCount == 0) return NAN;

    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _iaqBuffer[i];
    }
    return sum / _sampleCount;
}

float BMEWrapper::computeAverageBreathVocPpm() const {
    if (_sampleCount == 0) return NAN;

    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _breathVocBuffer[i];
    }
    return sum / _sampleCount;
}

float BMEWrapper::computeAverageCo2EquivalentPpm() const {
    if (_sampleCount == 0) return NAN;

    float sum = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        sum += _co2EquivalentBuffer[i];
    }
    return sum / _sampleCount;
}

void BMEWrapper::resetBuffer() {
    // set buffer to 0
    _bufferIndex = 0;
    _sampleCount = 0;

    _temperatureBuffer.fill(0.0f);
    _humidityBuffer.fill(0.0f);
    _pressureBuffer.fill(0.0f);
    _iaqBuffer.fill(0.0f);
    _breathVocBuffer.fill(0.0f);
    _co2EquivalentBuffer.fill(0.0f);
}