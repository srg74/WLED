#pragma once
#include "wled.h"

#ifdef USERMOD_GC9A01_DISPLAY

#include <TFT_eSPI.h>
#include <SPI.h>

// Pin definitions - use TFT_eSPI definitions when available
#ifndef TFT_CS
  #define TFT_CS 5  // Default fallback
#endif
#define GC9A01_CS_PIN TFT_CS

#ifndef TFT_DC  
  #define TFT_DC 16  // Default fallback
#endif
#define GC9A01_DC_PIN TFT_DC

#ifndef TFT_RST
  #define TFT_RST 17  // Default fallback  
#endif
#define GC9A01_RST_PIN TFT_RST

// Use TFT_eSPI's backlight pin definition
#ifndef TFT_BL
  #define TFT_BL 4  // Default fallback if not defined by TFT_eSPI
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
    uint8_t brightness = 255;
    uint16_t displayTimeout = 60000; // 60 seconds
    
    // Proper state tracking like 4-line display usermod
    uint8_t knownBrightness = 255;
    uint8_t knownMode = 255;
    uint8_t knownPalette = 255;
    uint32_t knownColor = 0;
    uint32_t knownBgColor = 0;
    bool knownPowerState = true;
    unsigned long nextUpdate = 0;
    unsigned long lastRedraw = 0; // Track last redraw time for timeout
    uint16_t refreshRate = 500; // Reduced to 500ms for better responsiveness
    
    // Time tracking for display updates
    uint8_t knownMinute = 99;
    uint8_t knownHour = 99;
    
    bool encoderEnabled = false;
    
    // Private method declarations
    void initDisplay();
    void updateDisplay();
    void drawMainInterface();
    void drawMainScreen();
    void drawStatusBar();
    void drawWLEDLogo();
    void drawWiFiIcon(int x, int y, bool connected, int rssi = 0);
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