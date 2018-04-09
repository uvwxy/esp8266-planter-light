#include "Config.h"
#include "FastLED.h"
#include "defines.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ConfigServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#define NUM_LEDS 2
#define DATA_PIN D5

ESP8266WebServer server(80);
ConfigServer cfgServer;
Config cfg;

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  EEPROM.begin(MAX_CONFIG_SIZE);
  SPIFFS.begin();

  WiFi.hostname("planter01");

  // implement your api actions here
  // UI Button: On/Off
  server.on("/api/power/toggle", HTTP_GET, [&]() {
    server.send(200, "application/json", "{\"msg\":\"OK\"}");
  });
  // UI Button: Activate Timer
  server.on("/api/timer/activate", HTTP_GET, [&]() {
    server.send(200, "application/json", "{\"msg\":\"OK\"}");
  });
  // UI Button: Update Time From Internet
  server.on("/api/time/update", HTTP_GET, [&]() {
    server.send(200, "application/json", "{\"msg\":\"OK\"}");
  });

  // define WIFI_SSID,WIFI_PASS in defines.h, then add to .gitignore
  cfgServer.joinWifi(WIFI_SSID, WIFI_PASS, cfg, server, EEPROM);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

uint8_t c = 0;
void loop(void) {
  server.handleClient();

  // Get the config object
  if (c % 10 == 0 && cfg.getConfigVersion(EEPROM) == cfg.getId()) {
    uint32_t len = cfg.getConfigLength(EEPROM);
    char buf[len + 1];
    cfg.getConfigString(EEPROM, buf, len);
    StaticJsonBuffer<MAX_CONFIG_SIZE> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(buf);

    // Read values via API
    const char *timerSettingsStartTime = cfg.getTimerSettingsStartTime(root);
    const char *timerSettingsEndTime = cfg.getTimerSettingsEndTime(root);
    float timerSettingsHue = cfg.getTimerSettingsHue(root);
    uint32_t timerSettingsSaturation = cfg.getTimerSettingsSaturation(root);
    uint32_t timerSettingsValue = cfg.getTimerSettingsValue(root);

    uint8_t hue = (timerSettingsHue / 360.0) * 255.0;
    uint8_t sat = timerSettingsSaturation;
    uint8_t value = timerSettingsValue;

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, sat, value);
    }

    FastLED.show();

  } else if (cfg.getConfigVersion(EEPROM) != cfg.getId()) {
    Serial.println("NO CONFIG");
  }
  delay(50);
  c++;
}