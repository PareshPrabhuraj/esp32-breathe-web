#include "led_breather.h"
#include <esp_arduino_version.h>
#include <math.h>

LedBreather breather;

// Pure mathematical function: maps phase [0.0, 1.0] to gamma-corrected duty cycle [0, maxCap]
uint8_t calculateBreathingDuty(float phaseFraction, uint8_t maxCap, float gamma) {
  if (phaseFraction < 0.0f) phaseFraction = 0.0f;
  if (phaseFraction > 1.0f) phaseFraction = 1.0f;

  // Sine breathing curve: 0 at phase 0, peaks at phase 0.5, returns to 0 at phase 1.0
  float rawSine = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * phaseFraction));
  
  // Stevens / CIE 1931 perceptual correction for uniform human eye response
  float corrected = powf(rawSine, gamma);
  
  float scaled = corrected * (float)maxCap;
  if (scaled > (float)maxCap) scaled = (float)maxCap;
  if (scaled < 0.0f) scaled = 0.0f;
  
  return (uint8_t)(scaled + 0.5f);
}

LedBreather::LedBreather()
  : currentColor(COLOR_GREEN),
    cycleStartTime(0),
    currentDuty(0) {}

void LedBreather::begin() {
  cycleStartTime = millis();

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcAttach(PIN_LED_RED, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
  ledcAttach(PIN_LED_GREEN, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
#else
  ledcSetup(LEDC_CHANNEL_RED, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
  ledcAttachPin(PIN_LED_RED, LEDC_CHANNEL_RED);

  ledcSetup(LEDC_CHANNEL_GREEN, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
  ledcAttachPin(PIN_LED_GREEN, LEDC_CHANNEL_GREEN);
#endif

  // Initial update
  update();
}

void LedBreather::writeHardwarePWM(BreatheColor color, uint8_t duty) {
#if COMMON_ANODE
  uint8_t activeDuty = 255 - duty;
  uint8_t inactiveDuty = 255; // Off for common anode
#else
  uint8_t activeDuty = duty;
  uint8_t inactiveDuty = 0;   // Off for common cathode
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  if (color == COLOR_GREEN) {
    ledcWrite(PIN_LED_GREEN, activeDuty);
    ledcWrite(PIN_LED_RED, inactiveDuty);
  } else {
    ledcWrite(PIN_LED_RED, activeDuty);
    ledcWrite(PIN_LED_GREEN, inactiveDuty);
  }
#else
  if (color == COLOR_GREEN) {
    ledcWrite(LEDC_CHANNEL_GREEN, activeDuty);
    ledcWrite(LEDC_CHANNEL_RED, inactiveDuty);
  } else {
    ledcWrite(LEDC_CHANNEL_RED, activeDuty);
    ledcWrite(LEDC_CHANNEL_GREEN, inactiveDuty);
  }
#endif
}

void LedBreather::update() {
  unsigned long now = millis();
  unsigned long elapsed = (now - cycleStartTime) % BREATHE_PERIOD_MS;
  float phaseFraction = (float)elapsed / (float)BREATHE_PERIOD_MS;

  currentDuty = calculateBreathingDuty(phaseFraction, MAX_BRIGHTNESS_CAP, GAMMA_EXPONENT);
  writeHardwarePWM(currentColor, currentDuty);
}

void LedBreather::setColor(BreatheColor newColor) {
  if (currentColor != newColor) {
    currentColor = newColor;
    // Continuous breathing phase invariant: DO NOT reset cycleStartTime!
    // The new color immediately assumes the exact phase duty cycle of the active cycle.
    writeHardwarePWM(currentColor, currentDuty);
  }
}

void LedBreather::toggleColor() {
  setColor((currentColor == COLOR_GREEN) ? COLOR_RED : COLOR_GREEN);
}
