#pragma once

#include <Arduino.h>

class CurrentSensor
{
public:
    int begin(uint8_t pin);
    int calibrate(uint8_t* calibParams, uint8_t numRanges);
    float getValue();
};