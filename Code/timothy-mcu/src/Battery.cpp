#include "Battery.hpp"

namespace tim
{

void Battery::begin(uint8_t pin)
{
    pinMode(pin, INPUT);
    this->pin = pin;
}

float Battery::getVoltage()
{
    int adcVal = analogRead(this->pin);
    return 5.2166e-4 * adcVal + 5.1078f;
}

} // namespace tim