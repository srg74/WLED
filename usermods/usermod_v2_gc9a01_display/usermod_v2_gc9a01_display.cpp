#include "usermod_v2_gc9a01_display.h"
#include "logo_data.h"

#ifdef USERMOD_GC9A01_DISPLAY

// Private method implementations
void UsermodGC9A01Display::initDisplay() {
  if (!displayEnabled) return;
  
  DEBUG_PRINTLN(F("[GC9A01] Initializing TFT display..."));
  
  pinMode(GC9A01_BL_PIN, OUTPUT);
  digitalWrite(GC9A01_BL_PIN, HIGH);
  DEBUG_PRINTLN(F("[GC9A01] Backlight pin configured"));
  
  tft.init();
  DEBUG_PRINTLN(F("[GC9A01] TFT init() completed"));
  
  tft.setRotation(0); // Portrait mode
  tft.fillScreen(TFT_BLACK); // Black background
  DEBUG_PRINTLN(F("[GC9A01] Screen cleared to black"));
  
  // Show the WLED logo first during initialization
  drawWLEDLogo();
  
  DEBUG_PRINTLN(F("[GC9A01] WLED logo displayed during initialization"));
  
  // Set welcome screen timer for non-blocking transition to main interface
  showingWelcomeScreen = true;
  welcomeScreenStartTime = millis();
}

void UsermodGC9A01Display::updateDisplay() {
  if (!displayEnabled || displayTurnedOff) {
    DEBUG_PRINTF("[GC9A01] Update blocked - enabled: %d, turnedOff: %d\n", displayEnabled, displayTurnedOff);
    return;
  }
  
  // Rate limiting like 4-line display usermod
  unsigned long now = millis();
  if (now < nextUpdate) return;
  nextUpdate = now + refreshRate;
  
  DEBUG_PRINTLN(F("[GC9A01] updateDisplay() called - checking for changes"));
  
  bool needsRedraw = false;
  
  // Handle welcome screen transition (non-blocking)
  if (showingWelcomeScreen && (now - welcomeScreenStartTime > 2000)) { // Show logo for 2 seconds
    showingWelcomeScreen = false;
    needsRedraw = true;
    DEBUG_PRINTLN(F("[GC9A01] Transitioning from logo to main interface"));
  }
  
  // Check for changes like 4-line display usermod does
  if (knownBrightness != bri) {
    knownBrightness = bri;
    needsRedraw = true;
  }
  
  bool powerState = (bri > 0);
  if (knownPowerState != powerState) {
    knownPowerState = powerState;
    needsRedraw = true;
  }
  
  // Check for effect mode changes
  uint8_t currentMode = strip.getMainSegment().mode; // Get mode from main segment
  if (knownMode != currentMode) {
    DEBUG_PRINTF("[GC9A01] Effect changed: %d -> %d\n", knownMode, currentMode);
    knownMode = currentMode;
    needsRedraw = true;
  }
  
  if (knownPalette != effectPalette) {
    knownPalette = effectPalette;
    needsRedraw = true;
  }

  // Check for static color changes (not dynamic pixel colors)
  uint32_t currentFxColor = strip.getMainSegment().colors[0]; // Static FX color (csl0)
  if (knownColor != currentFxColor) {
    knownColor = currentFxColor;
    needsRedraw = true;
  }

  uint32_t currentBgColor = strip.getMainSegment().colors[1]; // Static BG color (csl1)
  if (knownBgColor != currentBgColor) {
    knownBgColor = currentBgColor;
    needsRedraw = true;
  }

  // Check for time changes (like 4-line display usermod)
  if (localTime != 0) { // Only if time is available
    uint8_t currentMinute = minute(localTime);
    uint8_t currentHour = hour(localTime);
    
    if (knownMinute != currentMinute) {
      knownMinute = currentMinute;
      needsRedraw = true;
    }
    
    if (knownHour != currentHour) {
      knownHour = currentHour;
      needsRedraw = true;
    }
  }

  // Check for display timeout (like 4-line display usermod)
  if (!needsRedraw && displayTimeout > 0) {
    // Turn off display after timeout with no change
    if (!displayTurnedOff && (now - lastRedraw > displayTimeout)) {
      DEBUG_PRINTF("[GC9A01] Display timeout - going to sleep (no change for %lu ms)\n", now - lastRedraw);
      sleepDisplay();
      return;
    }
  }

  // Only redraw if something actually changed
  if (needsRedraw) {
    // Wake display if it was sleeping and there are changes
    if (displayTurnedOff) {
      DEBUG_PRINTLN(F("[GC9A01] Waking display due to changes"));
      wakeDisplay();
    }
    
    lastRedraw = now; // Update last redraw timestamp
    DEBUG_PRINTLN(F("[GC9A01] State changed - updating display"));
    drawMainInterface();
  } else {
    DEBUG_PRINTF("[GC9A01] No changes detected (last redraw: %lu ms ago)\n", now - lastRedraw);
  }
}

void UsermodGC9A01Display::drawMainInterface() {
  tft.fillScreen(TFT_BLACK);
  
  // Get static segment colors (not dynamic effect colors)
  // FX color from csl0 (primary color slot) - static
  uint32_t fxColor = strip.getMainSegment().colors[0]; // csl0 - Primary/FX color (static)
  uint8_t fx_r = (fxColor >> 16) & 0xFF;
  uint8_t fx_g = (fxColor >> 8) & 0xFF;
  uint8_t fx_b = fxColor & 0xFF;
  uint16_t fxColor565 = tft.color565(fx_r, fx_g, fx_b);
  
  // Background color from csl1 (secondary color slot)
  uint32_t bgColor = strip.getMainSegment().colors[1]; // csl1 - Secondary/BG color
  uint8_t bg_r = (bgColor >> 16) & 0xFF;
  uint8_t bg_g = (bgColor >> 8) & 0xFF;
  uint8_t bg_b = bgColor & 0xFF;
  uint16_t bgColor565 = tft.color565(bg_r, bg_g, bg_b);
  
  // Draw outer circle border (static blue)
  tft.drawCircle(120, 120, 115, TFT_BLUE);
  tft.drawCircle(120, 120, 114, TFT_BLUE);
  
  // Draw circular progress ring for brightness from 8 o'clock to 4 o'clock
  int brightnessPercent = map(bri, 0, 255, 0, 100);
  
  // Arc from 8 o'clock (240°) to 4 o'clock (120°) = 240° total arc
  // 8 o'clock = 240° from 0°, 4 o'clock = 480° (or 120° next day)
  float startAngle = 240; // 8 o'clock position
  float arcLength = 240;  // Total arc length in degrees
  float progressAngle = map(brightnessPercent, 0, 100, 0, arcLength);
  
  // Draw brightness progress ring in white
  for (float angle = 0; angle < progressAngle; angle += 3) {
    float currentAngle = startAngle + angle;
    float rad = radians(currentAngle - 90); // Convert to radians, adjust for top reference
    
    // Brightness ring (radius 108-104, 5px wide)
    for (int ringWidth = 0; ringWidth < 5; ringWidth++) {
      int radius = 108 - ringWidth;
      int x = 120 + radius * cos(rad);
      int y = 120 + radius * sin(rad);
      
      if (brightnessPercent > 0) {
        tft.drawPixel(x, y, TFT_WHITE);
      }
    }
  }
  
  // Draw background (unfilled) part of the arc in dark grey
  for (float angle = progressAngle; angle < arcLength; angle += 3) {
    float currentAngle = startAngle + angle;
    float rad = radians(currentAngle - 90);
    
    for (int ringWidth = 0; ringWidth < 5; ringWidth++) {
      int radius = 108 - ringWidth;
      int x = 120 + radius * cos(rad);
      int y = 120 + radius * sin(rad);
      tft.drawPixel(x, y, TFT_DARKGREY);
    }
  }
  
  // WiFi icon at top
  tft.setTextDatum(MC_DATUM);
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillCircle(120, 40, 12, TFT_DARKGREEN);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    tft.drawString("WiFi", 120, 40, 1);
  } else {
    tft.fillCircle(120, 40, 12, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.drawString("WiFi", 120, 40, 1);
  }
  
  // Time display (large, centered-top)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  
  // Get current time from WLED's NTP system
  String timeStr = "--:--"; // Default when time not available
  if (localTime != 0) { // Only if time is available
    uint8_t currentHour = hour(localTime);
    uint8_t currentMinute = minute(localTime);
    
    // Format time based on 12/24 hour preference (using 24h for now)
    timeStr = "";
    if (currentHour < 10) timeStr += "0";
    timeStr += String(currentHour);
    timeStr += ":";
    if (currentMinute < 10) timeStr += "0";
    timeStr += String(currentMinute);
  }
  tft.drawString(timeStr, 120, 80, 4); // Large font
  
  // Power status - get actual WLED state
  bool powerState = (bri > 0);
  if (powerState) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("ON", 120, 130, 2);
    
    // Green toggle switch (ON position)
    tft.fillRoundRect(105, 145, 30, 15, 7, TFT_GREEN);
    tft.fillCircle(125, 152, 6, TFT_WHITE); // Toggle knob (right position = ON)
  } else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("OFF", 120, 130, 2);
    
    // Red toggle switch (OFF position)
    tft.fillRoundRect(105, 145, 30, 15, 7, TFT_RED);
    tft.fillCircle(115, 152, 6, TFT_WHITE); // Toggle knob (left position = OFF)
  }
  
  // Effect name - moved to higher position
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  String effectName = "";
  if (currentPlaylist >= 0) {
    effectName = "Playlist";
  } else if (knownMode < strip.getModeCount() && knownMode >= 0) {
    // Use WLED's proper method to get mode name - simplified approach
    char modeBuffer[64];  // Increased buffer size
    strncpy_P(modeBuffer, strip.getModeData(knownMode), sizeof(modeBuffer)-1);
    modeBuffer[sizeof(modeBuffer)-1] = '\0'; // Ensure null termination
    
    // Find the first separator (could be @, ;, or other characters)
    char* sepPtr = strpbrk(modeBuffer, "@;,|=");
    if (sepPtr) *sepPtr = '\0'; // Terminate at first separator found
    
    // Convert to String and clean up
    effectName = String(modeBuffer);
    
    // Remove any remaining non-printable characters
    String cleanName = "";
    for (int i = 0; i < effectName.length(); i++) {
      char c = effectName.charAt(i);
      if (c >= 32 && c <= 126 && c != '@') { // Printable ASCII, exclude @
        cleanName += c;
      }
    }
    effectName = cleanName;
    
    // Fallback if we end up with empty name
    if (effectName.length() == 0) {
      effectName = "Effect " + String(knownMode);
    }
  } else {
    effectName = "Unknown";
  }
  
  // Trim length for display
  if (effectName.length() > 12) {
    effectName = effectName.substring(0, 9) + "...";
  }
  tft.drawString(effectName, 120, 190, 2); // Now at Y=190 (was brightness position)
  
  // Color indicators - positioned at clock positions, away from edge
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // FX indicator at 7:30 position (225 degrees)
  int fx_x = 120 + 90 * cos(radians(225 - 90)); // -90 to adjust for top reference
  int fx_y = 120 + 90 * sin(radians(225 - 90));
  tft.drawString("FX", fx_x, fx_y - 15, 1);
  tft.fillCircle(fx_x, fx_y, 8, fxColor565);
  tft.drawCircle(fx_x, fx_y, 8, TFT_WHITE);
  
  // BG indicator at 4:30 position (135 degrees)  
  int bg_x = 120 + 90 * cos(radians(135 - 90)); // -90 to adjust for top reference
  int bg_y = 120 + 90 * sin(radians(135 - 90));
  tft.drawString("BG", bg_x, bg_y - 15, 1);
  tft.fillCircle(bg_x, bg_y, 8, bgColor565);
  tft.drawCircle(bg_x, bg_y, 8, TFT_WHITE);
  
  // Brightness percentage - moved to lower position
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  String brightStr = String(brightnessPercent) + "%";
  tft.drawString(brightStr, 120, 210, 2); // Now at Y=210 (was effect position)
  
  DEBUG_PRINTF("[GC9A01] Single ring interface - brightness: %d%%, FX: R%d G%d B%d, BG: R%d G%d B%d, power: %s\n", 
                brightnessPercent, fx_r, fx_g, fx_b, bg_r, bg_g, bg_b, powerState ? "ON" : "OFF");
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
  if (knownPowerState) {
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
  String effectName = "";
  if (knownMode < strip.getModeCount()) {
    effectName = JSON_mode_names[knownMode];
  } else {
    effectName = "Unknown";
  }
  if (effectName.length() > 12) {
    effectName = effectName.substring(0, 9) + "...";
  }
  tft.drawString(effectName, 120, 120, 2);
  
  // Brightness arc/circle at bottom
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Bright", 120, 150, 1);
  
  // Convert brightness from 0-255 to 0-100% for display
  int brightnessPercent = map(knownBrightness, 0, 255, 0, 100);
  tft.drawString(String(brightnessPercent) + "%", 120, 165, 2);
  
  // Draw brightness indicator arc (bottom semicircle)
  int brightness_angle = map(knownBrightness, 0, 255, 180, 360); // 180-360 degrees
  for (int angle = 180; angle <= brightness_angle; angle += 5) {
    int x = 120 + 95 * cos(radians(angle));
    int y = 120 + 95 * sin(radians(angle));
    tft.fillCircle(x, y, 2, TFT_YELLOW);
  }
  
  // Color preview in center (small circle)
  if (knownPowerState) {
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

void UsermodGC9A01Display::drawWLEDLogo() {
  // Display the WLED logo bitmap centered on the display
  // The bitmap is 120x120 pixels, centered on 240x240 display (60px offset on each side)
  
  DEBUG_PRINTLN(F("[GC9A01] Drawing WLED logo bitmap..."));
  
  // Clear screen with black background
  tft.fillScreen(TFT_BLACK);
  
  // Calculate center position for 120x120 logo on 240x240 display
  const int LOGO_SIZE = 120;
  const int OFFSET_X = (240 - LOGO_SIZE) / 2; // 60px offset
  const int OFFSET_Y = (240 - LOGO_SIZE) / 2; // 60px offset
  
  // Set drawing window to the logo area (centered)
  tft.setAddrWindow(OFFSET_X, OFFSET_Y, LOGO_SIZE, LOGO_SIZE);
  
  // Start data transmission
  tft.startWrite();
  
  // Process each pixel in the bitmap
  for (int i = 0; i < 14400; i++) { // 120 * 120 = 14,400 pixels
    uint32_t pixel = pgm_read_dword(&epd_bitmap_[i]);
    
    // Extract RGB components from 32-bit ARGB (format: 0x00RRGGBB)
    uint8_t r = (pixel >> 16) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = pixel & 0xFF;
    
    // Convert to 16-bit RGB565 format
    uint16_t color = tft.color565(r, g, b);
    
    // Write pixel to display
    tft.pushColor(color);
  }
  
  // End data transmission
  tft.endWrite();
  
  DEBUG_PRINTLN(F("[GC9A01] WLED logo bitmap rendered successfully - 120x120 centered"));
}

void UsermodGC9A01Display::setBrightness(uint8_t bri) {
  brightness = bri;
  analogWrite(GC9A01_BL_PIN, brightness);
}

void UsermodGC9A01Display::sleepDisplay() {
  digitalWrite(GC9A01_BL_PIN, LOW);
  displayTurnedOff = true;
  DEBUG_PRINTLN(F("GC9A01: Display sleeping"));
}

void UsermodGC9A01Display::wakeDisplay() {
  digitalWrite(GC9A01_BL_PIN, HIGH);
  displayTurnedOff = false;
  needsRedraw = true;
  lastRedraw = millis(); // Reset timeout when waking up
  DEBUG_PRINTLN(F("GC9A01: Display waking"));
}

// Public method implementations (Usermod interface)
void UsermodGC9A01Display::setup() {
  DEBUG_PRINTLN(F(""));
  DEBUG_PRINTLN(F("=== GC9A01 Display Usermod ==="));
  DEBUG_PRINTLN(F("[GC9A01] Usermod successfully registered and setup() called"));
  DEBUG_PRINT(F("[GC9A01] TFT_eSPI library version: "));
  DEBUG_PRINTLN(TFT_ESPI_VERSION);
  
  initDisplay();
  
  #ifdef USERMOD_ROTARY_ENCODER_UI_ALT
    encoderEnabled = true;
    DEBUG_PRINTLN(F("[GC9A01] Rotary encoder integration enabled"));
  #endif
  
  DEBUG_PRINTLN(F("[GC9A01] Display initialization complete"));
  needsRedraw = true;
  lastRedraw = millis(); // Initialize timeout tracking
  
  // Initialize known values to force initial update
  knownMode = 255; // Force initial effect name update
  knownBrightness = 255; // Force initial brightness update
}

void UsermodGC9A01Display::loop() {
  // Simple loop like 4-line display usermod - minimal work here
  if (!displayEnabled || strip.isUpdating()) return;
  updateDisplay();
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