#include "web_api.h"
#include "web_index_html.h"
#include "config.h"

WebApi webApi;

WebApi::WebApi() : server(HTTP_SERVER_PORT) {}

void WebApi::addCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void WebApi::sendJsonResponse(int code, const String& body) {
  addCorsHeaders();
  server.send(code, "application/json", body);
}

void WebApi::handleRoot() {
  addCorsHeaders();
  server.send_P(200, "text/html", INDEX_HTML);
}

void WebApi::handleGetState() {
  String json = "{\"color\":\"";
  json += breather.getColorString();
  json += "\"}";
  sendJsonResponse(200, json);
}

void WebApi::handlePostToggle() {
  breather.toggleColor();
  String json = "{\"color\":\"";
  json += breather.getColorString();
  json += "\"}";
  sendJsonResponse(200, json);
}

void WebApi::handlePostColor() {
  String targetColor = "";

  if (server.hasArg("color")) {
    targetColor = server.arg("color");
  } else if (server.hasArg("plain")) {
    String body = server.arg("plain");
    body.toLowerCase();
    if (body.indexOf("\"red\"") >= 0 || body.indexOf("red") >= 0) {
      targetColor = "red";
    } else if (body.indexOf("\"green\"") >= 0 || body.indexOf("green") >= 0) {
      targetColor = "green";
    }
  }

  targetColor.toLowerCase();
  targetColor.trim();

  if (targetColor == "green") {
    breather.setColor(COLOR_GREEN);
    sendJsonResponse(200, "{\"color\":\"green\"}");
  } else if (targetColor == "red") {
    breather.setColor(COLOR_RED);
    sendJsonResponse(200, "{\"color\":\"red\"}");
  } else {
    sendJsonResponse(400, "{\"error\":\"Invalid color. Expected 'green' or 'red'\"}");
  }
}

void WebApi::handleOptions() {
  addCorsHeaders();
  server.send(204);
}

void WebApi::handleNotFound() {
  addCorsHeaders();
  server.send(404, "text/plain", "404: Not Found");
}

void WebApi::setupRoutes() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/api/state", HTTP_GET, [this]() { handleGetState(); });
  server.on("/api/toggle", HTTP_POST, [this]() { handlePostToggle(); });
  server.on("/api/color", HTTP_POST, [this]() { handlePostColor(); });

  // Handle CORS preflight
  server.on("/", HTTP_OPTIONS, [this]() { handleOptions(); });
  server.on("/api/state", HTTP_OPTIONS, [this]() { handleOptions(); });
  server.on("/api/toggle", HTTP_OPTIONS, [this]() { handleOptions(); });
  server.on("/api/color", HTTP_OPTIONS, [this]() { handleOptions(); });

  server.onNotFound([this]() { handleNotFound(); });
}

void WebApi::begin() {
  setupRoutes();
  server.begin();
}

void WebApi::handleClient() {
  server.handleClient();
}
