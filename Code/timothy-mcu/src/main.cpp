#include <Arduino.h>
#include <BluetoothSerial.h>

#include "RangeSensor.hpp"
#include "Wheel.hpp"

// Pin definitions
constexpr uint8_t RANGE_SENSOR_LEFT_TRIGGER_PIN = 23;
constexpr uint8_t RANGE_SENSOR_LEFT_ECHO_PIN = 16;
constexpr uint8_t RANGE_SENSOR_RIGHT_TRIGGER_PIN = RANGE_SENSOR_LEFT_TRIGGER_PIN;
constexpr uint8_t RANGE_SENSOR_RIGHT_ECHO_PIN = 14;

constexpr uint8_t WHEEL_LEFT_CURRENT_PIN = 4;
constexpr uint8_t WHEEL_LEFT_CONTROL_PIN = 22;

constexpr uint8_t WHEEL_RIGHT_CONTROL_B0_PIN = 16;
constexpr uint8_t WHEEL_RIGHT_CONTROL_B1_PIN = 5;
constexpr uint8_t WHEEL_RIGHT_CONTROL_B2_PIN = 18;
constexpr uint8_t WHEEL_RIGHT_CONTROL_B3_PIN = 19;
constexpr uint8_t WHEEL_RIGHT_CURRENT_PIN = 32;

// Range sensor PWM settings
constexpr uint8_t RANGE_SENSOR_LEFT_TRIGGER_CHANNEL = 0;
constexpr uint8_t RANGE_SENSOR_RIGHT_TRIGGER_CHANNEL = RANGE_SENSOR_LEFT_TRIGGER_CHANNEL;

// Right wheel control PWM settings
constexpr uint32_t WHEEL_LEFT_CONTROL_FREQ = 20000;
constexpr uint8_t WHEEL_LEFT_CONTROL_CHANNEL = 2;
constexpr uint8_t WHEEL_LEFT_CONTROL_RES = 11;

// Globals
RangeSensor rightRangeSensor, leftRangeSensor;
DigitalWheel leftWheel;
AnalogWheel rightWheel;
BluetoothSerial bluetooth;

void setup()
{
    // Begin serial
    Serial.begin(115200);

    // Begin range sensors
    rightRangeSensor.begin(RANGE_SENSOR_RIGHT_TRIGGER_PIN, RANGE_SENSOR_RIGHT_TRIGGER_CHANNEL, RANGE_SENSOR_RIGHT_ECHO_PIN);
    leftRangeSensor.begin(RANGE_SENSOR_LEFT_TRIGGER_PIN, RANGE_SENSOR_LEFT_TRIGGER_CHANNEL, RANGE_SENSOR_LEFT_ECHO_PIN);

    // Begin wheels
    leftWheel.begin(WHEEL_LEFT_CONTROL_PIN, WHEEL_LEFT_CONTROL_CHANNEL, WHEEL_LEFT_CONTROL_FREQ, WHEEL_LEFT_CONTROL_RES);
    rightWheel.begin(WHEEL_RIGHT_CONTROL_B0_PIN, WHEEL_RIGHT_CONTROL_B1_PIN, WHEEL_RIGHT_CONTROL_B2_PIN, WHEEL_RIGHT_CONTROL_B3_PIN);

    // Begin bluetooth
    bluetooth.begin("Timothy ō͡≡o˞̶");
}

void loop()
{    
    float distance = rightRangeSensor.getDistance();
    float leftWheelSpeed = 1.0f;

    float controlCommand = leftWheelSpeed * distance;
    // leftWheel.setSpeed(controlCommand);
    // rightWheel.setSpeed(1.0f);

    leftWheel.setSpeed(0.0f);
    rightWheel.setSpeed(0.0f);
}
