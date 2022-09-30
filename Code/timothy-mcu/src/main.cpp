#include <Arduino.h>

#include "BluetoothServer.hpp"
#include "Message.hpp"
#include "RangeSensor.hpp"
#include "Wheel.hpp"

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// Pin definitions
constexpr uint8_t RANGE_SENSOR_LEFT_TRIGGER_PIN = 23;
constexpr uint8_t RANGE_SENSOR_LEFT_ECHO_PIN = 21;
constexpr uint8_t RANGE_SENSOR_RIGHT_TRIGGER_PIN = RANGE_SENSOR_LEFT_TRIGGER_PIN;
constexpr uint8_t RANGE_SENSOR_RIGHT_ECHO_PIN = 14;

constexpr uint8_t WHEEL_LEFT_CURRENT_PIN = 4;
constexpr uint8_t WHEEL_LEFT_CONTROL_PIN = 22;

constexpr uint8_t WHEEL_RIGHT_CONTROL_B0_PIN = 16;
constexpr uint8_t WHEEL_RIGHT_CONTROL_B1_PIN = 5;
constexpr uint8_t WHEEL_RIGHT_CONTROL_B2_PIN = 18;
constexpr uint8_t WHEEL_RIGHT_CONTROL_B3_PIN = 19;
constexpr uint8_t WHEEL_RIGHT_CURRENT_PIN = 32;

constexpr uint8_t BATTERY_PIN = 15;

// Range sensor PWM settings
constexpr uint8_t RANGE_SENSOR_LEFT_TRIGGER_CHANNEL = 0;
constexpr uint8_t RANGE_SENSOR_RIGHT_TRIGGER_CHANNEL = RANGE_SENSOR_LEFT_TRIGGER_CHANNEL;

// Right wheel control PWM settings
constexpr uint32_t WHEEL_LEFT_CONTROL_FREQ = 20000;
constexpr uint8_t WHEEL_LEFT_CONTROL_CHANNEL = 2;
constexpr uint8_t WHEEL_LEFT_CONTROL_RES = 11;

// Global objects
RangeSensor rightRangeSensor, leftRangeSensor;
DigitalWheel leftWheel;
AnalogWheel rightWheel;
BluetoothServer bluetooth;

// Global variables
static float prevRightDistanceMeasured, prevRightDistanceOutput, rightDistanceBeta, rightDistanceOneMinusBeta;

void setup()
{
    // Begin serial and bluetooth
    Serial.begin(115200);
    bluetooth.begin("Timothy ō͡≡o˞̶");

    // Begin range sensors
    rightRangeSensor.begin(RANGE_SENSOR_RIGHT_TRIGGER_PIN, RANGE_SENSOR_RIGHT_TRIGGER_CHANNEL,
                           RANGE_SENSOR_RIGHT_ECHO_PIN);
    leftRangeSensor.begin(RANGE_SENSOR_LEFT_TRIGGER_PIN, RANGE_SENSOR_LEFT_TRIGGER_CHANNEL, RANGE_SENSOR_LEFT_ECHO_PIN);

    // Begin wheels
    leftWheel.begin(WHEEL_LEFT_CONTROL_PIN, WHEEL_LEFT_CONTROL_CHANNEL, WHEEL_LEFT_CONTROL_FREQ,
                    WHEEL_LEFT_CONTROL_RES);
    rightWheel.begin(WHEEL_RIGHT_CONTROL_B0_PIN, WHEEL_RIGHT_CONTROL_B1_PIN, WHEEL_RIGHT_CONTROL_B2_PIN,
                     WHEEL_RIGHT_CONTROL_B3_PIN);

    // Setup battery reading
    pinMode(BATTERY_PIN, INPUT);

    // Setup current sensors
    pinMode(WHEEL_LEFT_CURRENT_PIN, INPUT);
    pinMode(WHEEL_RIGHT_CURRENT_PIN, INPUT);
}

float getBatteryVoltage()
{
    int adcVal = analogRead(BATTERY_PIN);
    float voltage = 5.2166e-4 * adcVal + 5.1078f;

    return voltage;
}

float getLeftWheelCurrent()
{
    int adcVal = analogRead(WHEEL_LEFT_CURRENT_PIN);
    float current = 0.08294f * adcVal + 2.529f;

    if (current < 5.0f)
        current = 0.0f;

    return current;
}

float getRightWheelCurrent()
{
    int adcVal = analogRead(WHEEL_RIGHT_CURRENT_PIN);

    float current = 0.07361f * adcVal + 75.69f;

    if (current < 80.0f)
        current = 0.0f;

    return current;
}

// Send a response over serial
uint8_t sendSerial(uint8_t *buffer, uint8_t length)
{
    return Serial.write(buffer, length);
}

// Send a response over bluetooth
int sendBluetooth(uint8_t *buffer, uint8_t length)
{
    return bluetooth.send(buffer, length);
}

// Receive a request over serial
bool receiveSerial(uint8_t *buffer)
{
    int bytesAvailable = Serial.available();
    if (bytesAvailable > 0)
    {
        auto requiredLength = Message::getRequestLength(Serial.peek());

        if (bytesAvailable >= requiredLength)
        {
            Serial.readBytes(buffer, requiredLength);
        }

        return true;
    }

    return false;
}

// Receive a request over bluetooth
bool receiveBluetooth(uint8_t *buffer)
{
    if (bluetooth.dataReceived)
    {
        bluetooth.dataReceived = false;

        size_t numBytes = bluetooth.receive(buffer, sizeof(buffer));

        return true;
    }

    return false;
}

Message createResponse(Message &request)
{
    // Act on the parsed message and create a response
    Message response;
    response.type = request.type;
    switch (request.type)
    {
    case Message::Type::GetBatteryVoltage:
        response.data.voltage = getBatteryVoltage();
        break;
    case Message::Type::GetLSRange:
        response.data.distance = leftRangeSensor.getDistance();
        break;
    case Message::Type::GetLWCurrent:
        response.data.current = getLeftWheelCurrent();
        break;
    case Message::Type::GetLWSpeed:
        response.data.speed = leftWheel.getSpeedCommand();
        break;
    case Message::Type::GetRSRange:
        response.data.distance = rightRangeSensor.getDistance();
        break;
    case Message::Type::GetRWCurrent:
        response.data.current = getRightWheelCurrent();
        break;
    case Message::Type::GetRWSpeed:
        response.data.speed = rightWheel.getSpeedCommand();
        break;
    case Message::Type::SetLWSpeed:
        leftWheel.setSpeedCommand(request.data.speed);
        response.type = Message::Type::Undefined;
        break;
    case Message::Type::SetRWSpeed:
        rightWheel.setSpeedCommand(request.data.speed);
        response.type = Message::Type::Undefined;
        break;
        return response;
    }
}

// Returns length of response. 0 if there is no response or if the request passed in was 0
uint8_t processMessage(uint8_t *buffer)
{
    // Parse the message
    Message request;
    if (!Message::parse(buffer, request))
        return 0;

    // Create the response
    auto response = createResponse(request);

    // Serialize the response
    auto length = response.serialize(buffer);

    // Return the length that should be replied with
    return length;
}

void loop()
{
    uint8_t messageBuffer[20];
    uint8_t responseLength = 0;

    // Receive serial messages
    if (receiveSerial(messageBuffer))
    {
        responseLength = processMessage(messageBuffer);
        if (responseLength > 0)
        {
            sendSerial(messageBuffer, responseLength);
        }
    }

    // Receeive bluetooth messages
    if (receiveBluetooth(messageBuffer))
    {
        responseLength = processMessage(messageBuffer);
        if (responseLength > 0)
        {
            sendBluetooth(messageBuffer, responseLength);
        }
    }

    // Set right wheel (analog) speed
    rightWheel.setSpeed(rightWheel.getSpeedCommand());

    // Set left wheel (digital) speed. First get range sensor's value
    float rightDistance = rightRangeSensor.getDistance();
    rightDistance = rightDistance < 1.0f ? rightDistance : 1.0f;
    leftWheel.setSpeed(leftWheel.getSpeedCommand() * rightDistance, getBatteryVoltage());
}