// ============================================================================
//  BottleKonek: plastic bottles -> WiFi minutes
//  Bongabon Senior High School - Science Innovation Expo
//
//  Board:  ESP32 Dev Module   (Arduino core "esp32 by Espressif" version 3.x)
//  Partition scheme: "Default 4MB with spiffs"  (LittleFS uses that space)
//
//  The ESP32 does everything:
//    - joins the MAIN WiFi (name + password in config.h) to get internet
//    - broadcasts "BottleKonek WiFi" for students, with a sign-in portal
//    - passes internet only to phones that still have minutes
//    - runs the bottle machine (sensors, servos, LCD, LEDs, buzzer)
//
//  Edit config.h first. Tabs: network, accounts, machine, web.
// ============================================================================
#include "config.h"
#include "types.h"
#include <WiFi.h>

Phone       clients[MAX_CLIENTS];
DropSession drop;
Settings    cfg;
Stats       stats;
int         binPct = 0;

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\nBottleKonek starting...");

  memset(clients, 0, sizeof(clients));
  memset(&drop, 0, sizeof(drop));

  machineBegin();          // LCD + LEDs first so people see something
  lcdShow("BottleKonek", "Starting...");
  storageBegin();          // LittleFS + settings + stats
  networkBegin();          // main WiFi + hotspot + internet sharing + DNS
  webBegin();              // portal website
  lcdShow("BottleKonek", "Code " MACHINE_CODE " Ready");
  Serial.println("Ready. Join \"" PORTAL_WIFI_NAME "\" and open http://192.168.4.1");
}

void loop() {
  networkLoop();           // DNS relay + phone connect/disconnect events
  webLoop();               // HTTP requests
  machineLoop();           // bottle checking state machine

  static uint32_t lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick += 1000;
    tickClients();
  }
}

// ============================================================================
//  Phones (clients)
// ============================================================================
int findClientByMac(const uint8_t *mac) {
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (clients[i].used && memcmp(clients[i].mac, mac, 6) == 0) return i;
  return -1;
}

int findClientByIp(uint32_t ip) {
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (clients[i].used && clients[i].connected && clients[i].ip == ip) return i;
  return -1;
}

// Which slot made this HTTP request? Falls back to the packet filter's IP->MAC map.
int clientForRequestIp(uint32_t ip) {
  int s = findClientByIp(ip);
  if (s >= 0) return s;
  uint8_t mac[6];
  if (ipToMac(ip, mac)) {
    s = findClientByMac(mac);
    if (s < 0) { onPhoneConnected(mac); s = findClientByMac(mac); }
    if (s >= 0) clients[s].ip = ip;
  }
  return s;
}

static int newSlot() {
  for (int i = 0; i < MAX_CLIENTS; i++) if (!clients[i].used) return i;
  // table full: reuse the oldest disconnected guest
  for (int i = 0; i < MAX_CLIENTS; i++) if (!clients[i].connected) return i;
  return -1;
}

void onPhoneConnected(const uint8_t *mac) {
  int s = findClientByMac(mac);
  if (s >= 0) {                       // guest coming back inside the grace period
    clients[s].connected = true;
    clients[s].goneAtMs = 0;
    Serial.printf("Phone back: %s\n", macHex(mac).c_str());
    filterSync();
    return;
  }
  s = newSlot();
  if (s < 0) return;
  Phone &c = clients[s];
  memset(&c, 0, sizeof(c));
  c.used = true;
  memcpy(c.mac, mac, 6);
  c.connected = true;
  c.ip = macToIpLookup(mac);

  String u = rememberedUser(mac);     // "Remember this phone" -> log in automatically
  if (u.length()) {
    int32_t secs; uint32_t b, m;
    if (loadUserNumbers(u, secs, b, m)) {
      // only one phone per account
      for (int i = 0; i < MAX_CLIENTS; i++)
        if (i != s && clients[i].used && u.equals(clients[i].user)) unbindUser(i, false);
      strlcpy(c.user, u.c_str(), sizeof(c.user));
      c.secs = secs;
      c.remembered = true;
    }
  }
  Serial.printf("Phone joined: %s %s\n", macHex(mac).c_str(), c.user);
}

void onPhoneDisconnected(const uint8_t *mac) {
  int s = findClientByMac(mac);
  if (s < 0) return;
  Phone &c = clients[s];
  if (drop.active && drop.slot == s) dropEnd("owner left");
  if (c.user[0]) {                    // account: pause and save, free the slot
    if (c.online) setOnline(s, false);
    saveClientAccount(s);
    Serial.printf("Account %s paused (left WiFi), %d s saved\n", c.user, c.secs);
    memset(&c, 0, sizeof(c));
  } else {                            // guest: keep time for the grace period
    c.connected = false;
    c.goneAtMs = millis();
    if (c.goneAtMs == 0) c.goneAtMs = 1;
    Serial.printf("Guest left: %s (grace %d s)\n", macHex(mac).c_str(), cfg.guestGraceS);
  }
  filterSync();
}

void setOnline(int s, bool on) {
  Phone &c = clients[s];
  if (on && c.secs <= 0) on = false;
  if (on == c.online) return;
  if (on) {
    c.online = true;
    c.onlineSinceMs = millis();
    c.secsAtStart = c.secs;
  } else {
    c.online = false;
    int usedMin = (c.secsAtStart - c.secs) / 60;
    if (c.user[0] && usedMin >= 1)
      logHistory(c.user, "U," + String(nowEpoch()) + "," + String(usedMin));
  }
  filterSync();
}

void saveClientAccount(int s) {
  Phone &c = clients[s];
  if (!c.user[0]) return;
  saveUserSecs(c.user, c.secs);
  c.lastSaveMs = millis();
}

// Log out of the account on this phone (forget = also stop auto login).
void unbindUser(int s, bool forget) {
  Phone &c = clients[s];
  if (!c.user[0]) return;
  if (c.online) setOnline(s, false);
  saveClientAccount(s);
  if (forget) forgetDevice(c.mac);
  c.user[0] = 0;
  c.remembered = false;
  c.secs = 0;
  c.visitBottles = c.visitMinutes = 0;
  filterSync();
}

// Runs once per second.
void tickClients() {
  uint32_t now = millis();
  for (int i = 0; i < MAX_CLIENTS; i++) {
    Phone &c = clients[i];
    if (!c.used) continue;

    if (!c.connected) {
      // guest outside the grace period: time is gone
      if (!c.user[0] && c.goneAtMs && now - c.goneAtMs >= (uint32_t)cfg.guestGraceS * 1000UL) {
        Serial.printf("Guest %s gone > %d s, balance reset to 0\n", macHex(c.mac).c_str(), cfg.guestGraceS);
        memset(&c, 0, sizeof(c));
        filterSync();
      }
      continue;
    }

    if (c.online) {
      c.secs--;
      if (c.secs <= 0) {
        c.secs = 0;
        setOnline(i, false);          // time's up: portal comes back
        saveClientAccount(i);
      } else if (c.user[0] && now - c.lastSaveMs >= 60000UL) {
        saveClientAccount(i);         // power cut loses at most a minute
      }
    }
  }
  static uint32_t lastStatsSave = 0;
  if (now - lastStatsSave > 300000UL) { lastStatsSave = now; rollDay(); saveStats(); }
}
