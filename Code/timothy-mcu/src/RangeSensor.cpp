#include <Arduino.h>

#include "Error.hpp"
#include "RangeSensor.hpp"

namespace tim
{

RangeSensor *RangeSensor::interrupts[2] = {};

RangeSensor::RangeSensor()
{
    echoChangeHighTime = echoLength = 0;
}

int RangeSensor::begin(uint8_t triggerPin, uint8_t triggerChannel, uint8_t echoPin)
{
    this->echoPin = echoPin;

    // Setup trigger
    if (!ledcSetup(triggerChannel, RANGE_SENSOR_TRIGGER_FREQ, RANGE_SENSOR_TRIGGER_RES))
        return false;

    ledcDetachPin(triggerPin);
    ledcAttachPin(triggerPin, triggerChannel);
    ledcWrite(triggerChannel, 11); // set duty cycle to 11/65536 (10u out of 62.5ms)

    // Setup echo
    pinMode(echoPin, INPUT);

    int available = -1;
    for (int i = 0; i < sizeof(interrupts); i++)
    {
        if (!interrupts[i])
        {
            available = i;
            break;
        }
    }

    if (available == -1)
        return ERROR_UNAVAILABLE;

    interrupts[available] = this;

    void (*callback)() = nullptr;
    switch (available)
    {
    case 0:
        callback = RangeSensor::echoChanged0;
        break;
    case 1:
        callback = RangeSensor::echoChanged1;
        break;
    }

    attachInterrupt(digitalPinToInterrupt(echoPin), callback, CHANGE);

    return ERROR_NONE;
}

void RangeSensor::echoChanged()
{
    uint8_t echoState = digitalRead(echoPin);
    if (echoState)
    {
        echoChangeHighTime = micros();
    }
    else
    {
        echoLength = micros() - echoChangeHighTime;
    }
}

float RangeSensor::getDistance()
{
    return (343.0f * echoLength * 1e-6) / 2;
}

} // namespace tim