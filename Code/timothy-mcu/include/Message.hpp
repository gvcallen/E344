#pragma once

#include "Arduino.h"

constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_CURRENT_ID = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID = 2;
constexpr uint8_t MESSAGE_GET_LEFT_SENSOR_RANGE_ID = 3;
constexpr uint8_t MESSAGE_GET_RIGHT_SENSOR_RANGE_ID = 4;
constexpr uint8_t MESSAGE_GET_BATTERY_VOLTAGE_ID = 5;
constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_SPEED_ID = 6;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_SPEED_ID = 7;
constexpr uint8_t MESSAGE_GET_ALL_ID = 0;
constexpr uint8_t MESSAGE_SET_LEFT_WHEEL_SPEED_ID = 100;
constexpr uint8_t MESSAGE_SET_RIGHT_WHEEL_SPEED_ID = 101;

constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_ALL_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_SET_LEFT_WHEEL_SPEED_LENGTH_REQUEST = 2;
constexpr uint8_t MESSAGE_SET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST = 2;

constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE = 5;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_RESPONSE = 5;
constexpr uint8_t MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE = 5;
constexpr uint8_t MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE = 5;
constexpr uint8_t MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE = 5;
constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_ALL_LENGTH_RESPONSE =
    1 + MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE + MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_RESPONSE +
    MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE + MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE +
    MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE + MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_RESPONSE +
    MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_RESPONSE;

namespace tim
{

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
        struct
        {
            float lwc;
            float rwc;
            float lsr;
            float rsr;
            float bat;
            float lws;
            float rws;
        } getAll;
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

} // namespace tim