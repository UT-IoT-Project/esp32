#include "wirelessDetection.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <atomic>

namespace{
    std::atomic<uint32_t> packetsCount{0};
    std::atomic<uint32_t> bleCount{0};

    BLEScan* bleScan = nullptr;

    // Counting the Wi-Fi
    void onWifiPacket(
        void* buffer,
        wifi_promiscuous_pkt_type_t packetType
    ){
        if (buffer == nullptr){
            return;
        }
        packetsCount.fetch_add(1, std::memory_order_relaxed);
    }
    // Counting the Blue teeth
    class advertisementCallbacks : public BLEAdvertisedDeviceCallbacks{
        void onResult(BLEAdvertisedDevice device) override{
            bleCount.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // A seperate (not the main) loop for BLE scanning
    advertisementCallbacks bleCallbacks;
    void bluetoothTask(void* parameter){
        for (;;){
            bleScan->start(5, false);       // Scan for 5 seconds
            bleScan->clearResults();        // release scan results
            vTaskDelay(pdMS_TO_TICKS(100)); // Short pause before scanning again
        }
    }
}

namespace WirelessDetection{
    //Init Wi-Fi
    bool begin(){
        if (WiFi.status() != WL_CONNECTED){ // Check that wifi is actually connected
            Serial.println("Cannot do the wireless detection because WiFi is not connected :_(");
            return false;
        }

        wifi_promiscuous_filter_t filter = {};  // Configure Wi-Fi packet monitoring without losing connection or changing the Wi-Fi channel
        filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;

        esp_err_t result = esp_wifi_set_promiscuous_filter(&filter);

        if (result != ESP_OK){
            Serial.printf("Could not set Wi-Fi filter: %d\n", result);
            return false;
        }

        result = esp_wifi_set_promiscuous_rx_cb(&onWifiPacket);

        if (result != ESP_OK){
            Serial.printf("Could not set Wi-Fi callback: %d\n", result);
            return false;
        }


        result = esp_wifi_set_promiscuous(true);

        if (result != ESP_OK){
            Serial.printf("Could not enable Wi-Fi monitoring: %d\n", result);
            return false;
        }
        
        Serial.println("Wireless Wi-Fi detection succesfully enabled");

        //Init BLE
        BLEDevice::init("");
        bleScan = BLEDevice::getScan();

        if (bleScan == nullptr){
            Serial.println("Wireless detection BLE scanner init failed :_(");
            return false;
        }
        bleScan->setAdvertisedDeviceCallbacks(&bleCallbacks, true); // Count (repeated and new) advertisements
        bleScan->setActiveScan(false);                              // Do not send scan requests
        bleScan->setInterval(100);
        bleScan->setWindow(99);

        //BLE scan task
        BaseType_t taskResult = xTaskCreate(bluetoothTask, "BluetoothScanner", 4096, nullptr, 1, nullptr);

        if (taskResult != pdPASS){
            Serial.println("Wireless detection failed to create the Bluetooth task");
            return false;
        }

        Serial.println("Wireless detection BLE monitoring is enabled :-D");
        return true;
    }

    WirelessStats getAndResetStats(){
        WirelessStats stats;
        stats.wifiPackets = packetsCount.exchange(0, std::memory_order_relaxed);
        stats.bleAdverts = bleCount.exchange(0, std::memory_order_relaxed);
        return stats;
    }
}

