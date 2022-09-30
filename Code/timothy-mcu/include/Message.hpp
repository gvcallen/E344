#include "Arduino.h"

class Message
{
  public:
    enum class Type
    {
        Undefined,
        GetAll,
        GetLWCurrent,
        GetRWCurrent,
        GetLSRange,
        GetRSRange,
        GetBatteryVoltage,
        GetLWSpeed,
        GetRWSpeed,
        SetLWSpeed,
        SetRWSpeed,
    };

    union Data {
        float speed;
        float current;
        float distance;
        float voltage;
    };

  public:
    uint8_t serialize(uint8_t *buffer);

    static uint8_t getRequestLength(uint8_t messageId);
    static bool parse(uint8_t *buffer, Message &message);

  public:
    Type type;
    Data data;
    uint8_t id;
};