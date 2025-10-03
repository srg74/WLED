# GC9A01 Display Usermod

Adds support for GC9A01 240x240 round TFT display to WLED with comprehensive status display and rotary encoder integration.

## Features

### Visual Interface
- **Circular Design**: Optimized for 240x240 round displays with blue bezel theming
- **Real-time Clock**: Large digital clock display with precise positioning
- **WiFi Signal Strength**: Visual signal strength indicator with 4-level bars (25%, 50%, 75%, 100%)
- **Power Status**: Dynamic "OFF [switch] ON" layout with contextual text
- **Brightness Arc**: Semicircular brightness visualization (0-255 mapped to 180-360°)
- **Color Controls**: Three color buttons (FX, BG, CS) with live color preview
- **Effect Display**: Current effect name with smart text truncation
- **Startup Logo**: WLED logo display during initialization (2-second duration)

### Status Information
- **Real-time Updates**: 500ms refresh rate with optimized state tracking
- **WiFi Status**: Connection state and signal strength monitoring
- **Power State**: Visual ON/OFF indicator with switch metaphor
- **Brightness**: Percentage display and arc visualization
- **Effect Info**: Current effect name and mode display
- **Color Preview**: Live RGB color display for all three color slots

### Performance Features
- **State Caching**: Minimal redraws using change detection
- **Non-blocking Updates**: Asynchronous display updates
- **Memory Optimized**: Efficient memory usage with proper cleanup
- **Debug Support**: Comprehensive debug logging with WLED macros

## Hardware Requirements
- ESP32 development board
- GC9A01 240x240 TFT display (round recommended)
- Optional: Rotary encoder (usermod_v2_rotary_encoder_ui_ALT)

## Wiring

### GC9A01 Display
| GC9A01 Pin | ESP32 Pin | Function    | Description |
|------------|-----------|-------------|-------------|
| VCC        | 3.3V      | Power       | 3.3V power supply |
| GND        | GND       | Ground      | Common ground |
| SCL/SCLK   | GPIO18    | SPI Clock   | SPI clock signal |
| SDA/MOSI   | GPIO23    | SPI MOSI    | SPI data out |
| RES/RST    | GPIO17    | Reset       | Display reset |
| DC         | GPIO16    | Data/Command| Data/Command control |
| CS         | GPIO5     | Chip Select | SPI chip select |
| BL         | GPIO4     | Backlight   | Backlight control |

**Note**: Pin assignments can be customized via build flags (see Configuration section).

## Display Layout

The interface uses a circular layout optimized for round displays:

```
     12 o'clock: WiFi Signal (4 bars)
    1-2 o'clock: Clock Display (HH:MM)
    3 o'clock:   Power Switch (OFF/ON)
    4:30 clock:  BG Color Button
    6 o'clock:   Brightness % + Effect Name  
    7:30 clock:  FX Color Button
    9 o'clock:   CS Color Button
   Center Ring:  Brightness Arc (180°-360°)
   Outer Ring:  Blue Border (matches bezel)
```

## Configuration

### Basic Setup

Add to your `platformio_override.ini`:

```ini
[env:esp32_gc9a01]
extends = env:esp32dev
upload_speed = 460800
monitor_speed = 115200
build_flags = ${common.build_flags} ${esp32_idf_V4.build_flags}
  -D USERMOD_GC9A01_DISPLAY
  -D USERMOD_ROTARY_ENCODER_UI_ALT
  
  ; TFT_eSPI Configuration
  -DUSER_SETUP_LOADED=1
  -DGC9A01_DRIVER=1
  -DTFT_WIDTH=240
  -DTFT_HEIGHT=240
  -DTFT_MOSI=23
  -DTFT_SCLK=18
  -DTFT_CS=5
  -DTFT_DC=16
  -DTFT_RST=17
  -DTFT_BL=4
  -DTOUCH_CS=-1
  
  ; Font Loading
  -DLOAD_GLCD=1
  -DLOAD_FONT2=1
  -DLOAD_FONT4=1
  -DLOAD_FONT6=1
  -DLOAD_FONT7=1
  -DLOAD_FONT8=1
  -DLOAD_GFXFF=1
  -DSMOOTH_FONT=1
  
  ; SPI Performance
  -DSPI_FREQUENCY=27000000
  
  ; Debug (optional)
  -D WLED_DEBUG
  
lib_deps = ${env:esp32dev.lib_deps}
  bodmer/TFT_eSPI@^2.5.43

custom_usermods = usermod_v2_gc9a01_display, usermod_v2_rotary_encoder_ui_ALT
```

### Pin Customization

You can customize pin assignments using build flags. The usermod uses standard TFT_eSPI pin definitions:

```ini
build_flags = ${common.build_flags}
  ; TFT_eSPI pin assignments (used by both library and usermod)
  -DTFT_MOSI=23          ; SPI MOSI
  -DTFT_SCLK=18          ; SPI Clock  
  -DTFT_CS=5             ; Chip Select
  -DTFT_DC=15            ; Data/Command
  -DTFT_RST=17           ; Reset
  -DTFT_BL=26            ; Backlight pin
```

## Installation

1. **Copy Files**: Copy the usermod folder to `usermods/usermod_v2_gc9a01_display/`
2. **Update Configuration**: Add the configuration to your `platformio_override.ini`
3. **Build**: Run `pio run -e esp32_gc9a01`
4. **Upload**: Run `pio run -e esp32_gc9a01 --target upload`

## Features Detail

### WiFi Signal Strength
- 4 vertical bars indicating signal strength
- Real-time RSSI monitoring
- Color: White on blue background
- Thresholds: >-50dBm (100%), >-60dBm (75%), >-70dBm (50%), >-80dBm (25%)

### Color Controls
- **FX Button**: Primary color (strip.getMainSegment().colors[0])
- **BG Button**: Secondary color (strip.getMainSegment().colors[1])  
- **CS Button**: Tertiary color (strip.getMainSegment().colors[2])
- All buttons show live color preview with white borders

### Power Switch
- Dynamic layout: "OFF [switch] ON"
- Shows current state with appropriate text
- Visual switch metaphor in center

### Brightness Control
- Semicircular arc from 180° to 360°
- White pixels for filled portion
- Dark gray for unfilled portion
- Percentage display below arc

## Troubleshooting

### Display Issues
- **Black Screen**: Check wiring and power supply
- **Corrupted Display**: Verify SPI frequency (try lower values)
- **No Backlight**: Check backlight pin connection

### Performance Issues
- **Slow Updates**: Increase SPI frequency in configuration
- **Memory Issues**: Ensure sufficient heap memory available
- **Display Lag**: Check refresh rate settings

### Debug Information
Enable debug output with `-D WLED_DEBUG` to see detailed logging:
- Display initialization status
- Update timing information
- State change detection
- Error conditions

## Integration with Rotary Encoder

This usermod works seamlessly with `usermod_v2_rotary_encoder_ui_ALT`:
- Visual feedback for encoder changes
- Real-time brightness updates
- Effect switching display
- Color adjustments with live preview

## Version History

- **v2.0**: Complete redesign with circular layout and enhanced features
- **v1.5**: Added WiFi signal strength and power switch improvements  
- **v1.0**: Initial implementation with basic status display

## License

This usermod is released under the same license as WLED.
