#include "usermod_gc9a01_display.h"

#ifdef USERMOD_GC9A01_DISPLAY

// Implementation of all your methods here
void UsermodGC9A01Display::setup() {
  // ...implementation...
}

void UsermodGC9A01Display::loop() {
  // ...implementation...
}

// ...all other method implementations...

// EXACT REGISTRATION PATTERN - these two lines are critical
static UsermodGC9A01Display usermod_gc9a01_display;
REGISTER_USERMOD(usermod_gc9a01_display);

#endif // USERMOD_GC9A01_DISPLAY