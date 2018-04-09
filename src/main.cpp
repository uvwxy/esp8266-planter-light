#include "Config.h"
#include "FastLED.h"
#include "defines.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ConfigServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <TimeLib.h>
#include <WebTime.h>

#define NUM_LEDS 2
#define DATA_PIN D5

ESP8266WebServer server(80);
ConfigServer cfgServer;
Config cfg;

CRGB leds[NUM_LEDS];
bool timerEnabled = false;
boolean powerEnabled = true;

static WiFiClient client;

// https://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(MAX_CONFIG_SIZE);
  SPIFFS.begin();

  WiFi.hostname("planter01");

  // implement your api actions here
  // UI Button: On/Off
  server.on("/api/power/toggle", HTTP_GET, [&]() {
    powerEnabled = !powerEnabled;
    if (powerEnabled) {
      timerEnabled = false;
    }
    String resp = "{\"msg\":\"" +
                  (powerEnabled ? String("enabled") : String("disabled")) +
                  "\"}";
    server.send(200, "application/json", resp);
  });
  // UI Button: Activate Timer
  server.on("/api/timer/toggle", HTTP_GET, [&]() {
    timerEnabled = !timerEnabled;
    if (timerEnabled) {
      powerEnabled = false;
    }
    String resp = "{\"msg\":\"" +
                  (timerEnabled ? String("enabled") : String("disabled")) +
                  "\"}";
    server.send(200, "application/json", resp);
  });

  // UI Button: Update Time From Internet
  server.on("/api/time/update", HTTP_GET, [&]() {
    unsigned long unixTime = webUnixTime(client);
    unixTime += 2 * (3600); // ugly fix for timezone diff
    setTime(unixTime);
    server.send(200, "application/json",
                "{\"msg\":\"" + String(unixTime) + "\"}");
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

    if (timeStatus() != timeNotSet) {
      Serial.println("TIME: ");
      Serial.print(hour());
      Serial.print(":");
      Serial.print(minute());
      Serial.print(":");
      Serial.print(second());
    }

    bool timerActive = false;
    if (timerEnabled && timeStatus() != timeNotSet) {
      Serial.println("Test:");
      // turn light on
      int h = getValue(timerSettingsStartTime, ':', 0).toInt();
      int m = getValue(timerSettingsStartTime, ':', 1).toInt();
      Serial.println(h);
      Serial.println(m);
      if (hour() >= h && minute() >= m) {
        timerActive = true;
        Serial.println("-> on");
      }
      // turn it off
      h = getValue(timerSettingsEndTime, ':', 0).toInt();
      m = getValue(timerSettingsEndTime, ':', 1).toInt();
      Serial.println(h);
      Serial.println(m);
      if (hour() >= h && minute() >= m) {
        timerActive = false;
        Serial.println("-> off");
      }
    }

    for (int i = 0; i < NUM_LEDS; i++) {
      if (timerActive || powerEnabled) {
        leds[i] = CHSV(hue, sat, value);
      } else {
        leds[i] = CHSV(0, 0, 0);
      }
    }
    FastLED.show();

  } else if (cfg.getConfigVersion(EEPROM) != cfg.getId()) {
    Serial.println("NO CONFIG");
  }
  delay(50);
  c++;
}
