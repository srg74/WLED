// GC9A01 display configuration for TFT_eSPI library
#define GC9A01_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// Pin definitions (can be overridden via build flags)
#define TFT_CS     5   // Chip select control pin
#define TFT_DC     16  // Data Command control pin
#define TFT_RST    17  // Reset pin (could connect to RST pin)
#define TFT_BL     4   // LED back-light

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH
#define LOAD_FONT6  // Font 6. Large 48 pixel high font, needs ~2666 bytes in FLASH
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel high font, needs ~2438 bytes in FLASH
#define LOAD_FONT8  // Font 8. Large 75 pixel high font needs ~3256 bytes in FLASH

#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000