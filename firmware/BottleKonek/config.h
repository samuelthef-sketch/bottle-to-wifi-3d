// ============================================================================
//  BottleKonek - settings you edit before uploading
//  Bongabon Senior High School
// ============================================================================
#pragma once

// ---------- 1. MAIN INTERNET (the router or pocket WiFi that has internet) --
#define MAIN_WIFI_SSID      "PUT_MAIN_WIFI_NAME_HERE"
#define MAIN_WIFI_PASSWORD  "PUT_MAIN_WIFI_PASSWORD_HERE"

// ---------- 2. THE WIFI THAT STUDENTS JOIN ---------------------------------
#define PORTAL_WIFI_NAME    "BottleKonek WiFi"   // open network, no password
#define MAX_PHONES          8                    // ESP32 hotspot limit is 10

// ---------- 3. ADMIN PAGE (http://192.168.4.1/admin) -----------------------
#define ADMIN_PASSWORD      "bshs-admin-2026"    // change this!

// ---------- 4. RULES (admin page can change these later) -------------------
#define DEFAULT_MIN_PER_BOTTLE  5     // minutes added per accepted bottle
#define DEFAULT_GUEST_GRACE_S   60    // guest time resets after this many seconds away
#define DEFAULT_BIN_FULL_PCT    95    // stop accepting bottles at this bin level
#define DROP_IDLE_TIMEOUT_S     30    // drop session ends after this long with no bottle
#define REMEMBER_DAYS           30    // "Remember this phone" lasts this long
#define MACHINE_CODE            "4821"

// ---------- 5. BIN SIZE (measure with a ruler, in cm) -----------------------
#define DEFAULT_BIN_EMPTY_CM    60    // ultrasonic to bin bottom when empty
#define DEFAULT_BIN_FULL_CM     8     // ultrasonic to top of bottles when full

// ---------- 6. PINS (match the wiring diagram) ------------------------------
// 16x2 LCD, wired directly (option A). RW pin goes to GND.
#define PIN_LCD_RS   19
#define PIN_LCD_E    23
#define PIN_LCD_D4   18
#define PIN_LCD_D5   17
#define PIN_LCD_D6   16
#define PIN_LCD_D7    4

#define PIN_SERVO_GATE  25    // MG90S #1: blocks the chute while checking
#define PIN_SERVO_TRAY  26    // MG90S #2: tilts item to bin or back to user

#define PIN_METAL_BX    34    // LJ18A3-8-Z/BX (NPN) through pull-up + divider. LOW = metal
#define PIN_METAL_BY    35    // LJ18A3-8-Z/BY (PNP) through divider.          HIGH = metal
#define PIN_IR          27    // IR obstacle module OUT.                       LOW = item there
#define PIN_US_TRIG     33    // HC-SR04 TRIG
#define PIN_US_ECHO     32    // HC-SR04 ECHO through 1k/2k divider

#define PIN_LED_RED     21    // traffic light module R
#define PIN_LED_YELLOW  22    // traffic light module Y
#define PIN_LED_GREEN   13    // traffic light module G
#define PIN_BUZZER      14    // active buzzer + (black wire to GND)
#define PIN_BUTTON      15    // push button to GND: ends a drop session

// Optional: a capacitive sensor LJC18A3-B-Z/BY wired like the BY sensor.
// Set to a free pin (e.g. 5) to require plastic; leave -1 if you don't have one.
#define PIN_PLASTIC     -1    // HIGH = plastic

// ---------- 7. SERVO ANGLES (adjust after mounting) -------------------------
#define GATE_OPEN_DEG     0
#define GATE_CLOSED_DEG   90
#define TRAY_HOLD_DEG     90
#define TRAY_BIN_DEG      160   // tilts toward the bin
#define TRAY_RETURN_DEG   20    // tilts toward the return slot

// ---------- 8. TIMING (ms) --------------------------------------------------
#define ITEM_DEBOUNCE_MS  200   // IR must see the item this long
#define CHECK_TIME_MS     800   // how long metal sensors are watched
#define DROP_CONFIRM_MS   2000  // IR must clear within this time after tilting to bin
#define TRAY_MOVE_MS      700
