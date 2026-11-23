// ============================================================================
//  BottleKonek ULTRASONIC TEST (no WiFi, no website)
//  Wiring for this test (HC-SR04 "2020" version works on 3.3 V):
//    Vcc  -> ESP32 3V3        Gnd  -> ESP32 G
//    Trig -> D33              Echo -> D32  (direct, no resistors at 3.3 V)
//  Open Serial Monitor at 115200. Wave your hand in front of the sensor.
//  It also tries the pins swapped, in case Trig and Echo are reversed.
// ============================================================================
const int PIN_A = 33, PIN_B = 32;

float measure(int trig, int echo) {
  pinMode(trig, OUTPUT); pinMode(echo, INPUT);
  digitalWrite(trig, LOW);  delayMicroseconds(3);
  digitalWrite(trig, HIGH); delayMicroseconds(12);
  digitalWrite(trig, LOW);
  unsigned long us = pulseIn(echo, HIGH, 30000UL);
  return us ? us / 58.0f : 0;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Ultrasonic test ===");
}

void loop() {
  float normal  = measure(PIN_A, PIN_B);   // Trig D33, Echo D32 (correct)
  delay(60);
  float swapped = measure(PIN_B, PIN_A);   // Trig D32, Echo D33 (if wires are reversed)
  Serial.print("Trig D33/Echo D32: ");
  if (normal > 0) Serial.printf("%.1f cm", normal); else Serial.print("no echo");
  Serial.print("    |    swapped: ");
  if (swapped > 0) Serial.printf("%.1f cm", swapped); else Serial.print("no echo");
  Serial.println();
  delay(400);
}
