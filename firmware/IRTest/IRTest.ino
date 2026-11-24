// ============================================================================
//  BottleKonek IR SENSOR TEST (no WiFi, no website)
//  Wiring: VCC -> 3V3 (or a V pin = 5V), GND -> G, OUT -> D27 (S pin)
//  Open Serial Monitor at 115200. Move your finger 2-10 cm in front of it.
// ============================================================================
#define PIN_IR 27

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_IR, INPUT_PULLUP);
  Serial.println("\n=== IR test: watch the value change when your finger is near ===");
}

void loop() {
  static int last = -1;
  static uint32_t changes = 0;
  int v = digitalRead(PIN_IR);
  if (v != last) { changes++; last = v; }
  static uint32_t t = 0;
  if (millis() - t > 300) {
    t = millis();
    Serial.printf("OUT = %s   (%s)   changes so far: %u\n",
                  v ? "HIGH" : "LOW",
                  v ? "usually: nothing in front" : "usually: SOMETHING in front",
                  (unsigned)changes);
  }
}
