#include "step_detector.h"
#include "esp_timer.h"


StepDetector::StepDetector()
{
    // Górny próg wykrywania kroku.
    upper_threshold = 1.2f;

    // Dolny próg wykrywania kroku.
    lower_threshold = 0.8f;

    // Minimalny odstęp pomiędzy kolejnymi krokami.
    // Jednostka: mikrosekundy.
    cooldownTimeUs = 450000;

    // Czas ostatnio wykrytego kroku.
    lastStepTime = 0;

    // Aktualna liczba kroków.
    stepCount = 0;
}

bool StepDetector::detectStep(float magnitude)
{
    // Aktualny czas od uruchomienia ESP32.
    int64_t currentTime = esp_timer_get_time();

    // Sprawdzenie czy sygnał przekroczył jeden z progów.
    if (magnitude > upper_threshold || magnitude < lower_threshold)
    {
        // Sprawdzenie czy minął wymagany odstęp czasu
        // od poprzedniego kroku.
        if ((currentTime - lastStepTime) > cooldownTimeUs)
        {
            // Zapamiętanie czasu nowego kroku.
            lastStepTime = currentTime;

            // Zwiększenie licznika kroków.
            stepCount++;

            return true;
        }
    }

    return false;
}

uint32_t StepDetector::getStepCount()
{
    return stepCount;
}