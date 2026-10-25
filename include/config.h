#pragma once
#include <Arduino.h>

// ==============================================================================
// 1. HARDWARE PIN DEFINITIONS & LED TOPOLOGY
// ==============================================================================
// ESP32-S3 Pin Selection Rationale:
// - GPIO 4: ADC1_CH3, safe general-purpose high-speed output (Red channel)
// - GPIO 5: ADC1_CH4, safe general-purpose high-speed output (Green channel)
// - Both pins avoid SPI flash (GPIO 26-32/33-37) and strapping pins (GPIO 0, 3, 45, 46).
#define PIN_LED_RED          4
#define PIN_LED_GREEN        5

// Topology: 0 = Common Cathode (longest pin to GND), 1 = Common Anode (longest pin to 3.3V)
#define COMMON_ANODE         0

// ==============================================================================
// 2. BREATHING DYNAMICS & PERCEPTUAL PHOTOMETRY
// ==============================================================================
// Full respiratory breathing period (sine wave cycle) in milliseconds
#define BREATHE_PERIOD_MS    4000UL

// Maximum duty cycle cap (60% of 255 = ~153) to prevent eye strain
#define MAX_BRIGHTNESS_CAP   153

// Stevens / CIE 1931 perceptual gamma power for uniform human eye response
#define GAMMA_EXPONENT       2.2f

// LEDC PWM Hardware Properties
#define LEDC_FREQ_HZ         5000
#define LEDC_RESOLUTION_BITS 8
#define LEDC_CHANNEL_RED     0
#define LEDC_CHANNEL_GREEN   1

// ==============================================================================
// 3. WI-FI NETWORK CONFIGURATION
// ==============================================================================
// Default SoftAP: Broadcasts an open Wi-Fi network
#define AP_SSID              "ESP32-Breathe"
#define AP_PASS              "" // Open network
#define AP_CHANNEL           1
#define AP_MAX_CONN          4

// mDNS local URL: http://esp32.local
#define MDNS_HOSTNAME        "esp32"
#define HTTP_SERVER_PORT     80

// Optional Station Mode credentials from git-ignored secrets.h
#if __has_include("secrets.h")
  #include "secrets.h"
  #define HAS_WIFI_SECRETS 1
#else
  #define HAS_WIFI_SECRETS 0
#endif
