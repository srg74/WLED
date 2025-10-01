# GC9A01 Display Usermod

Adds support for GC9A01 240x240 round TFT display to WLED with rotary encoder integration.

## Features
- Real-time WLED status display
- Effect and palette information
- Brightness control visualization
- WiFi connection status
- Sleep/wake functionality
- Integration with rotary encoder usermod

## Hardware Requirements
- ESP32 development board
- GC9A01 240x240 TFT display
- Optional: Rotary encoder (usermod_v2_rotary_encoder_ui_ALT)

## Wiring

### GC9A01 Display
| GC9A01 | ESP32 | Description |
|--------|-------|-------------|
| VCC    | 3.3V  | Power       |
| GND    | GND   | Ground      |
| SCL    | GPIO18| SPI Clock   |
| SDA    | GPIO23| SPI MOSI    |
| RES    | GPIO17| Reset       |
| DC     | GPIO16| Data/Command|
| CS     | GPIO5 | Chip Select |
| BL     | GPIO4 | Backlight   |

## Configuration

Add to your `platformio_override.ini`:

```ini
[env:esp32_gc9a01]
extends = env:esp32dev
build_flags = ${common.build_flags} ${esp32.build_flags}
  -D USERMOD_GC9A01_DISPLAY
  -D USERMOD_ROTARY_ENCODER_UI_ALT  ; Optional rotary encoder
  -D GC9A01_CS_PIN=5
  -D GC9A01_DC_PIN=16
  -D GC9A01_RST_PIN=17
  -D GC9A01_BL_PIN=4
lib_deps = ${esp32.lib_deps}
  bodmer/TFT_eSPI@^2.5.0