#include "Message.hpp"

constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_CURRENT_ID = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID = 2;
constexpr uint8_t MESSAGE_GET_LEFT_SENSOR_RANGE_ID = 3;
constexpr uint8_t MESSAGE_GET_RIGHT_SENSOR_RANGE_ID = 4;
constexpr uint8_t MESSAGE_GET_BATTERY_VOLTAGE_ID = 5;
constexpr uint8_t MESSAGE_GET_ALL_ID = 0;
constexpr uint8_t MESSAGE_SET_LEFT_WHEEL_SPEED_ID = 100;
constexpr uint8_t MESSAGE_SET_RIGHT_WHEEL_SPEED_ID = 101;

constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_GET_ALL_LENGTH_REQUEST = 1;
constexpr uint8_t MESSAGE_SET_LEFT_WHEEL_SPEED_LENGTH_REQUEST = 2;
constexpr uint8_t MESSAGE_SET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST = 2;

constexpr uint8_t MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE = 2;
constexpr uint8_t MESSAGE_GET_ALL_LENGTH_RESPONSE =
    1 + MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE + MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_RESPONSE +
    MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE + MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE +
    MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE;

uint8_t Message::getRequestLength(uint8_t messageId)
{
    switch (messageId)
    {
    case MESSAGE_GET_ALL_ID:
        return MESSAGE_GET_ALL_LENGTH_REQUEST;

    case MESSAGE_GET_LEFT_WHEEL_CURRENT_ID:
        return MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_REQUEST;

    case MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID:
        return MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_REQUEST;

    case MESSAGE_GET_LEFT_SENSOR_RANGE_ID:
        return MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_REQUEST;

    case MESSAGE_GET_RIGHT_SENSOR_RANGE_ID:
        return MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_REQUEST;

    case MESSAGE_GET_BATTERY_VOLTAGE_ID:
        return MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_REQUEST;

    case MESSAGE_SET_LEFT_WHEEL_SPEED_ID:
        return MESSAGE_SET_LEFT_WHEEL_SPEED_LENGTH_REQUEST;

    case MESSAGE_SET_RIGHT_WHEEL_SPEED_ID:
        return MESSAGE_SET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST;
    }

    return 0;
}

// Buffer must at least be of length 1, and (if buffer[0] is a valid ID) MUST be the length of the message
bool Message::parse(uint8_t *buffer, Message &message)
{
    message.id = buffer[0];

    switch (buffer[0])
    {
    case MESSAGE_SET_LEFT_WHEEL_SPEED_ID:
        message.type = Message::Type::SetLWSpeed;
        message.data.speed = buffer[1] / 15.0f;
        return true;
    case MESSAGE_SET_RIGHT_WHEEL_SPEED_ID:
        message.type = Message::Type::SetRWSpeed;
        message.data.speed = buffer[1] / 15.0f;
        return true;
    case MESSAGE_GET_ALL_ID:
    case MESSAGE_GET_LEFT_WHEEL_CURRENT_ID:
    case MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID:
    case MESSAGE_GET_LEFT_SENSOR_RANGE_ID:
    case MESSAGE_GET_RIGHT_SENSOR_RANGE_ID:
    case MESSAGE_GET_BATTERY_VOLTAGE_ID:
        return true;
    default:
        return false;
    }

    return false;
}

// TODO
uint8_t floatToFixedPoint(float value)
{
    uint8_t integerPart = (uint8_t)value;
    return 0;
}

uint8_t Message::serialize(uint8_t *buffer)
{
    return 0;
}