#include "Arduino.h"

const int ledPin = 23;
const int freq = 16;
const int ledChannel = 0;
const int resolution = 16;

#define LED 2 // pin 2 will flash for debug only

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);                    // uart setup
    pinMode(LED, OUTPUT);                    // set up pin 2 as output
    ledcSetup(ledChannel, freq, resolution); // set pwn freq and resulutionf
    ledcAttachPin(ledPin, ledChannel);       // select pwm outpit pin and channel
    ledcWrite(ledChannel, 11);               // set dutycycle to 11/65536 (10u out of 62.5ms)
}

void loop()
{
    // This is just for debug. LED flashes and write to uart.
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    Serial.println("Heart-beat");
}
