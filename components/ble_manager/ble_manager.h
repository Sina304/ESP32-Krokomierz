#pragma once

#include <stdint.h>

// Klasa odpowiedzialna za komunikację Bluetooth Low Energy.
// Udostępnia licznik kroków telefonu przez BLE.
class BLE_Manager
{
public:
    BLE_Manager();

    // Inicjalizuje BLE, tworzy usługę GATT i uruchamia advertising.
    void init();

    // Aktualizuje wartość licznika kroków przechowywaną w BLE_Manager.
    void updateStepCount(uint32_t stepCount);

    // Wysyła aktualną liczbę kroków do telefonu przez BLE Notify.
    void notifyStepCount();
};