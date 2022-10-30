#include "Message.hpp"

namespace tim
{

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

    case MESSAGE_GET_LEFT_WHEEL_SPEED_ID:
        return MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_REQUEST;

    case MESSAGE_GET_RIGHT_WHEEL_SPEED_ID:
        return MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST;

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
        break;
    case MESSAGE_SET_RIGHT_WHEEL_SPEED_ID:
        message.type = Message::Type::SetRWSpeed;
        message.data.speed = buffer[1] / 15.0f;
        break;
    case MESSAGE_GET_ALL_ID:
        message.type = Message::Type::GetAll;
        break;
    case MESSAGE_GET_LEFT_WHEEL_CURRENT_ID:
        message.type = Message::Type::GetLWCurrent;
        break;
    case MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID:
        message.type = Message::Type::GetRWCurrent;
        break;
    case MESSAGE_GET_LEFT_WHEEL_SPEED_ID:
        message.type = Message::Type::GetLWSpeed;
        break;
    case MESSAGE_GET_RIGHT_WHEEL_SPEED_ID:
        message.type = Message::Type::GetRWSpeed;
        break;
    case MESSAGE_GET_LEFT_SENSOR_RANGE_ID:
        message.type = Message::Type::GetLSRange;
        break;
    case MESSAGE_GET_RIGHT_SENSOR_RANGE_ID:
        message.type = Message::Type::GetRSRange;
        break;
    case MESSAGE_GET_BATTERY_VOLTAGE_ID:
        message.type = Message::Type::GetBatteryVoltage;
        break;
    default:
        return false;
    }

    return true;
}

uint8_t Message::serialize(uint8_t *buffer)
{
    switch (this->type)
    {
    case Type::Undefined:
    case Type::SetLWSpeed:
    case Type::SetRWSpeed:
        return 0;
    case Type::GetAll:
        return 0;
    case Type::GetLWCurrent:
    case Type::GetRWCurrent:
    case Type::GetLSRange:
    case Type::GetRSRange:
    case Type::GetBatteryVoltage:
        buffer[0] = this->id;
        memcpy(&buffer[1], &this->data, sizeof(float));
        return 1 + sizeof(float);

    case Type::GetLWSpeed:
    case Type::GetRWSpeed:
        buffer[0] = this->id;
        buffer[1] = (uint8_t)(this->data.speed * 15.0f);
        return 2;
    }

    return 0;
}

} // namespace tim