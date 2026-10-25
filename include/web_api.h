#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "led_breather.h"

class WebApi {
public:
  WebApi();
  void begin();
  void handleClient();

private:
  void setupRoutes();
  void handleRoot();
  void handleGetState();
  void handlePostToggle();
  void handlePostColor();
  void handleOptions();
  void handleNotFound();

  void sendJsonResponse(int code, const String& body);
  void addCorsHeaders();

  WebServer server;
};

extern WebApi webApi;
