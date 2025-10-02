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

#ifndef USERMOD_ID_GC9A01_DISPLAY
  #define USERMOD_ID_GC9A01_DISPLAY 42
#endif

class UsermodGC9A01Display : public Usermod {
  private:
    TFT_eSPI tft = TFT_eSPI();
    
    bool displayEnabled = true;
    bool needsRedraw = true;
    bool displayTurnedOff = false;
    bool showingWelcomeScreen = true;
    unsigned long welcomeScreenStartTime = 0;
    unsigned long lastUpdate = 0;
    unsigned long lastRedraw = 0;
    unsigned long lastDebugPrint = 0;
    uint8_t brightness = 255;
    uint16_t displayTimeout = 60000; // 60 seconds
    
    String currentEffectName = "";
    uint8_t currentBrightness = 0;
    bool currentPowerState = false;
    
    bool encoderEnabled = false;
    
    // Private method declarations
    void initDisplay();
    void updateDisplay();
    void drawMainScreen();
    void drawStatusBar();
    void setBrightness(uint8_t bri);
    void sleepDisplay();
    void wakeDisplay();

  public:
    // Public method declarations (Usermod interface)
    void setup() override;
    void loop() override;
    void addToJsonInfo(JsonObject& root) override;
    void readFromJsonState(JsonObject& root) override;
    void addToJsonState(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    void addToConfig(JsonObject& root) override;
    void appendConfigData() override;
    uint16_t getId() override;
};

#endif // USERMOD_GC9A01_DISPLAY