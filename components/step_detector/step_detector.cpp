#include "step_detector.h"
#include "esp_timer.h"

StepDetector::StepDetector()
{
    upper_threshold = 1.2f;
    lower_threshold = 0.8f;

    cooldownTimeUs = 450000;

    lastStepTime = 0;
    stepCount = 0;
}

bool StepDetector::detectStep(float magnitude)
{
    int64_t currentTime = esp_timer_get_time();

    if (magnitude > upper_threshold || magnitude < lower_threshold)
    {
        if ((currentTime - lastStepTime) > cooldownTimeUs)
        {
            lastStepTime = currentTime;
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