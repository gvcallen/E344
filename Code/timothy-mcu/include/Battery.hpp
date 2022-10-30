#pragma once
#include <Arduino.h>

namespace tim
{

class Battery
{
  public:
    void begin(uint8_t pin);
    float getVoltage();

  private:
    uint8_t pin;
};

} // namespace tim