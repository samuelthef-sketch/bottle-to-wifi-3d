// ============================================================================
//  BottleKonek HARDWARE TEST  (no WiFi, no website)
//  Upload this, open Serial Monitor at 115200, and watch the parts.
//  When everything passes, upload BottleKonek.ino again.
// ============================================================================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22
#define PIN_LED_GREEN    4
#define PIN_LED_YELLOW  16
#define PIN_LED_RED     17
#define PIN_BUZZER      14
#define PIN_BUTTON      15

LiquidCrystal_I2C *lcd = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n\n=== BottleKonek hardware test ===");

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // 1. I2C scan: is the LCD's I2C board answering?
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.println("[1] Looking for the LCD on SDA=D21, SCL=D22 ...");
  uint8_t found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("    found a device at 0x%02X\n", a);
      if (!found) found = a;
    }
  }
  if (found) {
    lcd = new LiquidCrystal_I2C(found, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->setCursor(0, 0); lcd->print("BottleKonek");
    lcd->setCursor(0, 1); lcd->print("LCD test OK");
    Serial.println("    LCD OK: you should see 'LCD test OK'.");
    Serial.println("    Backlight on but no letters? Turn the blue screw on the I2C board.");
  } else {
    Serial.println("    NOTHING FOUND. Check: VCC->5V, GND->GND, SDA->D21, SCL->D22.");
    Serial.println("    SDA and SCL swapped is the most common mistake.");
  }
}

void loop() {
  struct { int pin; const char *name; } leds[] = {
    { PIN_LED_GREEN, "GREEN (D4)" }, { PIN_LED_YELLOW, "YELLOW (D16)" }, { PIN_LED_RED, "RED (D17)" } };

  for (auto &l : leds) {
    Serial.printf("[2] %s LED should be ON now\n", l.name);
    if (lcd) { lcd->setCursor(0, 1); lcd->print("LED ");  lcd->print(l.name); lcd->print("    "); }
    digitalWrite(l.pin, HIGH); delay(1000); digitalWrite(l.pin, LOW);
  }

  Serial.println("[3] BUZZER (D14) should beep now");
  if (lcd) { lcd->setCursor(0, 1); lcd->print("Buzzer beep     "); }
  digitalWrite(PIN_BUZZER, HIGH); delay(300); digitalWrite(PIN_BUZZER, LOW);

  Serial.printf("[4] Button (D15) is %s\n", digitalRead(PIN_BUTTON) == LOW ? "PRESSED" : "not pressed");
  Serial.println("--- repeating in 2 s ---");
  delay(2000);
}
