#pragma once
#include <Arduino.h>
#include "config.h"

enum BreatheColor {
  COLOR_GREEN = 0,
  COLOR_RED   = 1
};

// Pure mathematical function for gamma-corrected respiratory lighting
uint8_t calculateBreathingDuty(float phaseFraction, uint8_t maxCap, float gamma);

class LedBreather {
public:
  LedBreather();
  void begin();
  void update(); // Non-blocking millis-driven update loop
  
  void setColor(BreatheColor newColor);
  void toggleColor();
  BreatheColor getColor() const { return currentColor; }
  const char* getColorString() const { return (currentColor == COLOR_GREEN) ? "green" : "red"; }
  uint8_t getCurrentDuty() const { return currentDuty; }

private:
  void writeHardwarePWM(BreatheColor color, uint8_t duty);

  BreatheColor currentColor;
  unsigned long cycleStartTime;
  uint8_t currentDuty;
};

extern LedBreather breather;
