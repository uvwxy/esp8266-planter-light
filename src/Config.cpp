#include "Config.h"
#include <Arduino.h>

uint8_t Config::getId() { return 1; };

const char* Config::getTimerSettingsStartTime(JsonObject& root) {
  return root["Timer Settings"]["Start Time"];
}

const char* Config::getTimerSettingsEndTime(JsonObject& root) {
  return root["Timer Settings"]["End Time"];
}

float Config::getTimerSettingsHue(JsonObject& root) {
  return root["Timer Settings"]["Hue"];
}

uint32_t Config::getTimerSettingsSaturation(JsonObject& root) {
  return root["Timer Settings"]["Saturation"];
}

uint32_t Config::getTimerSettingsValue(JsonObject& root) {
  return root["Timer Settings"]["Value"];
}