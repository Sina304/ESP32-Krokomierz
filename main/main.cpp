#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// extern "C" wyłącza modyfikację nazwy funkcji przez kompilator C++ (tzw. name mangling).
// Dzięki temu wewnętrzny kod startowy ESP-IDF (napisany w C) bez problemu znajdzie i uruchomi tę funkcję.
extern "C" void app_main(void)
{
    printf("Uruchamiam system krokomierza z FreeRTOS!\n");
    
    while(true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);      //usypianie wątku zamiast delay(), bo RTOS
    }
}