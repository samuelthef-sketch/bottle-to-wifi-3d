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

// ---------- 4. RULES (admin page can change minutes and grace later) -------
#define DEFAULT_MIN_PER_BOTTLE  5     // minutes added per accepted bottle
#define DEFAULT_GUEST_GRACE_S   60    // guest time resets after this many seconds away
#define DEFAULT_BIN_CAPACITY    70    // bin counts as FULL after this many bottles (admin can change)
#define DROP_IDLE_TIMEOUT_S     30    // drop session ends after this long with no bottle
#define REMEMBER_DAYS           30    // "Remember this phone" lasts this long
#define MACHINE_CODE            "4821"

// ---------- 5. PINS (match the wiring diagram) ------------------------------
// 16x2 LCD with I2C board on its back (GND, VCC, SDA, SCL).
// Its address (0x27 or 0x3F) is found automatically at start-up.
#define PIN_I2C_SDA  21
#define PIN_I2C_SCL  22

#define PIN_SERVO_GATE  25    // MG90S: swinging gate at the end of the PVC chute

#define PIN_IR          27    // IR obstacle module OUT at the gate. LOW = bottle waiting
#define PIN_METAL       35    // ONE metal sensor (LJ18A3-8-Z/BY, PNP) through 10k/3.3k divider
#define USE_METAL_SENSOR  0   // 0 now (no MT3608 yet). When the MT3608 is installed: change to 1 and upload
#define METAL_ACTIVE_HIGH 1   // BY (PNP) = 1.  If you use the BX (NPN) sensor instead, set 0
#define PIN_US_TRIG     33    // HC-SR04 below the gate: sees the bottle fall through
#define PIN_US_ECHO     32    // HC-SR04 ECHO through 1k/2.2k divider

#define PIN_LED_RED     17    // traffic light module R
#define PIN_LED_YELLOW  16    // traffic light module Y
#define PIN_LED_GREEN    4    // traffic light module G
#define PIN_BUZZER      14    // active buzzer + (black wire to GND)

// Optional: a capacitive sensor LJC18A3-B-Z/BY wired like the metal sensor.
// Set to a free pin (e.g. 34) to require plastic; leave -1 if you don't have one.
#define PIN_PLASTIC     -1    // HIGH = plastic

// ---------- 6. GATE SERVO ---------------------------------------------------
// Your MG90S is the 360-degree (continuous) type: it cannot go to an angle,
// it spins while it gets a signal. The code spins it for a short time to open,
// stops, and spins it back the same time to close.
// Before powering on, turn the gate by hand to the CLOSED position.
#define SERVO_360         1     // 1 = 360-degree servo (yours). 0 = normal 180-degree servo
#define GATE_OPEN_MS      300   // how long it spins to swing open (tune: bigger = opens wider)
#define GATE_CLOSE_MS     300   // how long it spins back to close (make equal or tune)
#define GATE_SPEED_US     250   // spin speed (100 = slow, 400 = fast)
#define GATE_OPEN_DIR     1     // 1 or -1: flip if the gate opens the wrong way
// Only used when SERVO_360 is 0 (180-degree servo):
#define GATE_CLOSED_DEG   0
#define GATE_OPEN_DEG     90

// ---------- 7. DROP DETECTION (ultrasonic below the gate) -------------------
// At start-up the ESP32 measures the empty distance across the drop path.
// A bottle falling past makes the distance shorter by at least DROP_DELTA_CM.
#define DROP_DELTA_CM     4

// ---------- 8. TIMING (ms) --------------------------------------------------
#define ITEM_DEBOUNCE_MS  200   // IR must see the bottle this long
#define CHECK_TIME_MS     800   // how long the metal sensor is watched
#define DROP_WAIT_MS      3000  // gate stays open this long waiting for the drop
#define GATE_CLOSE_DELAY  600   // keep the gate open a bit after the drop
