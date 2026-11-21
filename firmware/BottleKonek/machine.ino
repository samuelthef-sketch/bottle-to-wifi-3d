// ============================================================================
//  machine: PVC chute with one swinging gate
//
//   1. Bottle slides down the PVC pipe and stops at the closed gate.
//      The IR sensor at the gate sees it.
//   2. The metal sensor is watched for 0.8 s.
//   3a. Metal (can, foil): gate STAYS CLOSED, red light flashes, 2 beeps.
//       The student takes the item back out.
//   3b. Not metal: gate OPENS. The ultrasonic sensor below the gate must see
//       the bottle fall past within 3 s -> +minutes, 1 beep, gate closes.
//   Bin: every accepted bottle adds 1 to the bin count. At the bin capacity
//   (admin page, default 70) the machine says BIN FULL until the admin
//   presses "I emptied the bin".
// ============================================================================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C *lcd = nullptr;   // created in machineBegin() once the address is known

enum MState { M_IDLE, M_CHECK, M_WAIT_DROP, M_CLOSING, M_WAIT_REMOVE };
static MState   mstate = M_IDLE;
static uint32_t mT = 0;              // time the current state started
static uint32_t irSince = 0;         // IR has seen a bottle since
static uint32_t irClearSince = 0;
static bool     armed = true;        // the gate area must be clear before the next bottle counts
static bool     metalSeen = false, plasticSeen = false;
static float    baselineCm = 0;      // empty distance across the drop path (measured at start-up)
static uint32_t lastUs = 0;

bool binIsFull() { return stats.binCount >= (uint32_t)cfg.binCapacity; }

// ---------------------------------------------------------------------------
//  Servo (LEDC PWM, 50 Hz). MG90S: 500-2400 us
// ---------------------------------------------------------------------------
static void servoWrite(int pin, int deg) {
  deg = constrain(deg, 0, 180);
  uint32_t us = 500 + (uint32_t)deg * 1900 / 180;
  ledcWrite(pin, us * 16383UL / 20000UL);
}
static void gateOpen()  { servoWrite(PIN_SERVO_GATE, GATE_OPEN_DEG); }
static void gateClose() { servoWrite(PIN_SERVO_GATE, GATE_CLOSED_DEG); }

// ---------------------------------------------------------------------------
//  LCD: status screen + short messages
// ---------------------------------------------------------------------------
static String lcdL1, lcdL2;
static uint32_t lcdTempUntil = 0;

void lcdShow(const String &l1, const String &l2) {
  String a = l1.substring(0, 16), b = l2.substring(0, 16);
  while (a.length() < 16) a += ' ';
  while (b.length() < 16) b += ' ';
  if (a == lcdL1 && b == lcdL2) return;
  lcdL1 = a; lcdL2 = b;
  if (!lcd) return;
  lcd->setCursor(0, 0); lcd->print(a);
  lcd->setCursor(0, 1); lcd->print(b);
}

static void lcdTemp(const String &l1, const String &l2, uint32_t ms) {
  lcdShow(l1, l2);
  lcdTempUntil = millis() + ms;
}

static void lcdStatusScreen() {
  static uint32_t lastDraw = 0;
  if (millis() - lastDraw < 250) return;   // redraw at most 4 times a second
  lastDraw = millis();
  if (millis() < lcdTempUntil) return;
  if (mstate == M_WAIT_REMOVE) { lcdShow("Rejected!", "Take item out"); return; }
  if (binIsFull()) { lcdShow("BIN FULL", "Please call admin"); return; }
  if (drop.active) {
    Phone &c = clients[drop.slot];
    String who = c.user[0] ? String("Hi ") + c.user : String("Hi Guest");
    int left = DROP_IDLE_TIMEOUT_S - (int)((millis() - drop.lastActivityMs) / 1000);
    lcdShow(who, "B:" + String(drop.accepted) + " +" + String(drop.accepted * cfg.minPerBottle) + "m  " + String(max(left, 0)) + "s");
    return;
  }
  // idle: alternate two screens every 3 s
  if ((millis() / 3000) % 2) lcdShow("Join WiFi:", PORTAL_WIFI_NAME);
  else lcdShow("BottleKonek " MACHINE_CODE, "1 bottle=" + String(cfg.minPerBottle) + " min");
}

// ---------------------------------------------------------------------------
//  LEDs + buzzer
// ---------------------------------------------------------------------------
static uint32_t redUntil = 0;
static int      beepsLeft = 0;
static uint32_t beepNext = 0;
static bool     beepOn = false;

void beep(int times) { beepsLeft = times; beepNext = millis(); beepOn = false; }

static void outputsLoop() {
  uint32_t now = millis();
  bool red = now < redUntil || binIsFull() || mstate == M_WAIT_REMOVE;
  if (mstate == M_WAIT_REMOVE) red = (now / 250) % 2;          // flash red while the rejected item is in the chute
  bool yellow = !red && mstate != M_WAIT_REMOVE && (drop.active || mstate != M_IDLE);
  bool green = !red && !yellow && mstate != M_WAIT_REMOVE;
  digitalWrite(PIN_LED_RED, red);
  digitalWrite(PIN_LED_YELLOW, yellow);
  digitalWrite(PIN_LED_GREEN, green);

  if (beepsLeft > 0 && (int32_t)(now - beepNext) >= 0) {
    beepOn = !beepOn;
    digitalWrite(PIN_BUZZER, beepOn);
    if (!beepOn) beepsLeft--;
    beepNext = now + 120;
  }
}

// ---------------------------------------------------------------------------
//  Sensors
// ---------------------------------------------------------------------------
static bool irItem()     { return digitalRead(PIN_IR) == LOW; }
static bool metalNow()   { return digitalRead(PIN_METAL) == (METAL_ACTIVE_HIGH ? HIGH : LOW); }
static bool plasticNow() { return PIN_PLASTIC >= 0 && digitalRead(PIN_PLASTIC) == HIGH; }

// Distance in cm from the ultrasonic sensor, 0 if no echo.
static float readCm() {
  digitalWrite(PIN_US_TRIG, LOW);  delayMicroseconds(3);
  digitalWrite(PIN_US_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);
  unsigned long us = pulseIn(PIN_US_ECHO, HIGH, 12000UL);   // up to ~2 m
  return us ? us / 58.0f : 0;
}

static void calibrateDrop() {
  float v[5]; int n = 0;
  for (int i = 0; i < 7 && n < 5; i++) { float d = readCm(); if (d > 0) v[n++] = d; delay(40); }
  if (n == 0) { baselineCm = 0; Serial.println("Ultrasonic: no echo! Check TRIG->D33, ECHO->D32 (divider), VCC 5V, GND"); return; }
  for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (v[j] < v[i]) { float t = v[i]; v[i] = v[j]; v[j] = t; }
  baselineCm = v[n / 2];
  Serial.printf("Ultrasonic: empty drop path = %.1f cm (a bottle must make it < %.1f cm)\n", baselineCm, baselineCm - DROP_DELTA_CM);
}

// True when something is passing in front of the ultrasonic sensor.
static bool bottlePassing() {
  float d = readCm();
  if (baselineCm <= 0) return d > 0 && d < 30;                 // no baseline: anything closer than 30 cm
  return d > 0 && d < baselineCm - DROP_DELTA_CM;
}

// ---------------------------------------------------------------------------
//  Drop sessions
// ---------------------------------------------------------------------------
static void addEvent(bool ok, const char *text) {
  for (int i = 5; i > 0; i--) drop.ev[i] = drop.ev[i - 1];
  drop.ev[0].ok = ok;
  strlcpy(drop.ev[0].text, text, sizeof(drop.ev[0].text));
  if (drop.evCount < 6) drop.evCount++;
}

bool machineBusy() { return drop.active; }

bool dropStart(int slot) {
  if (slot < 0 || binIsFull()) return false;
  if (drop.active && drop.slot != slot) return false;
  if (!drop.active) {
    memset(&drop, 0, sizeof(drop));
    drop.active = true;
    drop.slot = slot;
    memcpy(drop.mac, clients[slot].mac, 6);
  }
  drop.lastActivityMs = millis();
  beep(1);
  Serial.printf("Drop session started by %s\n", clients[slot].user[0] ? clients[slot].user : "guest");
  return true;
}

// useNow: 1 = start internet now, 0 = keep as balance, -1 = automatic (timeout / left)
void dropEnd(const char *why, int useNow) {
  if (!drop.active) return;
  drop.active = false;
  int s = drop.slot;
  bool sameOwner = s >= 0 && clients[s].used && memcmp(clients[s].mac, drop.mac, 6) == 0;
  int minutes = drop.accepted * cfg.minPerBottle;
  if (sameOwner && (drop.accepted || drop.rejected)) {
    Phone &c = clients[s];
    if (c.user[0]) {
      logHistory(c.user, "D," + String(nowEpoch()) + "," + String(drop.accepted) + "," +
                         String(drop.rejected) + "," + String(minutes));
      addUserTotals(c.user, drop.accepted, minutes);
      if (useNow == 1 && c.secs > 0) setOnline(s, true);      // account chose "Use now"
      saveClientAccount(s);                                   // otherwise it stays as balance
    } else if (c.secs > 0 && useNow != 0) {
      setOnline(s, true);                                     // guest minutes cannot be saved
    }
  }
  saveStats();
  Serial.printf("Drop session ended (%s): %d accepted, %d rejected\n", why, drop.accepted, drop.rejected);
  if (drop.accepted) lcdTemp("Thank you!", "+" + String(minutes) + " min total", 3000);
}

static void creditBottle() {
  stats.totalBottles++;
  stats.todayBottles++;
  stats.binCount++;
  if (binIsFull()) { Serial.println("BIN FULL"); beep(3); }
  if (!drop.active) return;
  drop.accepted++;
  drop.lastActivityMs = millis();
  int s = drop.slot;
  if (s >= 0 && clients[s].used && memcmp(clients[s].mac, drop.mac, 6) == 0) {
    Phone &c = clients[s];
    c.secs += cfg.minPerBottle * 60;
    c.visitBottles++;
    c.visitMinutes += cfg.minPerBottle;
    if (c.user[0]) saveClientAccount(s);
    if (c.online) filterSync();
  }
  addEvent(true, "Plastic bottle accepted");
  lcdTemp("Accepted!", "+" + String(cfg.minPerBottle) + " min", 1500);
  beep(1);
}

// Gate stays closed; student must take the item back out.
static void rejectItem(const char *phoneText, const char *lcd1, const char *lcd2) {
  stats.todayRejects++;
  if (drop.active) { drop.rejected++; addEvent(false, phoneText); drop.lastActivityMs = millis(); }
  lcdTemp(lcd1, lcd2, 2500);
  beep(2);
  gateClose();
  mstate = M_WAIT_REMOVE; mT = millis();
}

// ---------------------------------------------------------------------------
void machineBegin() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_METAL, INPUT);                 // GPIO35 has no internal pull-up: use the 10k/3.3k divider
  pinMode(PIN_IR, INPUT_PULLUP);
  if (PIN_PLASTIC >= 0) pinMode(PIN_PLASTIC, INPUT);
  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);

  // find the LCD's I2C board (usually 0x27, some are 0x3F)
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  uint8_t addr = 0;
  const uint8_t tryAddr[] = { 0x27, 0x3F, 0x20, 0x38 };
  for (uint8_t a : tryAddr) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { addr = a; break; }
  }
  if (addr) {
    lcd = new LiquidCrystal_I2C(addr, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->clear();
    Serial.printf("LCD found at I2C address 0x%02X\n", addr);
  } else {
    Serial.println("LCD not found: check SDA->D21, SCL->D22, VCC->5V, GND");
  }

  // Start-up self-test: red, yellow, green LED one by one, then one beep.
  lcdShow("BottleKonek", "Self-test...");
  Serial.println("Self-test: red, yellow, green LED, then one beep");
  const int leds[] = { PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN };
  for (int p : leds) { digitalWrite(p, HIGH); delay(400); digitalWrite(p, LOW); }
  digitalWrite(PIN_BUZZER, HIGH); delay(200); digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);

  ledcAttach(PIN_SERVO_GATE, 50, 14);
  gateClose();
  calibrateDrop();                           // keep the drop path empty while starting up
}

void machineLoop() {
  uint32_t now = millis();

  // IR debounce
  if (irItem()) { irClearSince = 0; if (!irSince) irSince = now; }
  else          { irSince = 0; if (!irClearSince) irClearSince = now; }
  if (!armed && irClearSince && now - irClearSince > 400) armed = true;

  switch (mstate) {
    case M_IDLE:
      if (armed && irSince && now - irSince >= ITEM_DEBOUNCE_MS) {
        armed = false;
        metalSeen = plasticSeen = false;
        if (drop.active) drop.lastActivityMs = now;
        lcdTemp("Checking...", "Please wait", 1500);
        mstate = M_CHECK; mT = now;
      }
      break;

    case M_CHECK:
      metalSeen |= metalNow();
      plasticSeen |= plasticNow();
      if (now - mT >= CHECK_TIME_MS) {
        if (!drop.active)                           rejectItem("", "Open the portal", "on your phone");
        else if (binIsFull())                       rejectItem("Rejected: bin is full", "Bin full", "Take item out");
        else if (metalSeen)                         rejectItem("Rejected: metal can or foil", "Metal not allowed", "Take item out");
        else if (PIN_PLASTIC >= 0 && !plasticSeen)  rejectItem("Rejected: plastic bottles only", "Plastic only", "Take item out");
        else { gateOpen(); mstate = M_WAIT_DROP; mT = now; lastUs = 0; }
      }
      break;

    case M_WAIT_DROP:                              // ultrasonic must see the bottle fall
      if (now - lastUs >= 40) {
        lastUs = now;
        if (bottlePassing()) {
          creditBottle();
          mstate = M_CLOSING; mT = now;
          break;
        }
      }
      if (now - mT > DROP_WAIT_MS) {
        if (drop.active) addEvent(false, "Drop not detected, try again");
        lcdTemp("Not detected", "Try again", 2500);
        redUntil = now + 1500;
        beep(3);
        gateClose();
        mstate = irItem() ? M_WAIT_REMOVE : M_IDLE;
        mT = now;
      }
      break;

    case M_CLOSING:
      if (now - mT >= GATE_CLOSE_DELAY) { gateClose(); mstate = M_IDLE; }
      break;

    case M_WAIT_REMOVE:                            // rejected item still at the gate
      if (irClearSince && now - irClearSince > 500) { mstate = M_IDLE; armed = true; }
      break;
  }

  // drop session timeout
  if (drop.active && mstate == M_IDLE && now - drop.lastActivityMs > DROP_IDLE_TIMEOUT_S * 1000UL)
    dropEnd("no bottle for 30 s", -1);

  outputsLoop();
  lcdStatusScreen();
}
