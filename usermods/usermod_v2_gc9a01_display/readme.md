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
extends = env:esp32dev
upload_speed = 460800
monitor_speed = 115200
custom_usermods =
  usermod_v2_gc9a01_display
  usermod_v2_rotary_encoder_ui_ALT
build_flags = ${common.build_flags} ${esp32_idf_V4.build_flags}
  -D FLD_SPI_DEFAULT
  -D HW_PIN_SCLKSPI=18
  -D HW_PIN_MOSISPI=23
  -D FLD_PIN_DC=15
  -D FLD_PIN_CS=5
  -D FLD_PIN_RESET=17
  -D WLED_DEBUG
  -D WLED_DISABLE_BROWNOUT_DET
  -D USERMOD_GC9A01_DISPLAY
  -DUSER_SETUP_LOADED=1
  -DGC9A01_DRIVER=1
  -DTFT_WIDTH=240
  -DTFT_HEIGHT=240
  -DTFT_MOSI=23
  -DTFT_SCLK=18
  -DTFT_CS=5
  -DTFT_DC=15
  -DTFT_RST=17
  -DTFT_BL=26
  -DTOUCH_CS=-1
  -DLOAD_GLCD=1
  -DLOAD_FONT2=1
  -DLOAD_FONT4=1
  -DLOAD_FONT6=1
  -DLOAD_FONT7=1
  -DLOAD_FONT8=1
  -DLOAD_GFXFF=1
  -DSMOOTH_FONT=1
  -DSPI_FREQUENCY=27000000
```
