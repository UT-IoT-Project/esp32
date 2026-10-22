#pragma once

#include <Arduino.h>
#include "driver/i2s_std.h"

class Microphone {
public:
    Microphone();

    bool begin(int sckPin, int wsPin, int sdPin, uint32_t sampleRate = 16000);
    void update();
    float getAverageSoundLevelDbFS();
    bool isInitialized() const;

private:
    static constexpr size_t BUFFER_SAMPLES = 512;

    i2s_chan_handle_t _rxHandle = nullptr;
    int32_t _buffer[BUFFER_SAMPLES];

    double _sumSquares = 0.0;
    uint64_t _sampleCount = 0;
    bool _initialized = false;
};