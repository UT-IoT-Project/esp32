#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class Microphone {
public:
    Microphone();

    bool begin(int sckPin, int wsPin, int sdPin, uint32_t sampleRate = 16000);

    // Read audio samples and accumulate sound energy, does nothing if initialization failed
    void update();

    // get the interval's RMS level in dBFS and reset the accumulated measurements
    float getAverageSoundLevelDbFS();

    bool isInitialized() const;

private:
    static constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
    static constexpr size_t BUFFER_SAMPLES = 512;

    int32_t _buffer[BUFFER_SAMPLES];

    double _sumSquares;
    uint64_t _sampleCount;
    bool _initialized;
};