#include "Arduino.h"

#define TRIGGER_PIN 23
#define CONTROL_PIN 22
#define BIT_0_PIN 35
#define BIT_1_PIN 34
#define BIT_2_PIN 39
#define BIT_3_PIN 36
#define ECHO_PIN 14

const int triggerFreq = 16;
const int triggerChannel = 0;
const int triggerRes = 8;
const int controlFreq = 20000;
const int controlChannel = 2;
const int controlRes = 11;

unsigned long echoReceiveTime = 0;

void echoChanged()
{
    static unsigned long echoHighTime;
    
    uint8_t echoState = digitalRead(ECHO_PIN);
    if (echoState)
    {
        echoHighTime = micros();
    }
    else
    {
        echoReceiveTime = micros() - echoHighTime;
    }
}

void setMotorSpeed(float speedNormalized)
{
    speedNormalized *= 0.88f;
    
    uint32_t duty = speedNormalized * (1 << controlRes);
    ledcWrite(controlChannel, duty);
}

void setup()
{
    Serial.begin(115200);

    // Setup digital input pins
    pinMode(BIT_0_PIN, INPUT);
    pinMode(BIT_1_PIN, INPUT);
    pinMode(BIT_2_PIN, INPUT);
    pinMode(BIT_3_PIN, INPUT);

    // Setup trigger
    ledcSetup(triggerChannel, triggerFreq, triggerRes);
    ledcAttachPin(TRIGGER_PIN, triggerChannel);
    ledcWrite(triggerChannel, 11);               // set dutycycle to 11/65536 (10u out of 62.5ms)

    // Setup control    
    ledcSetup(controlChannel, controlFreq, controlRes);
    ledcAttachPin(CONTROL_PIN, controlChannel);
    ledcWrite(controlChannel, 8);

    // Setup echo
    pinMode(ECHO_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoChanged, CHANGE);
}

void loop()
{
    // Read in 4-bit digital value and store as 4-bit number in speedCommand
    uint8_t bit0 = digitalRead(BIT_0_PIN);
    uint8_t bit1 = digitalRead(BIT_1_PIN);
    uint8_t bit2 = digitalRead(BIT_2_PIN);
    uint8_t bit3 = digitalRead(BIT_3_PIN);
    uint8_t speedCommand = (bit0) | (bit1 << 1) | (bit2 << 2) | (bit3 << 3);
    
    float distance = (343.0f * echoReceiveTime * 1e-6) / 2;
    
    if (distance > 1) distance = 1;
    
    float controlCommand = ((float)speedCommand / 15.0f) * distance;
    setMotorSpeed(controlCommand);

}
