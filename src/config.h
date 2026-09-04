#pragma once
// Node configuration (outline 5.5). Credentials live in secrets.h, copied
// from secrets.h.example and git-ignored.
#include <Arduino.h>
#include <stdint.h>

// 1 = SHT40 emulated at the I2C frame level; 0 = real SHT40 on Qwiic / Wire1.
#define SHT4X_SIMULATED 1

#define DEVICE_ID "node01"

// Scheduling (outline 5.3). Every sample is published; there is no
// aggregation window on the node, averaging is a backend query.
constexpr uint32_t SAMPLE_INTERVAL_MS  = 10000;
constexpr uint32_t SAMPLE_INTERVAL_MIN_MS = 2000;   // lower bound for the serial "sample" command
constexpr uint32_t ACQUIRE_TIMEOUT_MS  = 1000;   // give up on a slow sensor after this

// Pins (outline 7)
constexpr uint8_t PIN_ONEWIRE = 2;    // DS18B20 data, 4.7 k pull-up to 5 V
constexpr uint8_t PIN_NTC     = A0;   // divider midpoint, 10 k series to 5 V

// NTC divider (outline 3.3)
constexpr int      ADC_BITS        = 14;
constexpr uint32_t ADC_MAX         = (1u << ADC_BITS) - 1;
constexpr uint8_t  NTC_SAMPLES     = 16;
constexpr float    NTC_R_SERIES    = 10000.0f;
constexpr float    NTC_R0          = 10000.0f;
constexpr float    NTC_BETA        = 3950.0f;   // verify against the part datasheet
constexpr float    NTC_OFFSET_C    = 0.0f;      // single-point calibration result

// DS18B20
constexpr uint8_t DS18B20_RESOLUTION_BITS = 12;

// Network (outline 5.3, 10)
constexpr uint32_t WIFI_RETRY_MS      = 5000;
constexpr uint32_t MQTT_RETRY_MS      = 5000;
constexpr uint32_t MQTT_KEEPALIVE_MS  = 60000;
constexpr uint32_t MQTT_CONNECT_TIMEOUT_MS = 5000;

// MQTT topics (outline 5.4): microhydros/<device_id>/{telemetry,status}
// The cmd topic is not subscribed; commands go over serial (SerialCommand).
#define MQTT_TOPIC_PREFIX "microhydros/" DEVICE_ID
#define MQTT_TOPIC_TELEMETRY MQTT_TOPIC_PREFIX "/telemetry"
#define MQTT_TOPIC_STATUS    MQTT_TOPIC_PREFIX "/status"

constexpr unsigned long SERIAL_BAUD = 115200;
