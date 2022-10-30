#include <Arduino.h>

#include "Battery.hpp"
#include "BluetoothServer.hpp"
#include "Config.hpp"
#include "Message.hpp"
#include "RangeSensor.hpp"
#include "Wheel.hpp"
#include "WiFiServer.hpp"

// Globals
tim::RangeSensor rightRangeSensor, leftRangeSensor;
tim::DigitalWheel leftWheel;
tim::AnalogWheel rightWheel;
tim::BluetoothServer bluetooth;
tim::WiFiServer wiFi;
tim::Battery battery;

void setup()
{
    // Begin serial and bluetooth
    Serial.begin(SERIAL_BAUD);

    // Begin range sensors
    rightRangeSensor.begin(RANGE_SENSOR_RIGHT_TRIGGER_PIN, RANGE_SENSOR_RIGHT_TRIGGER_CHANNEL,
                           RANGE_SENSOR_RIGHT_ECHO_PIN);
    leftRangeSensor.begin(RANGE_SENSOR_LEFT_TRIGGER_PIN, RANGE_SENSOR_LEFT_TRIGGER_CHANNEL, RANGE_SENSOR_LEFT_ECHO_PIN);

    // Begin left wheel
    leftWheel.begin(WHEEL_LEFT_CONTROL_PIN, WHEEL_LEFT_CONTROL_CHANNEL, WHEEL_LEFT_CONTROL_FREQ,
                    WHEEL_LEFT_CONTROL_RES);
    leftWheel.beginCurrentSensor(WHEEL_LEFT_CURRENT_PIN, 0.08294f, 2.529f);

    // Begin right wheel
    rightWheel.begin(WHEEL_RIGHT_CONTROL_B0_PIN, WHEEL_RIGHT_CONTROL_B1_PIN, WHEEL_RIGHT_CONTROL_B2_PIN,
                     WHEEL_RIGHT_CONTROL_B3_PIN);
    rightWheel.beginCurrentSensor(WHEEL_RIGHT_CURRENT_PIN, 0.07361f, 75.69f);

    // Setup battery reading
    battery.begin(BATTERY_PIN);

    // Delay for debugging
    delay(1000);
}

tim::Message respond(tim::Message &request)
{
    using tim::Message;

    // Act on the parsed message and create a response
    Message response;
    response.type = request.type;
    response.id = request.id;

    switch (request.type)
    {
    case Message::Type::GetBatteryVoltage:
        response.data.voltage = battery.getVoltage();
        break;
    case Message::Type::GetLSRange:
        response.data.distance = leftRangeSensor.getDistance();
        break;
    case Message::Type::GetLWCurrent:
        response.data.current = leftWheel.getCurrent();
        break;
    case Message::Type::GetLWSpeed:
        response.data.speed = leftWheel.getSpeedCommand();
        break;
    case Message::Type::GetRSRange:
        response.data.distance = rightRangeSensor.getDistance();
        break;
    case Message::Type::GetRWCurrent:
        response.data.current = rightWheel.getCurrent();
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
    case Message::Type::GetAll:
        auto &all = response.data.getAll;
        // all.bat = battery.getVoltage();
        all.bat = 6.5f;

        all.lsr = leftRangeSensor.getDistance();

        // all.lwc = leftWheel.getCurrent();
        all.lwc = 1.0f;

        all.lws = leftWheel.getSpeedCommand();
        all.rsr = rightRangeSensor.getDistance();

        all.rwc = rightWheel.getCurrent();
        // all.rwc = 0.5f;

        all.rws = rightWheel.getSpeedCommand();
        break;
    }
    return response;
}

// Returns length of response. 0 is returned in the case of no response.
uint8_t processSerialMessage(uint8_t *buffer)
{
    using tim::Message;

    // Parse the message
    Message request;
    if (!Message::parse(buffer, request))
        return 0;

    // Create the response
    auto response = respond(request);

    // Serialize the response
    auto length = response.serialize(buffer);

    // Return the length that should be replied with
    return length;
}

// Receive a request over serial
bool receiveSerial(uint8_t *buffer)
{
    int bytesAvailable = Serial.available();
    if (bytesAvailable > 0)
    {
        auto requiredLength = tim::Message::getRequestLength(Serial.peek());

        if (bytesAvailable >= requiredLength)
        {
            Serial.readBytes(buffer, requiredLength);
        }

        return true;
    }

    return false;
}

void updateSerial()
{
    uint8_t messageBuffer[20];

    if (receiveSerial(messageBuffer))
    {
        uint8_t responseLength = processSerialMessage(messageBuffer);
        if (responseLength > 0)
            Serial.write(messageBuffer, responseLength);
    }
}

void updateBluetooth()
{
    uint8_t messageBuffer[20];

    if (bluetooth.tryReceive(messageBuffer, sizeof(messageBuffer)))
    {
        auto responseLength = processSerialMessage(messageBuffer);
        if (responseLength > 0)
            bluetooth.send(messageBuffer, responseLength);
    }
}

void updateWiFi()
{
    using tim::Message;

    wiFi.update();

    auto requests = wiFi.getRequests();

    for (auto request : requests)
    {
        Message response = respond(request);
        wiFi.sendResponse(response);
    }
}

void updateWireless()
{
    if (!bluetooth.connected() && !wiFi.connected())
    {
        if (!wiFi.running())
        {
            Serial.println("Starting WiFi...");
            wiFi.begin(DEVICE_NAME, WIFI_PASSWORD, WIFI_PORT);
        }

        if (!bluetooth.running())
        {
            Serial.println("Starting Bluetooth...");
            bluetooth.begin(DEVICE_NAME);
        }
    }

    if (bluetooth.connected() && wiFi.running())
    {
        Serial.println("Ending WiFi...");
        wiFi.end();
    }

    if (wiFi.connected() && bluetooth.running())
    {
        Serial.println("Ending Bluetooth...");
        bluetooth.end();
    }

    if (bluetooth.running())
        updateBluetooth();

    if (wiFi.running())
        updateWiFi();
}

void updateWheels()
{
    rightWheel.update();
    leftWheel.update(rightRangeSensor.getDistance(), battery.getVoltage());
}

void loop()
{
    updateSerial();
    updateWireless();
    // if (!wiFi.running())
    // wiFi.begin(DEVICE_NAME, WIFI_PASSWORD, WIFI_PORT);
    // updateWiFi();
    updateWheels();
    // Serial.println(leftRangeSensor.getDistance() * 100);
}