#include <Arduino.h>

#include "Wheel.hpp"
#include "Error.hpp"

int DigitalWheel::begin(uint8_t pwmPin, uint8_t pwmChannel, uint32_t pwmFreq, uint8_t pwmRes)
{
    ledcSetup(pwmChannel, pwmFreq, pwmRes);
    ledcAttachPin(pwmPin, pwmChannel);
    ledcWrite(pwmChannel, 0);

    this->pwmRes = pwmRes;
    this->pwmChannel = pwmChannel;

    return ERROR_NONE;
}

int DigitalWheel::setSpeed(float normalizedSpeed)
{
    normalizedSpeed *= 0.88f;
    
    uint32_t dutyCycle = normalizedSpeed * (1 << pwmRes);
    ledcWrite(pwmChannel, dutyCycle);

    return ERROR_NONE;
}

int AnalogWheel::begin(uint8_t bit0Pin, uint8_t bit1Pin, uint8_t bit2Pin, uint8_t bit3Pin)
{
    pinMode(bit0Pin, OUTPUT);
    pinMode(bit1Pin, OUTPUT);
    pinMode(bit2Pin, OUTPUT);
    pinMode(bit3Pin, OUTPUT);

    this->bit0Pin = bit0Pin;
    this->bit1Pin = bit1Pin;
    this->bit2Pin = bit2Pin;
    this->bit3Pin = bit3Pin;

    return ERROR_NONE;
}

int AnalogWheel::setSpeed(float speed)
{
    uint8_t dacSpeed = (uint8_t)(speed * 15);

    digitalWrite(bit0Pin, dacSpeed & 0b0001 ? HIGH : LOW);
    digitalWrite(bit1Pin, dacSpeed & 0b0010 ? HIGH : LOW);
    digitalWrite(bit2Pin, dacSpeed & 0b0100 ? HIGH : LOW);
    digitalWrite(bit3Pin, dacSpeed & 0b1000 ? HIGH : LOW);

    return ERROR_NONE;
}