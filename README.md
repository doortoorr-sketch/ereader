⚠️CURRENTLY DOES NOT OPEN BOOKS FROM SD CARD⚠️





ESP32 E-Reader Project

Project Overview

A Kindle-like e-reader device built on the ESP32-S3 Super Mini microcontroller. The device reads .txt files from a MicroSD card and displays them on a WeAct Studio e-paper display. It includes a DS3231 RTC for timekeeping, four navigation buttons, and battery charging support. The UI is designed using SquareLine Studio (LVGL-based).

Current Project Focus

- Active area: 2.9" RBW e-paper bring-up and SD-card / SPI bus stability.
- Update this section when the current work area changes, so future sessions know where the project is at a glance.
- If debugging, start with the active area above before revisiting older subsystems.


Hardware

MCU: ESP32-S3 Super Mini

- Chip: ESP32-S3 dual-core Xtensa LX7, up to 240 MHz
- Flash: 4 MB (QIO mode)
- SRAM: 512 KB
- Board dimensions: 22.52 × 18 mm
- USB: USB-C (native USB, no external serial chip)
- Connectivity: Wi-Fi 802.11 b/g/n, Bluetooth 5.0 / BLE (not used in this project)
- Arduino board selection: "ESP32S3 Dev Module" from esp32 by Espressif package
- PlatformIO board: esp32-s3-devkitm-1
- Reference: https://www.espboards.dev/esp32/esp32-s3-super-mini/

Safe GPIOs (no boot/flash/USB conflicts)

IO1, IO2, IO4, IO5, IO6, IO7, IO8, IO15, IO16, IO17, IO18, IO21

GPIOs to Avoid or Use with Caution

- IO0: Boot button
- IO3: Strapping pin (JTAG interface select) — currently used for battery charge receiver; be aware of boot-time behavior
- IO9: Connected to external flash (FSPIHD) on most modules — currently used for DS3231 SCL; verify it works on your specific module variant
- IO10: Flash chip select (FSPICS0) on some modules — currently used for YELLOW button; verify compatibility
- IO11: Flash data line (FSPID) on some modules — currently used for BLUE button; verify compatibility
- IO12: Flash clock (FSPICLK) on some modules — currently used for GREEN button; verify compatibility
- IO48: Onboard WS2812 RGB LED + Red LED (shared)
- IO45, IO46: Strapping pins (flash voltage, boot mode)

⚠️ IMPORTANT: GPIOs 9–14 are documented as flash/PSRAM pins on some ESP32-S3 module variants. The Super Mini with 4MB flash may or may not use all of these. If you encounter boot failures or erratic behavior, the button/RTC pins on IO9–IO12 are the first suspects. Test thoroughly during hardware bringup.

E-Paper Display: WeAct Studio Epaper Module

- Supported sizes: 1.54", 2.13", 2.9", 3.7", 4.2"
- Interface: SPI
- Driver IC: SSD1680 (for 2.13" and 2.9" variants)
- Variants: Black & White (BW), or Red/Black/White (RBW)
- BW full refresh time: ~3 seconds
- RBW full refresh time: ~27 seconds
- BW partial refresh: Supported (must still do full refresh periodically)
- RBW partial refresh: NOT supported
- Resolution (2.9" BW): 128 × 296 pixels
- Reference: https://github.com/WeActStudio/WeActStudio.EpaperModule

E-Paper Wiring

| E-Paper Pin | Function         | ESP32 GPIO |
|-------------|------------------|------------|
| BUSY        | Busy signal      | 16     |
| RES         | Reset            | 2      |
| DC          | Data/Command     | 7      |
| CS          | Chip Select      | 5      |
| SDA (MOSI)  | SPI Data In      | 6      |
| SCL (SCK)   | SPI Clock        | 4      |
| VCC         | Power            | 3.3V       |
| GND         | Ground           | GND        |

E-Paper Critical Notes

1. The display MUST be put into powerOff() or hibernate() mode after every refresh. Leaving it powered on will permanently damage the display — this is not recoverable.
2. Minimum refresh interval: 180 seconds recommended. Refresh at least once every 24 hours.
3. Partial refresh caveat: After several partial refreshes, a full refresh is mandatory. Failure to do so causes irreversible display artifacts.
4. Before long-term storage: Run a full clear/refresh cycle.

DS3231 RTC Module

- Interface: I2C
- Library: RTClib by Adafruit (recommended) or ErriezDS3231
- Features: High-precision real-time clock, battery-backed, temperature-compensated crystal oscillator

DS3231 Wiring

| DS3231 Pin | Function | ESP32 GPIO |
|------------|----------|------------|
| SDA        | I2C Data | 8      |
| SCL        | I2C Clock| 9      |
| VCC        | Power    | 3.3V       |
| GND        | Ground   | GND        |

The DS3231 uses a dedicated I2C bus (GPIO 8/9), separate from the SPI bus used by the e-paper and SD card. Initialize with: Wire.begin(8, 9);
MicroSD Card Module

- Interface: SPI
- Filesystem: FAT32 expected

MicroSD Wiring

| SD Pin | Function    | ESP32 GPIO |
|--------|-------------|------------|
| CS     | Chip Select | 18     |
| MOSI   | Data In     | 6      |
| CLK    | Clock       | 4      |
| MISO   | Data Out    | 17     |
| VCC    | Power       | 3.3V       |
| GND    | Ground      | GND        |

SPI Bus Sharing: E-Paper + MicroSD

The e-paper display and MicroSD card share the same SPI bus (MOSI=GPIO6, SCK=GPIO4) with separate CS lines (e-paper CS=GPIO5, SD CS=GPIO18). The e-paper does not use MISO (it's a write-only device); the SD card uses MISO=GPIO17.

SPI Bus Initialization Order (Critical)

Per Espressif documentation, when sharing an SPI bus with an SD card:

1. Initialize the SPI bus.
2. Pull all non-SD CS lines HIGH (set GPIO5 HIGH to deselect e-paper).
3. Mount the SD card FIRST — this puts the SD card into SPI mode. If you communicate with other SPI devices before the SD card is initialized, the SD card may interpret those signals and enter an undefined state.
4. After SD card is mounted, you can freely communicate with the e-paper display.

When using Arduino framework with GxEPD2, you may need to reinitialize SPI with custom pins:cpp
SPI.end();
SPI.begin(/SCK=/4, /MISO=/17, /MOSI=/6, /SS=/5);
⚠️ Always deassert (pull HIGH) the CS of the device you're NOT talking to. SPI bus contention between the SD card and e-paper will cause data corruption and potentially hang the bus.

Buttons

| Button | Color  | ESP32 GPIO | Notes                    |
|--------|--------|------------|--------------------------|
| RED    | Red    | 1      | Safe GPIO                |
| YELLOW | Yellow | 10     | Potential flash pin — test |
| BLUE   | Blue   | 11     | Potential flash pin — test |
| GREEN  | Green  | 12     | Potential flash pin — test |

Configure with internal pull-ups; buttons should connect GPIO to GND when pressed (active LOW):cpp
pinMode(1, INPUT_PULLUP);   // RED
pinMode(10, INPUT_PULLUP);  // YELLOW
pinMode(11, INPUT_PULLUP);  // BLUE
pinMode(12, INPUT_PULLUP);  // GREEN
Debounce all button inputs (software debounce ~50ms or use a library).

Battery Charging

- Charge detection pin: GPIO 3
- Note: GPIO3 is a strapping pin (JTAG interface select). It is sampled at reset. Ensure the battery charging circuit does not pull this pin to an unexpected level during boot, or it may alter the JTAG/debug interface selection.

Pin Assignment Summary

| GPIO | Assignment              | Bus/Protocol | Notes                          |
|------|-------------------------|--------------|--------------------------------|
| 1    | RED button              | Digital In   | Safe GPIO, use INPUT_PULLUP    |
| 2    | E-paper RES (Reset)     | Digital Out  | Safe GPIO                      |
| 3    | Battery charge receiver | Analog/Digital| ⚠️ Strapping pin              |
| 4    | E-paper SCL + SD CLK    | SPI SCK      | Shared SPI clock               |
| 5    | E-paper CS              | SPI CS       | Safe GPIO                      |
| 6    | E-paper SDA + SD MOSI   | SPI MOSI     | Shared SPI data out            |
| 7    | E-paper DC              | Digital Out  | Safe GPIO                      |
| 8    | DS3231 SDA              | I2C Data     | Safe GPIO                      |
| 9    | DS3231 SCL              | I2C Clock    | ⚠️ May be flash pin on some variants |
| 10   | YELLOW button           | Digital In   | ⚠️ May be flash CS pin         |
| 11   | BLUE button             | Digital In   | ⚠️ May be flash data pin       |
| 12   | GREEN button            | Digital In   | ⚠️ May be flash clock pin      |
| 16   | E-paper BUSY            | Digital In   | Safe GPIO                      |
| 17   | SD MISO                 | SPI MISO     | Safe GPIO                      |
| 18   | SD CS                   | SPI CS       | Safe GPIO                      |

Software Stack

Framework

- Arduino framework on ESP32-S3 (esp32 board package by Espressif, v3.x+)
- Board selection: ESP32S3 Dev Module
- Or PlatformIO with board = esp32-s3-devkitm-1
UI: SquareLine Studio + LVGL

- UI layouts and screens are designed in SquareLine Studio and exported as LVGL C code.
- LVGL drives the e-paper display through a custom flush callback that bridges LVGL's framebuffer to the e-paper driver (GxEPD2 or direct SSD1680 commands).
- Color depth: Use 1-bit monochrome in SquareLine Studio settings to match the BW e-paper.
- Display resolution: Must match the e-paper panel exactly (e.g., 128×296 for 2.9").
- LVGL's lv_disp_drv_t flush callback must convert the LVGL buffer to the e-paper's expected format and trigger a display update.

Key Libraries

| Library                  | Purpose                        | Notes                                      |
|--------------------------|--------------------------------|--------------------------------------------|
| GxEPD2               | E-paper driver                 | Use GxEPD2_BW for BW, GxEPD2_3C for RBW. Driver class depends on panel size (e.g., GxEPD2_290_BS for 2.9" BW SSD1680). |
| Adafruit GFX         | Graphics primitives            | Dependency of GxEPD2                       |
| LVGL                 | UI framework                   | SquareLine Studio export target            |
| RTClib (Adafruit)    | DS3231 RTC interface           | I2C on GPIO 8/9                            |
| SD or SdFat      | MicroSD card file access       | FAT32, SPI on shared bus                   |
| Wire                 | I2C for DS3231                 | Wire.begin(8, 9)                         |

GxEPD2 Driver Selection Guide

For WeAct Studio modules using SSD1680:

| Panel Size | BW Driver Class       | 3-Color Driver Class     |
|------------|-----------------------|--------------------------|
| 2.13"      | GxEPD2_213_BN      | GxEPD2_213_Z98c       |
| 2.9"       | GxEPD2_290_BS      | GxEPD2_290_C90c       |

If unsure, consult GxEPD2_display_selection_new_style.h in the GxEPD2 examples for all supported drivers.

Architecture & Key Design Decisions

Reading Flow

1. On boot, initialize I2C (RTC) and SPI (SD card first, then e-paper).
2. Scan the MicroSD card root (or a /books/ directory) for .txt files.
3. Present a book selection menu on the e-paper via LVGL UI.
4. User navigates with buttons (RED/YELLOW/BLUE/GREEN) to select a book.
5. Open the selected .txt file, paginate the text to fit the e-paper resolution.
6. Display one page at a time; buttons control next/previous page, back to menu, etc.
7. Save reading position (current file + byte offset) to SD card or RTC NVRAM for resume functionality.

Text Pagination

- Read a chunk of the .txt file from SD into a RAM buffer.
- Use LVGL's text layout engine (or manual calculation with font metrics) to determine how many characters/lines fit on one e-paper screen.
- Cache page boundary offsets (byte positions in the file) so backward navigation doesn't require re-reading from the start.
- Be mindful of UTF-8 multi-byte characters — do not split mid-character.

Power Management

- E-paper hibernate after every screen update to protect the display.
- ESP32-S3 deep sleep between interactions to maximize battery life. Wake on button press (configure GPIOs as ext0/ext1 wakeup sources).
- Consider reducing CPU frequency (e.g., 80 MHz or even 10 MHz) during active reading — e-paper refresh still works at lower clock speeds.
- The e-paper retains its image with zero power — perfect for a battery-powered reader.

Memory Constraints

- ESP32-S3 Super Mini has 4 MB flash with a max sketch size of 1280 KB and max SPIFFS/data partition of 320 KB (default partition scheme).
- 512 KB SRAM — LVGL needs a display buffer. For a 128×296 monochrome display, a full framebuffer is only ~4.7 KB (128×296/8), which is very manageable.
- Load text from SD in chunks (e.g., 4 KB at a time) rather than loading entire books into RAM.
- Consider a custom partition table if you need more program space for LVGL + fonts.

Common Pitfalls

1. SPI bus contention: Always manage CS lines explicitly. Never leave both e-paper CS and SD CS asserted simultaneously. Initialize SD card before e-paper on shared bus.

2. E-paper damage: Forgetting to call hibernate() or powerOff() after refresh WILL permanently damage the display. There is no fix.

3. GxEPD2 default SPI pins: The library uses the board's default SPI pins, which are NOT your custom pins. You MUST call SPI.end() then SPI.begin(4, 17, 6, 5) after display.init() to remap to your wiring.

4. Boot failures from GPIO 9–12: If buttons or RTC lines on these GPIOs cause unexpected levels during boot, the ESP32-S3 may fail to start or enter the wrong boot mode. Add pull-up/pull-down resistors as needed, or move to safer GPIOs if issues arise.

5. GPIO3 at boot: The battery charging circuit on GPIO3 must not pull this strapping pin to an unintended level during power-on reset.

6. SD card not entering SPI mode: If the SD card is not the first device communicated with on the shared SPI bus after initialization, it may stay in SD mode and interfere with all bus traffic.

7. 3.3V only for e-paper: The e-paper VCC MUST be 3.3V, NOT 5V. Connecting 5V will damage the display.

Button Mapping (Suggested)

| Button | Primary Action        | Secondary/Long Press     |
|--------|-----------------------|--------------------------|
| RED    | Back / Exit           | Power off / Deep sleep   |
| YELLOW | Previous page         | Jump back 10 pages       |
| BLUE   | Next page             | Jump forward 10 pages    |
| GREEN  | Select / Confirm      | Toggle info overlay (time, battery, progress) |

This mapping is a suggestion — adapt to your SquareLine Studio UI design.

Build & Upload

Arduino IDE

1. Install esp32 board package by Espressif (v3.x+).
2. Select board: ESP32S3 Dev Module.
3. Set Flash Size: 4MB, Flash Mode: QIO, USB CDC On Boot: Enabled.
4. Install libraries: GxEPD2, Adafruit GFX, LVGL, RTClib, SD (or SdFat).
5. Copy SquareLine Studio exported UI files into the sketch's ui/ directory.
6. Configure lv_conf.h for monochrome, correct resolution, and tick source.

PlatformIOini
[env:esp32s3-ereader]
platform = espressif32
board = esp32-s3-devkitm-1
framework = arduino
monitor_speed = 115200
board_build.flash_mode = qio
lib_deps =
    zinggjm/GxEPD2
    adafruit/Adafruit GFX Library
    adafruit/RTClib
    lvgl/lvgl
build_flags =
    -D LV_CONF_INCLUDE_SIMPLE
    -D LV_COLOR_DEPTH=1
File Structure (Suggested)
project-root/
├── claude.md                  # This file
├── src/
│   ├── main.cpp               # Entry point, setup/loop
│   ├── display.h/.cpp         # E-paper init, flush, hibernate wrappers
│   ├── sdcard.h/.cpp          # SD card mount, file listing, text reading
│   ├── rtc.h/.cpp             # DS3231 init, get/set time
│   ├── buttons.h/.cpp         # Button reading, debounce, event dispatch
│   ├── reader.h/.cpp          # Text pagination, bookmarking, page cache
│   └── ui/                    # SquareLine Studio generated LVGL UI code
│       ├── ui.h
│       ├── ui.c
│       ├── ui_helpers.h/.c
│       ├── screens/
│       └── fonts/
├── lib/                       # Local library overrides if needed
├── platformio.ini             # PlatformIO config
└── data/                      # SPIFFS data (if used for config/fonts)
MicroSD Card Structure (Expected)
microsd-root/
├── books/
│   ├── book1.txt
│   ├── book2.txt
│   └── ...
└── .bookmarks/                # Auto-generated reading progress
    ├── book1.txt.pos          # Byte offset into the file
    └── book2.txt.pos
References

- ESP32-S3 Super Mini: https://www.espboards.dev/esp32/esp32-s3-super-mini/
- WeAct E-Paper Module: https://github.com/WeActStudio/WeActStudio.EpaperModule
- GxEPD2 Library: https://github.com/ZinggJM/GxEPD2
- LVGL Documentation: https://docs.lvgl.io/
- SquareLine Studio: https://squareline.io/
- Espressif SPI Bus Sharing with SD Card: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/sdspi_share.html
- RTClib (Adafruit): https://github.com/adafruit/RTClib
- ESP32-S3 Super Mini + WeAct 2.9" E-Paper Tutorial: https://medium.com/@androidcrypto/getting-started-with-a-2-9-inch-epaper-display-connected-to-an-esp32-s3-supermini-device-80c8a5b53267

A few things worth calling out about what I included and why:

The GPIO conflict warnings are important. Your current pin assignments put the DS3231 SCL on GPIO9 and three buttons on GPIOs 10, 11, and 12 — all of which are documented as flash/PSRAM pins on some ESP32-S3 module variants. The 4MB Super Mini may not use all of these for flash, but it's a real risk. I flagged every instance so you (or Claude in a future session) won't overlook this during debugging. If you start seeing random boot failures, those pins are the first place to look.

The SPI bus sharing initialization order is something that trips up nearly everyone who puts an SD card and another SPI device on the same bus. The SD card must be initialized first, or it can stay in SD mode and corrupt all traffic. I documented the exact sequence from Espressif's own guide.

The e-paper damage warnings are repeated deliberately — the WeAct documentation and Waveshare both stress that leaving the display powered without hibernation causes permanent, irreparable damage. Every developer on the project needs to internalize this.
