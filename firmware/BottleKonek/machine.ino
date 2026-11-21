// ============================================================================
//  machine: bottle checking, servos, LCD, traffic light LEDs, buzzer, bin level
//
//  One item at a time:
//   IR sees item on tray -> gate closes -> metal sensors watched for 0.8 s
//   metal (or no drop session) -> tray tilts to return slot, red LED, 2 beeps
//   ok -> tray tilts to bin, IR must clear within 2 s -> +minutes, 1 beep
// ============================================================================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C *lcd = nullptr;   // created in machineBegin() once the address is known

enum MState { M_IDLE, M_CHECK, M_TO_BIN, M_BACK, M_RETURN, M_RETURN_WAIT };
static MState   mstate = M_IDLE;
static uint32_t mT = 0;              // time the current state started
static uint32_t irSince = 0;         // IR has seen an item since
static uint32_t irClearSince = 0;
static bool     armed = true;        // tray must be clear before the next item counts
static bool     metalSeen = false, plasticSeen = false;
static bool     binFull = false;

// ---------------------------------------------------------------------------
//  Servos (LEDC PWM, 50 Hz). MG90S: 500-2400 us
// ---------------------------------------------------------------------------
static void servoWrite(int pin, int deg) {
  deg = constrain(deg, 0, 180);
  uint32_t us = 500 + (uint32_t)deg * 1900 / 180;
  ledcWrite(pin, us * 16383UL / 20000UL);
}

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
  if (binFull) { lcdShow("BIN FULL", "Please call admin"); return; }
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
  bool red = now < redUntil || binFull;
  bool yellow = !red && (drop.active || mstate != M_IDLE);
  bool green = !red && !yellow;
  digitalWrite(PIN_LED_RED, red);
  digitalWrite(PIN_LED_YELLOW, yellow);
  digitalWrite(PIN_LED_GREEN, green);

  if (beepsLeft > 0 && (int32_t)(now - beepNext) >= 0) {
    beepOn = !beepOn;
    digitalWrite(PIN_BUZZER, beepOn);
    if (!beepOn) beepsLeft--;
    beepNext = now + (beepOn ? 120 : 120);
  }
}

// ---------------------------------------------------------------------------
//  Sensors
// ---------------------------------------------------------------------------
static bool irItem()   { return digitalRead(PIN_IR) == LOW; }
static bool metalNow() { return digitalRead(PIN_METAL_BX) == LOW || digitalRead(PIN_METAL_BY) == HIGH; }
static bool plasticNow() { return PIN_PLASTIC >= 0 && digitalRead(PIN_PLASTIC) == HIGH; }

static void readBin() {
  digitalWrite(PIN_US_TRIG, LOW);  delayMicroseconds(3);
  digitalWrite(PIN_US_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);
  unsigned long us = pulseIn(PIN_US_ECHO, HIGH, 30000UL);
  if (us == 0) return;                       // no echo, keep the last value
  float cm = us / 58.0f;
  int span = max(1, cfg.binEmptyCm - cfg.binFullCm);
  int pct = (int)((cfg.binEmptyCm - cm) * 100.0f / span);
  pct = constrain(pct, 0, 100);
  binPct = (binPct * 2 + pct) / 3;           // smooth out jumps
  bool wasFull = binFull;
  binFull = binPct >= cfg.binFullPct;
  if (binFull && !wasFull) { Serial.println("BIN FULL"); beep(3); }
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
  if (slot < 0 || binFull) return false;
  if (drop.active && drop.slot != slot) return false;
  if (!drop.active) {
    memset(&drop, 0, sizeof(drop));
    drop.active = true;
    drop.slot = slot;
    memcpy(drop.mac, clients[slot].mac, 6);
  }
  drop.lastActivityMs = millis();
  servoWrite(PIN_SERVO_GATE, GATE_OPEN_DEG);
  beep(1);
  Serial.printf("Drop session started by %s\n", clients[slot].user[0] ? clients[slot].user : "guest");
  return true;
}

void dropEnd(const char *why) {
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
      saveClientAccount(s);
    } else if (c.secs > 0) {
      setOnline(s, true);                    // guests go online right away
    }
  }
  saveStats();
  Serial.printf("Drop session ended (%s): %d accepted, %d rejected\n", why, drop.accepted, drop.rejected);
  if (drop.accepted) lcdTemp("Thank you!", "+" + String(minutes) + " min total", 3000);
}

static void creditBottle() {
  stats.totalBottles++;
  stats.todayBottles++;
  if (!drop.active) return;
  drop.accepted++;
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

static void rejectItem(const char *phoneText, const char *lcd1, const char *lcd2) {
  stats.todayRejects++;
  if (drop.active) { drop.rejected++; addEvent(false, phoneText); }
  lcdTemp(lcd1, lcd2, 2500);
  redUntil = millis() + 1500;
  beep(2);
  servoWrite(PIN_SERVO_TRAY, TRAY_RETURN_DEG);
  mstate = M_RETURN; mT = millis();
}

// ---------------------------------------------------------------------------
void machineBegin() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_METAL_BX, INPUT);              // GPIO34/35 have no internal pull-ups: use the resistors
  pinMode(PIN_METAL_BY, INPUT);
  pinMode(PIN_IR, INPUT_PULLUP);
  if (PIN_PLASTIC >= 0) pinMode(PIN_PLASTIC, INPUT);
  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);
  digitalWrite(PIN_LED_GREEN, HIGH);

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

  ledcAttach(PIN_SERVO_GATE, 50, 14);
  ledcAttach(PIN_SERVO_TRAY, 50, 14);
  servoWrite(PIN_SERVO_GATE, GATE_OPEN_DEG);
  servoWrite(PIN_SERVO_TRAY, TRAY_HOLD_DEG);
}

void machineLoop() {
  uint32_t now = millis();

  // IR debounce
  if (irItem()) { irClearSince = 0; if (!irSince) irSince = now; }
  else          { irSince = 0; if (!irClearSince) irClearSince = now; }
  if (!armed && irClearSince && now - irClearSince > 300) armed = true;

  switch (mstate) {
    case M_IDLE:
      if (armed && irSince && now - irSince >= ITEM_DEBOUNCE_MS) {
        armed = false;
        servoWrite(PIN_SERVO_GATE, GATE_CLOSED_DEG);
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
        if (!drop.active)                         rejectItem("", "Open the portal", "on your phone");
        else if (binFull)                         rejectItem("Rejected: bin is full", "Bin full", "Item returned");
        else if (metalSeen)                       rejectItem("Rejected: metal can or foil", "Metal not allowed", "Item returned");
        else if (PIN_PLASTIC >= 0 && !plasticSeen) rejectItem("Rejected: plastic bottles only", "Plastic only", "Item returned");
        else { servoWrite(PIN_SERVO_TRAY, TRAY_BIN_DEG); mstate = M_TO_BIN; mT = now; }
      }
      break;

    case M_TO_BIN:                                // IR must clear = bottle fell into the bin
      if (irClearSince && now - irClearSince > 100) {
        creditBottle();
        mstate = M_BACK; mT = now;
      } else if (now - mT > DROP_CONFIRM_MS) {
        if (drop.active) addEvent(false, "Bottle stuck, no minutes added");
        lcdTemp("Bottle stuck", "Try again", 2500);
        redUntil = now + 1500;
        beep(3);
        mstate = M_BACK; mT = now;
      }
      break;

    case M_BACK:
      if (now - mT >= TRAY_MOVE_MS) {
        servoWrite(PIN_SERVO_TRAY, TRAY_HOLD_DEG);
        servoWrite(PIN_SERVO_GATE, GATE_OPEN_DEG);
        if (drop.active) drop.lastActivityMs = now;
        mstate = M_IDLE;
      }
      break;

    case M_RETURN:
      if (now - mT >= TRAY_MOVE_MS * 2) {
        servoWrite(PIN_SERVO_TRAY, TRAY_HOLD_DEG);
        mstate = M_RETURN_WAIT; mT = now;
      }
      break;

    case M_RETURN_WAIT:
      if ((irClearSince && now - irClearSince > 300) || now - mT > 3000) {
        servoWrite(PIN_SERVO_GATE, GATE_OPEN_DEG);
        if (drop.active) drop.lastActivityMs = now;
        mstate = M_IDLE;
      }
      break;
  }

  // drop session timeout + button
  if (drop.active && mstate == M_IDLE && now - drop.lastActivityMs > DROP_IDLE_TIMEOUT_S * 1000UL)
    dropEnd("no bottle for 30 s");
  static uint32_t btnSince = 0;
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (!btnSince) btnSince = now;
    else if (btnSince != 1 && now - btnSince > 60) { btnSince = 1; dropEnd("button"); }
  } else btnSince = 0;

  // bin level every 3 s while nothing is moving
  static uint32_t lastBin = 0;
  if (mstate == M_IDLE && now - lastBin > 3000) { lastBin = now; readBin(); }

  outputsLoop();
  lcdStatusScreen();
}
