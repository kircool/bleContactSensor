#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include "esp_sleep.h"
#include "driver/gpio.h" // Essential for working with GPIO during sleep

#define SENSOR_PIN GPIO_NUM_4

void goToSleep() {
    // 1. Read the final state BEFORE going to sleep
    // Add a small delay to avoid contact bounce
    delay(50); 
    int currentState = digitalRead(SENSOR_PIN);

    // 2. Configure the wake-up
    // If it's currently 1 (open), wake up when it's 0 (grounded)
    // If it's currently 0 (closed), wake up when it's 1 (open)
    esp_deep_sleep_enable_gpio_wakeup(1ULL << SENSOR_PIN, 
        currentState ? ESP_GPIO_WAKEUP_GPIO_LOW : ESP_GPIO_WAKEUP_GPIO_HIGH);

    // 3. CRITICAL MOMENT: Configure the pin during sleep
    // Enable power-on pull-up (to always have a 1 when open)
    gpio_pullup_en(SENSOR_PIN); 
    gpio_pulldown_dis(SENSOR_PIN);
    
    // Fix the pin configuration for Deep Sleep
    gpio_hold_en(SENSOR_PIN); 

    Serial.println("Going to sleep...");
    Serial.flush();
    esp_deep_sleep_start();
}

void setup() {
    // At the beginning of setup, we need to REMOVE the pin fixation, otherwise we won't be able to read it
    gpio_hold_dis(SENSOR_PIN);

    Serial.begin(115200);
    pinMode(SENSOR_PIN, INPUT_PULLUP);

    // Wait for the level to stabilize
    delay(10);
    int sensorState = digitalRead(SENSOR_PIN);
    
    // --- Further BLE code (as in the previous answer) ---
    BLEDevice::init("ESP32-C3_DoorSensor");
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData oAdvData;
    
    // Формируем бинарный пакет:
    // Байты 0-1: ID производителя (0xFFFF)
    // Байт 2: Состояние (1 или 0)
    uint8_t mfg_data[3];
    mfg_data[0] = 0xFF; 
    mfg_data[1] = 0xFF;
    mfg_data[2] = (sensorState == HIGH) ? 0x01 : 0x00;

// Передаем как массив байтов
oAdvData.setManufacturerData(std::string((char*)mfg_data, 3));

pAdvertising->setAdvertisementData(oAdvData);
pAdvertising->start();

    Serial.printf("Status [%s] sent to the air.\n", (sensorState == HIGH) ? "01" : "00");

    delay(1500); // Give the packet time to leave
    pAdvertising->stop();

    goToSleep();
}

void loop() {}