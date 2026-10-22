#include "Microphone.h"
#include <math.h>

Microphone::Microphone() {}

bool Microphone::begin(int sckPin, int wsPin, int sdPin, uint32_t sampleRate) {
    i2s_chan_config_t chanConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    if (i2s_new_channel(&chanConfig, nullptr, &_rxHandle) != ESP_OK) {
        Serial.println("I2S channel creation failed");
        return false;
    }

    i2s_std_config_t stdConfig = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = static_cast<gpio_num_t>(sckPin),
            .ws = static_cast<gpio_num_t>(wsPin),
            .dout = I2S_GPIO_UNUSED,
            .din = static_cast<gpio_num_t>(sdPin),
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    if (i2s_channel_init_std_mode(_rxHandle, &stdConfig) != ESP_OK) {
        Serial.println("I2S std mode init failed");
        i2s_del_channel(_rxHandle);
        _rxHandle = nullptr;
        return false;
    }

    if (i2s_channel_enable(_rxHandle) != ESP_OK) {
        Serial.println("I2S channel enable failed");
        i2s_del_channel(_rxHandle);
        _rxHandle = nullptr;
        return false;
    }

    _initialized = true;
    Serial.println("INMP441 initialized");
    return true;
}

void Microphone::update() {
    if (!_initialized) {
        return;
    }

    size_t bytesRead = 0;

    if (i2s_channel_read(
            _rxHandle,
            _buffer,
            sizeof(_buffer),
            &bytesRead,
            0
        ) != ESP_OK) {
        return;
    }

    size_t slotsRead = bytesRead / sizeof(int32_t);

    // INMP441 with L/R tied to GND: use left-channel samples.
    for (size_t i = 0; i < slotsRead; i += 2) {
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