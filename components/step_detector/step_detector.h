#pragma once

#include <stdint.h>

class StepDetector
{
public:
    StepDetector();
    
    uint32_t getStepCount();

    bool detectStep(float magnitude);

private:
    float upper_threshold;
    float lower_threshold;

    int64_t cooldownTimeUs;
    int64_t lastStepTime;

    uint32_t stepCount;
};