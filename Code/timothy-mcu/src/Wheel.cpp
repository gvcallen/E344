#include <Arduino.h>

#include "Error.hpp"
#include "Wheel.hpp"

namespace tim
{

int Wheel::beginCurrentSensor(uint8_t pin, float a, float b)
{
    this->currentSensor.pin = pin;
    this->currentSensor.a = a;
    this->currentSensor.b = b;

    pinMode(pin, INPUT);
    return ERROR_NONE;
}

float Wheel::getCurrent()
{
    int adcVal = analogRead(this->currentSensor.pin);
    float current = this->currentSensor.a * adcVal + this->currentSensor.b;

    if (current < 80.0f)
        current = 0.0f;

    return current;
}

int DigitalWheel::begin(uint8_t pwmPin, uint8_t pwmChannel, uint32_t pwmFreq, uint8_t pwmRes)
{
    ledcSetup(pwmChannel, pwmFreq, pwmRes);
    ledcAttachPin(pwmPin, pwmChannel);
    ledcWrite(pwmChannel, 0);

    this->pwmRes = pwmRes;
    this->pwmChannel = pwmChannel;

    return ERROR_NONE;
}

int DigitalWheel::setSpeed(float speed, float batteryVoltage)
{
    // Convert the speed command to the voltage of the analog wheel at that speed
    float analogVoltage = (0.3716f * speed * 15.0f) + 0.44358f;

    // "Saturate" the command at the rail
    analogVoltage = min(analogVoltage, batteryVoltage);

    float dutyCycle = analogVoltage / batteryVoltage;

    uint32_t dutyCycleTicks = dutyCycle * ((1 << pwmRes) - 1);

    ledcWrite(pwmChannel, dutyCycleTicks);

    return ERROR_NONE;
}

int DigitalWheel::update(float rightDistance, float batteryVoltage)
{
    rightDistance = rightDistance < 1.0f ? rightDistance : 1.0f;
    this->setSpeed(this->getSpeedCommand() * rightDistance, batteryVoltage);
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

int AnalogWheel::update()
{
    this->setSpeed(this->getSpeedCommand());

    return ERROR_NONE;
}

} // namespace tim