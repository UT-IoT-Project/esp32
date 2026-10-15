
#include "Microphone.h"
#include <math.h>

Microphone::Microphone()
    : _sumSquares(0.0), _sampleCount(0), _initialized(false) {}

bool Microphone::begin(int sckPin, int wsPin, int sdPin, uint32_t sampleRate) {
    i2s_config_t config = {};

    config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    config.sample_rate = sampleRate;
    config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    config.dma_buf_count = 8;
    config.dma_buf_len = 256;
    config.use_apll = false;
    config.tx_desc_auto_clear = false;
    config.fixed_mclk = 0;

    esp_err_t result = i2s_driver_install(I2S_PORT, &config, 0, nullptr);

    if (result != ESP_OK) {
        Serial.printf("I2S driver install failed: %d\n", result);
        return false;
    }

    i2s_pin_config_t pins = {};

    pins.bck_io_num = sckPin;
    pins.ws_io_num = wsPin;
    pins.data_out_num = I2S_PIN_NO_CHANGE;
    pins.data_in_num = sdPin;

    result = i2s_set_pin(I2S_PORT, &pins);

    if (result != ESP_OK) {
        Serial.printf("I2S pin setup failed: %d\n", result);
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }

    // discard initial buffered data
    i2s_zero_dma_buffer(I2S_PORT);

    _initialized = true;
    Serial.println("INMP441 initialized");
    return true;
}

void Microphone::update() {
    if (!_initialized) {
        return;
    }

    size_t bytesRead = 0;

    esp_err_t result = i2s_read(
        I2S_PORT,
        _buffer,
        sizeof(_buffer),
        &bytesRead,
        0
    );

    if (result != ESP_OK) {
        return;
    }

    size_t slotsRead = bytesRead / sizeof(int32_t);

    // mic L/R tied to GND: left channel, stereo slots are interleaved left, right.
    for (size_t i = 0; i + 1 < slotsRead; i += 2) {
        int32_t sample = _buffer[i] >> 8;
        double value = static_cast<double>(sample);

        _sumSquares += value * value;
        ++_sampleCount;
    }
}

float Microphone::getAverageSoundLevelDbFS() {
    if (!_initialized || _sampleCount == 0) {
        return -100.0f;
    }

    double meanSquare = _sumSquares / _sampleCount;
    double rms = sqrt(meanSquare);
    double normalizedRms = rms / 8388608.0;

    float dbFS = -100.0f;

    if (normalizedRms > 0.000001) {
        dbFS = 20.0f * log10(normalizedRms);
    }

    _sumSquares = 0.0;
    _sampleCount = 0;

    return dbFS;
}

bool Microphone::isInitialized() const {
    return _initialized;
}