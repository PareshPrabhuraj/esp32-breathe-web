#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "config.h"
#include "led_breather.h"
#include "web_api.h"

void handleSerialCommands() {
  while (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    if (cmd == "TOGGLE") {
      breather.toggleColor();
      Serial.print("STATE:");
      Serial.println(breather.getColorString());
    } else if (cmd == "GREEN") {
      breather.setColor(COLOR_GREEN);
      Serial.print("STATE:");
      Serial.println(breather.getColorString());
    } else if (cmd == "RED") {
      breather.setColor(COLOR_RED);
      Serial.print("STATE:");
      Serial.println(breather.getColorString());
    } else if (cmd == "STATE") {
      Serial.print("STATE:");
      Serial.println(breather.getColorString());
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("==================================================");
  Serial.println("      ESP32-S3 LED BREATHER WEB CONTROLLER        ");
  Serial.println("==================================================");

  // 1. Initialize Non-blocking LED Breather hardware
  breather.begin();
  Serial.println("[LED] Hardware PWM breather initialized (Default: GREEN)");

  // 2. Setup Wi-Fi Modes
#if HAS_WIFI_SECRETS
  Serial.println("[WiFi] Initializing AP + Station mode...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
#else
  Serial.println("[WiFi] Initializing Standalone SoftAP mode...");
  WiFi.mode(WIFI_AP);
#endif

  // Configure SoftAP network (Open AP)
  bool apSuccess = WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, 0, AP_MAX_CONN);
  if (apSuccess) {
    Serial.print("[WiFi] Access Point Active. SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi] AP Web Page: http://");
    Serial.print(WiFi.softAPIP());
    Serial.println("/");
  } else {
    Serial.println("[WiFi] Error starting Access Point!");
  }

#if HAS_WIFI_SECRETS
  // Optional Station connection check (up to 5 seconds timeout)
  Serial.print("[WiFi] Connecting to router SSID: ");
  Serial.println(WIFI_STA_SSID);
  unsigned long staStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - staStart < 5000) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WiFi] Connected to local router! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WiFi] Router connection timed out. SoftAP remains fully active.");
  }
#endif

  // 3. Initialize mDNS responder (http://esp32.local)
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", HTTP_SERVER_PORT);
    Serial.print("[mDNS] Responder started at: http://");
    Serial.print(MDNS_HOSTNAME);
    Serial.println(".local/");
  }

  // 4. Start HTTP Web API
  webApi.begin();
  Serial.println("[HTTP] Web server listening on port 80.");
  Serial.println("==================================================");
  Serial.println("System READY. Breather running smoothly.");
}

void loop() {
  breather.update();
  webApi.handleClient();
  handleSerialCommands();
}
