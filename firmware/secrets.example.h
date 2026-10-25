#pragma once

// ==============================================================================
// OPTIONAL WI-FI ROUTER CREDENTIALS
// ==============================================================================
// The ESP32 defaults to running its own Access Point:
//   SSID: ESP32-Breathe (Open, IP: 192.168.4.1)
//
// If you also want the ESP32 to connect to your local Wi-Fi router:
// 1. Copy this file to "include/secrets.h":
//    cp include/secrets.example.h include/secrets.h
// 2. Fill in your SSID and Password below.
// 3. The ESP32 will connect to your router and serve mDNS at http://esp32.local/
//
// NOTE: "secrets.h" is git-ignored and will NEVER be committed to GitHub.

#define WIFI_STA_SSID     "Your_WiFi_SSID"
#define WIFI_STA_PASSWORD "Your_WiFi_Password"
