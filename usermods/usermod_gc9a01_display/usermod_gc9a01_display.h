#pragma once
#include "wled.h"

#ifdef USERMOD_GC9A01_DISPLAY

#include <TFT_eSPI.h>
#include <SPI.h>

// Pin definitions (can be overridden via build flags)
#ifndef GC9A01_CS_PIN
  #define GC9A01_CS_PIN 5
#endif

#ifndef GC9A01_DC_PIN
  #define GC9A01_DC_PIN 16
#endif

#ifndef GC9A01_RST_PIN
  #define GC9A01_RST_PIN 17
#endif

#ifndef GC9A01_BL_PIN
  #define GC9A01_BL_PIN 4
#endif

class UsermodGC9A01Display : public Usermod {
  private:
    TFT_eSPI tft = TFT_eSPI();
    
    bool displayEnabled = true;
    bool needsRedraw = true;
    bool displayTurnedOff = false;
    unsigned long lastUpdate = 0;
    unsigned long lastRedraw = 0;
    uint8_t brightness = 255;
    uint16_t displayTimeout = 60000; // 60 seconds
    
    String currentEffectName = "";
    uint8_t currentBrightness = 0;
    bool currentPowerState = false;
    
    bool encoderEnabled = false;
    
    void initDisplay() {
      if (!displayEnabled) return;
      
      pinMode(GC9A01_BL_PIN, OUTPUT);
      digitalWrite(GC9A01_BL_PIN, HIGH);
      
      tft.init();
      tft.setRotation(0); // Portrait mode
      tft.fillScreen(TFT_BLACK);
      
      // Initialize with WLED logo
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(2);
      tft.drawString("WLED", 120, 100, 2);
      tft.setTextSize(1);
      tft.drawString("GC9A01 Display", 120, 130, 2);
      
      Serial.println(F("GC9A01: Display initialized"));
    }
    
    void updateDisplay() {
      if (displayTurnedOff) return;
      
      bool stateChanged = false;
      
      // Check for state changes
      if (currentPowerState != (bri > 0)) {
        currentPowerState = (bri > 0);
        stateChanged = true;
      }
      
      if (currentBrightness != bri) {
        currentBrightness = bri;
        stateChanged = true;
      }
      
      String effectName = "";
      if (currentPlaylist >= 0) {
        effectName = "Playlist";
      } else {
        effectName = JSON_mode_names[strip.getMode()];
      }
      
      if (currentEffectName != effectName) {
        currentEffectName = effectName;
        stateChanged = true;
      }
      
      if (stateChanged) {
        drawMainScreen();
      }
    }
    
    void drawMainScreen() {
      tft.fillScreen(TFT_BLACK);
      
      drawStatusBar();
      
      // Power state
      if (currentPowerState) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("ON", 10, 40, 2);
      } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("OFF", 10, 40, 2);
      }
      
      // Brightness
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Brightness:", 10, 70, 2);
      tft.drawString(String(currentBrightness), 10, 90, 2);
      
      // Effect name
      tft.drawString("Effect:", 10, 120, 2);
      
      String displayEffect = currentEffectName;
      if (displayEffect.length() > 15) {
        displayEffect = displayEffect.substring(0, 12) + "...";
      }
      tft.drawString(displayEffect, 10, 140, 2);
      
      // Color preview
      if (currentPowerState) {
        uint32_t color = strip.getPixelColor(0);
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        uint16_t color565 = tft.color565(r, g, b);
        tft.fillRect(10, 170, 60, 30, color565);
      }
    }
    
    void drawStatusBar() {
      // WiFi status
      if (WiFi.status() == WL_CONNECTED) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("WiFi", 10, 10, 1);
      } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("WiFi", 10, 10, 1);
      }
      
      // Time display
      if (toki.isValid()) {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(toki.getTimeString(), 150, 10, 1);
      }
    }
    
    void setBrightness(uint8_t bri) {
      brightness = bri;
      analogWrite(GC9A01_BL_PIN, brightness);
    }
    
    void sleepDisplay() {
      digitalWrite(GC9A01_BL_PIN, LOW);
      displayTurnedOff = true;
      Serial.println(F("GC9A01: Display sleeping"));
    }
    
    void wakeDisplay() {
      digitalWrite(GC9A01_BL_PIN, HIGH);
      displayTurnedOff = false;
      needsRedraw = true;
      lastUpdate = millis();
      Serial.println(F("GC9A01: Display waking"));
    }
    
  public:
    void setup() override {
      initDisplay();
      
      #ifdef USERMOD_ROTARY_ENCODER_UI_ALT
        encoderEnabled = true;
        Serial.println(F("GC9A01: Rotary encoder integration enabled"));
      #endif
      
      needsRedraw = true;
    }
    
    void loop() override {
      if (!displayEnabled) return;
      
      unsigned long now = millis();
      
      // Check for display timeout
      if (displayTimeout > 0 && (now - lastUpdate > displayTimeout)) {
        if (!displayTurnedOff) {
          sleepDisplay();
        }
        return;
      }
      
      // Update display every 100ms or when needed
      if (needsRedraw || (now - lastRedraw > 100)) {
        updateDisplay();
        lastRedraw = now;
        needsRedraw = false;
      }
    }
    
    void onUpdateBegin(bool init) override {
      if (displayTurnedOff) {
        wakeDisplay();
      } else {
        lastUpdate = millis();
        needsRedraw = true;
      }
    }
    
    void connected() override {
      needsRedraw = true;
    }
    
    void addToJsonInfo(JsonObject& root) override {
      JsonObject user = root[F("u")];
      if (user.isNull()) user = root.createNestedObject(F("u"));
      
      JsonArray gc9a01_arr = user.createNestedArray(F("GC9A01"));
      
      if (displayEnabled) {
        gc9a01_arr.add(F("Display: ON"));
        gc9a01_arr.add(displayTurnedOff ? F("Sleeping") : F("Active"));
      } else {
        gc9a01_arr.add(F("Display: OFF"));
      }
    }
    
    void addToConfig(JsonObject& root) override {
      JsonObject top = root.createNestedObject(F("GC9A01"));
      top[F("enabled")] = displayEnabled;
      top[F("timeout")] = displayTimeout / 1000; // Convert to seconds
      top[F("brightness")] = brightness;
    }
    
    bool readFromConfig(JsonObject& root) override {
      JsonObject top = root[F("GC9A01")];
      if (top.isNull()) return false;
      
      bool configComplete = !top.isNull();
      configComplete &= getJsonValue(top[F("enabled")], displayEnabled);
      
      uint16_t timeoutSeconds = displayTimeout / 1000;
      configComplete &= getJsonValue(top[F("timeout")], timeoutSeconds);
      displayTimeout = timeoutSeconds * 1000;
      
      configComplete &= getJsonValue(top[F("brightness")], brightness);
      
      if (configComplete) {
        setBrightness(brightness);
      }
      
      return configComplete;
    }
    
    // Rotary encoder integration methods
    void displayNextItem() {
      if (displayTurnedOff) {
        wakeDisplay();
        return;
      }
      lastUpdate = millis();
      needsRedraw = true;
    }
    
    void displayPreviousItem() {
      if (displayTurnedOff) {
        wakeDisplay();
        return;
      }
      lastUpdate = millis();
      needsRedraw = true;
    }
    
    void displaySelectItem() {
      if (displayTurnedOff) {
        wakeDisplay();
        return;
      }
      lastUpdate = millis();
      needsRedraw = true;
    }
    
    void forceRedraw() { needsRedraw = true; }
    
    uint16_t getId() override { return USERMOD_ID_GC9A01_DISPLAY; }
};

#endif // USERMOD_GC9A01_DISPLAY