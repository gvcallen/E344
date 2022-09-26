#include "CurrentSensor.hpp"
#include "Error.hpp"


int CurrentSensor::begin(uint8_t pin)
{
    return ERROR_NONE;
}

int CurrentSensor::calibrate(uint8_t* calibParams, uint8_t numRanges)
{
    return ERROR_NONE;
}

float CurrentSensor::getValue()
{
    return 0.0f;
}