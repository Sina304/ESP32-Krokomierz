#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mpu6050.h"
#include "step_detector.h"

extern "C" void app_main(void)
{
    printf("Uruchamiam system krokomierza!\n");

    MPU6050 sensor;
    StepDetector detector;

    sensor.init();
    //sensor.readWhoAmI();
    // sensor.readAcceleration();
    // sensor.readAccelerationG();
    // sensor.getAccelerationMagnitude();

    while (true)
    {
        float magnitude = sensor.getAccelerationMagnitude();

        if (detector.detectStep(magnitude))
        {
            printf("KROK! Liczba krokow: %lu\n",
                   detector.getStepCount());
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}