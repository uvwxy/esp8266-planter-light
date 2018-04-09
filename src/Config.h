#ifndef PlanterConfig_H
#define PlanterConfig_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ConfigServerConfig.h>
class Config : public ConfigServerConfig {
    public:
      uint8_t getId();
      const char* getTimerSettingsStartTime(JsonObject& root);
      const char* getTimerSettingsEndTime(JsonObject& root);
      float getTimerSettingsHue(JsonObject& root);
      uint32_t getTimerSettingsSaturation(JsonObject& root);
      uint32_t getTimerSettingsValue(JsonObject& root);
    };
#endif