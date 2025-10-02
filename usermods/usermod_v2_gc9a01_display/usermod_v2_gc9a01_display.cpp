#include "usermod_v2_gc9a01_display.h"

#ifdef USERMOD_GC9A01_DISPLAY

// Private method implementations
void UsermodGC9A01Display::initDisplay() {
  if (!displayEnabled) return;
  
  Serial.println(F("[GC9A01] Initializing TFT display..."));
  
  pinMode(GC9A01_BL_PIN, OUTPUT);
  digitalWrite(GC9A01_BL_PIN, HIGH);
  Serial.println(F("[GC9A01] Backlight pin configured"));
  
  tft.init();
  Serial.println(F("[GC9A01] TFT init() completed"));
  
  tft.setRotation(0); // Portrait mode
  tft.fillScreen(TFT_BLACK);
  Serial.println(F("[GC9A01] Screen cleared to black"));
  
  // Brief welcome screen - centered layout
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM); // Middle Center datum
  
  // Main title at center
  tft.drawString("WLED", 120, 100, 4);
  
  // Subtitle below
  tft.drawString("GC9A01", 120, 130, 2);
  tft.drawString("Starting...", 120, 150, 2);
  
  // Draw a circle border to show the round area
  tft.drawCircle(120, 120, 110, TFT_BLUE);
  
  Serial.println(F("[GC9A01] Welcome screen displayed - will update to main screen shortly"));
  
  // Set welcome screen timer for non-blocking transition
  showingWelcomeScreen = true;
  welcomeScreenStartTime = millis();
}

void UsermodGC9A01Display::updateDisplay() {
  if (!displayEnabled || displayTurnedOff) return;
  
  lastUpdate = millis();
  
  bool stateChanged = false;
  
  // Force initial update if this is the first run
  static bool firstRun = true;
  if (firstRun) {
    firstRun = false;
    stateChanged = true;
    Serial.println(F("[GC9A01] First updateDisplay() call - forcing screen update"));
  }
  
  // Check brightness changes
  if (bri != currentBrightness) {
    currentBrightness = bri;
    stateChanged = true;
  }
  
  // Check power state changes
  bool powerState = (bri > 0);
  if (powerState != currentPowerState) {
    currentPowerState = powerState;
    stateChanged = true;
  }
  
  String effectName = "";
  if (currentPlaylist >= 0) {
    effectName = "Playlist";
  } else {
    effectName = JSON_mode_names[strip.getMainSegment().mode];
  }
  
  if (currentEffectName != effectName) {
    currentEffectName = effectName;
    stateChanged = true;
  }
  
  if (stateChanged) {
    Serial.println(F("[GC9A01] State changed - updating display"));
    drawMainScreen();
  }
}

void UsermodGC9A01Display::drawMainScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // Set text datum to middle center for all text
  tft.setTextDatum(MC_DATUM);
  
  // Draw outer circle border
  tft.drawCircle(120, 120, 110, TFT_DARKGREY);
  
  // Status ring at top (12 o'clock position)
  drawStatusBar();
  
  // Power state in center-top area
  if (currentPowerState) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillCircle(120, 80, 15, TFT_DARKGREEN);
    tft.drawString("ON", 120, 80, 2);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.fillCircle(120, 80, 15, TFT_MAROON);
    tft.drawString("OFF", 120, 80, 2);
  }
  
  // Main effect name in center
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  String displayEffect = currentEffectName;
  if (displayEffect.length() > 12) {
    displayEffect = displayEffect.substring(0, 9) + "...";
  }
  tft.drawString(displayEffect, 120, 120, 2);
  
  // Brightness arc/circle at bottom
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Bright", 120, 150, 1);
  
  // Convert brightness from 0-255 to 0-100% for display
  int brightnessPercent = map(currentBrightness, 0, 255, 0, 100);
  tft.drawString(String(brightnessPercent) + "%", 120, 165, 2);
  
  // Draw brightness indicator arc (bottom semicircle)
  int brightness_angle = map(currentBrightness, 0, 255, 180, 360); // 180-360 degrees
  for (int angle = 180; angle <= brightness_angle; angle += 5) {
    int x = 120 + 95 * cos(radians(angle));
    int y = 120 + 95 * sin(radians(angle));
    tft.fillCircle(x, y, 2, TFT_YELLOW);
  }
  
  // Color preview in center (small circle)
  if (currentPowerState) {
    uint32_t color = strip.getPixelColor(0);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    uint16_t color565 = tft.color565(r, g, b);
    tft.fillCircle(120, 200, 12, color565);
    tft.drawCircle(120, 200, 12, TFT_WHITE);
  }
}

void UsermodGC9A01Display::drawStatusBar() {
  // WiFi status (top left arc position)
  tft.setTextDatum(TL_DATUM);  // Top Left datum for status items
  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillCircle(50, 50, 8, TFT_DARKGREEN);
    tft.drawString("WiFi", 65, 45, 1);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.fillCircle(50, 50, 8, TFT_MAROON);
    tft.drawString("No WiFi", 65, 45, 1);
  }
  
  // Time display (top right arc position)
  tft.setTextDatum(TR_DATUM);  // Top Right datum
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Time", 190, 45, 1);
  tft.fillCircle(190, 50, 6, TFT_BLUE);
}

void UsermodGC9A01Display::setBrightness(uint8_t bri) {
  brightness = bri;
  analogWrite(GC9A01_BL_PIN, brightness);
}

void UsermodGC9A01Display::sleepDisplay() {
  digitalWrite(GC9A01_BL_PIN, LOW);
  displayTurnedOff = true;
  Serial.println(F("GC9A01: Display sleeping"));
}

void UsermodGC9A01Display::wakeDisplay() {
  digitalWrite(GC9A01_BL_PIN, HIGH);
  displayTurnedOff = false;
  needsRedraw = true;
  Serial.println(F("GC9A01: Display waking"));
}

// Public method implementations (Usermod interface)
void UsermodGC9A01Display::setup() {
  Serial.println(F(""));
  Serial.println(F("=== GC9A01 Display Usermod ==="));
  Serial.println(F("[GC9A01] Usermod successfully registered and setup() called"));
  Serial.print(F("[GC9A01] TFT_eSPI library version: "));
  Serial.println(TFT_ESPI_VERSION);
  
  initDisplay();
  
  #ifdef USERMOD_ROTARY_ENCODER_UI_ALT
    encoderEnabled = true;
    Serial.println(F("[GC9A01] Rotary encoder integration enabled"));
  #endif
  
  Serial.println(F("[GC9A01] Display initialization complete"));
  needsRedraw = true;
}

void UsermodGC9A01Display::loop() {
  if (!displayEnabled) return;
  
  unsigned long now = millis();
  
  // Handle welcome screen transition (non-blocking)
  if (showingWelcomeScreen && (now - welcomeScreenStartTime > 2000)) {
    showingWelcomeScreen = false;
    Serial.println(F("[GC9A01] Transitioning from welcome screen to main display"));
    needsRedraw = true; // Force immediate update to main screen
  }
  
  // Debug output every 10 seconds to verify usermod is running
  if (now - lastDebugPrint > 10000) {
    Serial.println(F("[GC9A01] Usermod loop() running - registration confirmed"));
    int brightnessPercent = map(currentBrightness, 0, 255, 0, 100);
    Serial.printf("[GC9A01] Current state: brightness=%d (%d%%), power=%s, effect=%s\n", 
                  currentBrightness, brightnessPercent, currentPowerState ? "ON" : "OFF", currentEffectName.c_str());
    lastDebugPrint = now;
  }
  
  // Check for display timeout
  if (displayTimeout > 0 && (now - lastUpdate > displayTimeout)) {
    if (!displayTurnedOff) {
      sleepDisplay();
    }
    return;
  }
  
  // Skip updates while showing welcome screen
  if (showingWelcomeScreen) return;
  
  // Update display every 100ms or when needed
  if (needsRedraw || (now - lastRedraw > 100)) {
    updateDisplay();
    lastRedraw = now;
    needsRedraw = false;
  }
}

void UsermodGC9A01Display::addToJsonInfo(JsonObject& root) {
  JsonObject user = root["u"];
  if (user.isNull()) user = root.createNestedObject("u");

  JsonArray temp = user.createNestedArray(F("GC9A01 Display"));
  temp.add(displayEnabled ? F("Enabled") : F("Disabled"));
  temp.add(F(" "));
}

void UsermodGC9A01Display::readFromJsonState(JsonObject& root) {
  if (root[F("gc9a01")] != nullptr) {
    if (root[F("gc9a01")][F("on")] != nullptr) {
      bool newState = root[F("gc9a01")][F("on")];
      if (newState != displayEnabled) {
        displayEnabled = newState;
        if (displayEnabled) {
          wakeDisplay();
        } else {
          sleepDisplay();
        }
      }
    }
  }
}

void UsermodGC9A01Display::addToJsonState(JsonObject& root) {
  JsonObject gc9a01 = root.createNestedObject(F("gc9a01"));
  gc9a01[F("on")] = displayEnabled;
}

bool UsermodGC9A01Display::readFromConfig(JsonObject& root) {
  JsonObject top = root[FPSTR("GC9A01")];
  if (top.isNull()) {
    return false;
  }

  displayEnabled = top[FPSTR("enabled")] | displayEnabled;
  displayTimeout = top[FPSTR("timeout")] | displayTimeout;
  return true;
}

void UsermodGC9A01Display::addToConfig(JsonObject& root) {
  JsonObject top = root.createNestedObject(FPSTR("GC9A01"));
  top[FPSTR("enabled")] = displayEnabled;
  top[FPSTR("timeout")] = displayTimeout;
}

void UsermodGC9A01Display::appendConfigData() {
  oappend(SET_F("addInfo('GC9A01:enabled', 1, 'Enable/disable display');"));
  oappend(SET_F("addInfo('GC9A01:timeout', 1, 'Display timeout in ms (0=disabled)');"));
}

uint16_t UsermodGC9A01Display::getId() {
  return USERMOD_ID_GC9A01_DISPLAY;
}

#endif // USERMOD_GC9A01_DISPLAY

// Registration
UsermodGC9A01Display gc9a01DisplayUsermod;
REGISTER_USERMOD(gc9a01DisplayUsermod);