#pragma once
#include <Arduino.h>
#include "CurrentSensor.hpp"

class Wheel
{
public:
    Wheel() {}
    CurrentSensor& getCurrentSensor() { return currentSensor; }

private:
    CurrentSensor currentSensor;
};

class DigitalWheel : public Wheel
{
public:
    int begin(uint8_t pwmPin, uint8_t pwmChannel, uint32_t pwmFreq, uint8_t pwmRes);
    int setSpeed(float speed);

private:
    uint8_t pwmRes;
    uint8_t pwmChannel;
};

class AnalogWheel : public Wheel
{
public:
    int begin(uint8_t bit0Pin, uint8_t bit1Pin, uint8_t bit2Pin, uint8_t bit3Pin);
    int setSpeed(float speed);
private:
    uint8_t bit0Pin, bit1Pin, bit2Pin, bit3Pin;
};