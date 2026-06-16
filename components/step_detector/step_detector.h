#pragma once

#include <stdint.h>

// Klasa odpowiedzialna za wykrywanie kroków
// na podstawie wartości magnitude obliczonej z MPU6050.
class StepDetector
{
public:

    // Konstruktor
    StepDetector();

    // Getter - zwraca aktualną liczbę wykrytych kroków.
    uint32_t getStepCount();

    // Sprawdza, czy podana wartość magnitude
    // spełnia warunki uznania jej za krok.
    bool detectStep(float magnitude);

private:

    float upper_threshold;

    float lower_threshold;

    int64_t cooldownTimeUs;

    int64_t lastStepTime;

    uint32_t stepCount;
};