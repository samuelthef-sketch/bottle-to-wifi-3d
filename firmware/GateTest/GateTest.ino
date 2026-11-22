// ============================================================================
//  BottleKonek GATE TEST for the 360-degree MG90S servo (no WiFi, no website)
//  Opens the gate, waits, closes it, waits, repeats.
//  Tune the 4 numbers below until the gate opens enough and closes exactly,
//  then copy them into config.h of the main BottleKonek code.
//  Before powering on: turn the gate by hand to the CLOSED position.
// ============================================================================
#define PIN_SERVO_GATE  25
#define GATE_OPEN_MS    300   // spin time to open  (bigger = opens wider)
#define GATE_CLOSE_MS   300   // spin time to close (adjust if it drifts open or closed)
#define GATE_SPEED_US   250   // 100 = slow, 400 = fast
#define GATE_OPEN_DIR   1     // 1 or -1: flip if it opens the wrong way

void spin(int dir, int ms) {
  ledcWrite(PIN_SERVO_GATE, (1500 + dir * GATE_SPEED_US) * 16383UL / 20000UL);
  delay(ms);
  ledcWrite(PIN_SERVO_GATE, 0);          // no signal = stop
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  ledcAttach(PIN_SERVO_GATE, 50, 14);
  ledcWrite(PIN_SERVO_GATE, 0);
  Serial.println("Gate test: open 3 s, closed 3 s, repeating");
}

void loop() {
  Serial.println("OPEN");
  spin(GATE_OPEN_DIR, GATE_OPEN_MS);
  delay(3000);
  Serial.println("CLOSE");
  spin(-GATE_OPEN_DIR, GATE_CLOSE_MS);
  delay(3000);
}
