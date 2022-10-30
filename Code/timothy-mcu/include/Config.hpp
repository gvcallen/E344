#pragma once

#include <Arduino.h>

// ADC1: 25 32 33 34 35 36 39
// ADC2: 0 2 4 12 13 14 15 26 27

// Pin definitions
static constexpr uint8_t RANGE_SENSOR_LEFT_TRIGGER_PIN = 23;
static constexpr uint8_t RANGE_SENSOR_LEFT_ECHO_PIN = 21;
static constexpr uint8_t RANGE_SENSOR_RIGHT_TRIGGER_PIN = RANGE_SENSOR_LEFT_TRIGGER_PIN;
static constexpr uint8_t RANGE_SENSOR_RIGHT_ECHO_PIN = 14;

static constexpr uint8_t WHEEL_LEFT_CURRENT_PIN = 4; // ADC2
static constexpr uint8_t WHEEL_LEFT_CONTROL_PIN = 22;

static constexpr uint8_t WHEEL_RIGHT_CONTROL_B0_PIN = 16;
static constexpr uint8_t WHEEL_RIGHT_CONTROL_B1_PIN = 5;
static constexpr uint8_t WHEEL_RIGHT_CONTROL_B2_PIN = 18;
static constexpr uint8_t WHEEL_RIGHT_CONTROL_B3_PIN = 19;
static constexpr uint8_t WHEEL_RIGHT_CURRENT_PIN = 32;

static constexpr uint8_t BATTERY_PIN = 39;

// Range sensor PWM settings
static constexpr uint8_t RANGE_SENSOR_LEFT_TRIGGER_CHANNEL = 0;
static constexpr uint8_t RANGE_SENSOR_RIGHT_TRIGGER_CHANNEL = RANGE_SENSOR_LEFT_TRIGGER_CHANNEL;

// Right wheel control PWM settings
static constexpr uint32_t WHEEL_LEFT_CONTROL_FREQ = 20000;
static constexpr uint8_t WHEEL_LEFT_CONTROL_CHANNEL = 2;
static constexpr uint8_t WHEEL_LEFT_CONTROL_RES = 11;

// Other constants
static constexpr unsigned long SERIAL_BAUD = 115200;
static constexpr const char *DEVICE_NAME = "Timothy ō͡≡o˞̶";
static constexpr const char *WIFI_PASSWORD = "wroomwroom";
static constexpr uint16_t WIFI_PORT = 80;