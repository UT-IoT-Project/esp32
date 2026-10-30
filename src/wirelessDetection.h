#pragma once
#include <Arduino.h>

struct WirelessStats{
    uint32_t wifiPackets;
    uint32_t bleAdverts;
};

namespace WirelessDetection{
    bool begin();               // begin the scanning
    WirelessStats getAndResetStats();
}