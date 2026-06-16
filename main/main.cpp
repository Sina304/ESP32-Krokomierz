#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mpu6050.h"
#include "step_detector.h"
#include "ble_manager.h"

// ESP-IDF szuka funkcji o nazwie app_main.
// Ponieważ plik jest w C++, dodajemy extern "C",
// żeby nazwa funkcji nie została zmieniona przez kompilator.
extern "C" void app_main(void)
{
    printf("Uruchamiam system krokomierza!\n");

    // Utworzenie obiektów głównych modułów systemu.
    MPU6050 sensor;
    StepDetector detector;
    BLE_Manager ble;

    // Inicjalizacja czujnika MPU6050.
    sensor.init();

    // Inicjalizacja Bluetooth Low Energy.
    ble.init();

    // Funkcje testowe używane podczas uruchamiania projektu.
    // sensor.readWhoAmI(); - test I2C
    /* Acceleration accel = sensor.readAcceleration(); - test MPU6050

        printf("X=%d  Y=%d  Z=%d\n",
            accel.x,
            accel.y,
            accel.z
        );
    */
    // sensor.readAccelerationG();
    // sensor.getAccelerationMagnitude();

    while (true)
    {
        // Odczyt aktualnego przyspieszenia i wyznaczenie
        // wartości wypadkowej (magnitude).
        float magnitude = sensor.getAccelerationMagnitude();

        // Sprawdzenie, czy algorytm wykrył krok.
        if (detector.detectStep(magnitude))
        {
            // Pobranie aktualnej liczby kroków.
            uint32_t steps = detector.getStepCount();

            // Aktualizacja wartości udostępnianej przez BLE.
            ble.updateStepCount(steps);

            // Wysłanie nowej wartości do telefonu (BLE Notify).
            ble.notifyStepCount();

            printf("KROK! Liczba krokow: %lu\n", steps);
        }

        // Odczyt czujnika wykonywany co 50 ms.
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}