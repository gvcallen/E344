#pragma once
#include <Arduino.h>

namespace tim
{

constexpr uint32_t RANGE_SENSOR_TRIGGER_FREQ = 16;
constexpr uint8_t RANGE_SENSOR_TRIGGER_RES = 8;

class RangeSensor
{
  public:
    RangeSensor();
    int begin(uint8_t triggerPin, uint8_t triggerChannel, uint8_t echoPin);
    float getDistance();

  private:
    void echoChanged();

  private:
    unsigned long echoChangeHighTime;
    uint64_t echoLength;
    uint8_t echoPin;

  private:
    static RangeSensor *interrupts[2];
    static void echoChanged0()
    {
        interrupts[0]->echoChanged();
    }
    static void echoChanged1()
    {
        interrupts[1]->echoChanged();
    }
};

} // namespace tim