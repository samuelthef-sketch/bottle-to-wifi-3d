// ============================================================================
//  accounts: usernames, passwords, balances and history, saved in ESP32 flash
//
//  Files (LittleFS):
//    /u/<username>   account: password hash, salt, balance, totals
//    /h/<username>   history lines  D,<time>,<accepted>,<rejected>,<minutes>
//                                   U,<time>,<minutes used>
//    /d/<MAC>        remembered phone: <username>,<expires>
// ============================================================================
#include <LittleFS.h>
#include <Preferences.h>
#include <time.h>
#include "mbedtls/sha256.h"

static Preferences prefs;

void storageBegin() {
  if (!LittleFS.begin(true)) Serial.println("LittleFS mount failed (check partition scheme)");
  if (!LittleFS.exists("/u")) LittleFS.mkdir("/u");
  if (!LittleFS.exists("/h")) LittleFS.mkdir("/h");
  if (!LittleFS.exists("/d")) LittleFS.mkdir("/d");
  loadSettings();
  loadStats();
}

uint32_t nowEpoch() {
  time_t t = time(nullptr);
  return t > 1700000000 ? (uint32_t)t : 0;   // 0 = clock not set yet (no internet)
}

// ---------------------------------------------------------------------------
bool validUsername(const String &u) {
  if (u.length() < 3 || u.length() > USER_MAX) return false;
  for (size_t i = 0; i < u.length(); i++) {
    char c = u[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '_')) return false;
  }
  return true;
}

static String userPath(const String &u) { return "/u/" + u; }
static String histPath(const String &u) { return "/h/" + u; }

bool userExists(const String &u) { return validUsername(u) && LittleFS.exists(userPath(u)); }

static String hashPass(const String &salt, const String &pass) {
  String in = salt + ":" + pass;
  uint8_t out[32];
  mbedtls_sha256((const unsigned char *)in.c_str(), in.length(), out, 0);
  char hex[65];
  for (int i = 0; i < 32; i++) sprintf(hex + i * 2, "%02x", out[i]);
  return String(hex);
}

// Simple key=value file (UserRec is in types.h)

static bool readUser(const String &u, UserRec &r) {
  File f = LittleFS.open(userPath(u), "r");
  if (!f) return false;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() < 2 || line[1] != '=') continue;
    String v = line.substring(2);
    switch (line[0]) {
      case 'h': r.h = v; break;
      case 's': r.s = v; break;
      case 'b': r.b = v.toInt(); break;
      case 't': r.t = v.toInt(); break;
      case 'm': r.m = v.toInt(); break;
      case 'c': r.c = v.toInt(); break;
    }
  }
  f.close();
  return r.h.length() > 0;
}

static bool writeUser(const String &u, const UserRec &r) {
  String tmp = userPath(u) + ".tmp";
  File f = LittleFS.open(tmp, "w");
  if (!f) return false;
  f.printf("h=%s\ns=%s\nb=%d\nt=%u\nm=%u\nc=%u\n", r.h.c_str(), r.s.c_str(), (int)r.b,
           (unsigned)r.t, (unsigned)r.m, (unsigned)r.c);
  f.close();
  LittleFS.remove(userPath(u));
  return LittleFS.rename(tmp, userPath(u));
}

bool createUser(const String &u, const String &p, int32_t startSecs) {
  if (!validUsername(u) || userExists(u) || p.length() < 4) return false;
  UserRec r;
  char salt[17];
  snprintf(salt, sizeof(salt), "%08x%08x", (unsigned)esp_random(), (unsigned)esp_random());
  r.s = salt;
  r.h = hashPass(r.s, p);
  r.b = startSecs;
  r.c = nowEpoch();
  return writeUser(u, r);
}

bool checkPassword(const String &u, const String &p) {
  UserRec r;
  if (!userExists(u) || !readUser(u, r)) return false;
  return hashPass(r.s, p) == r.h;
}

bool setPassword(const String &u, const String &p) {
  UserRec r;
  if (p.length() < 4 || !readUser(u, r)) return false;
  r.h = hashPass(r.s, p);
  return writeUser(u, r);
}

bool loadUserNumbers(const String &u, int32_t &secs, uint32_t &bottles, uint32_t &minutes) {
  UserRec r;
  if (!userExists(u) || !readUser(u, r)) return false;
  secs = r.b; bottles = r.t; minutes = r.m;
  return true;
}

bool saveUserSecs(const String &u, int32_t secs) {
  UserRec r;
  if (!readUser(u, r)) return false;
  if (r.b == secs) return true;
  r.b = secs;
  return writeUser(u, r);
}

bool addUserTotals(const String &u, uint32_t bottles, uint32_t minutes) {
  UserRec r;
  if (!readUser(u, r)) return false;
  r.t += bottles; r.m += minutes;
  return writeUser(u, r);
}

bool deleteUser(const String &u) {
  if (!userExists(u)) return false;
  LittleFS.remove(histPath(u));
  return LittleFS.remove(userPath(u));
}

int countUsers() {
  int n = 0;
  File dir = LittleFS.open("/u");
  if (!dir) return 0;
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    String nm = f.name();
    if (!nm.endsWith(".tmp")) n++;
  }
  return n;
}

String usersJson() {
  String out = "[";
  File dir = LittleFS.open("/u");
  if (!dir) return "[]";
  bool first = true;
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    String nm = f.name();
    int slash = nm.lastIndexOf('/');
    if (slash >= 0) nm = nm.substring(slash + 1);
    f.close();
    if (!validUsername(nm)) continue;
    UserRec r;
    if (!readUser(nm, r)) continue;
    // a logged-in phone may hold a newer balance than the file
    int32_t secs = r.b;
    for (int i = 0; i < MAX_CLIENTS; i++) if (clients[i].used && nm.equals(clients[i].user)) secs = clients[i].secs;
    if (!first) out += ",";
    first = false;
    out += "{\"u\":\"" + nm + "\",\"s\":" + String(secs) + ",\"b\":" + String(r.t) + ",\"m\":" + String(r.m) + "}";
  }
  return out + "]";
}

// ---------------------------------------------------------------------------
//  History
// ---------------------------------------------------------------------------
void logHistory(const String &u, const String &line) {
  String path = histPath(u);
  File f = LittleFS.open(path, "a");
  if (!f) return;
  f.println(line);
  size_t size = f.size();
  f.close();
  if (size > 12000) {                       // keep the newest ~6 KB
    File in = LittleFS.open(path, "r");
    in.seek(size - 6000);
    in.readStringUntil('\n');
    String rest = in.readString();
    in.close();
    File o = LittleFS.open(path, "w");
    o.print(rest);
    o.close();
  }
}

// Newest 40 entries as JSON, newest first
String historyJson(const String &u) {
  String lines[40];
  int n = 0, head = 0;
  File f = LittleFS.open(histPath(u), "r");
  if (f) {
    while (f.available()) {
      String l = f.readStringUntil('\n');
      l.trim();
      if (!l.length()) continue;
      lines[head] = l;
      head = (head + 1) % 40;
      if (n < 40) n++;
    }
    f.close();
  }
  String out = "[";
  for (int k = 0; k < n; k++) {
    String l = lines[(head - 1 - k + 80) % 40];
    // D,epoch,acc,rej,min   or   U,epoch,min
    int p1 = l.indexOf(','), p2 = l.indexOf(',', p1 + 1);
    if (p1 < 0) continue;
    String type = l.substring(0, p1);
    if (k) out += ",";
    if (type == "D") {
      int p3 = l.indexOf(',', p2 + 1), p4 = l.indexOf(',', p3 + 1);
      out += "{\"t\":\"D\",\"e\":" + l.substring(p1 + 1, p2) + ",\"a\":" + l.substring(p2 + 1, p3) +
             ",\"r\":" + l.substring(p3 + 1, p4) + ",\"m\":" + l.substring(p4 + 1) + "}";
    } else {
      out += "{\"t\":\"U\",\"e\":" + l.substring(p1 + 1, p2) + ",\"m\":" + l.substring(p2 + 1) + "}";
    }
  }
  return out + "]";
}

// ---------------------------------------------------------------------------
//  Remembered phones
// ---------------------------------------------------------------------------
void rememberDevice(const uint8_t *mac, const String &u) {
  File f = LittleFS.open("/d/" + macHex(mac), "w");
  if (!f) return;
  uint32_t now = nowEpoch();
  f.printf("%s,%u", u.c_str(), (unsigned)(now ? now + REMEMBER_DAYS * 86400UL : 0));
  f.close();
}

void forgetDevice(const uint8_t *mac) { LittleFS.remove("/d/" + macHex(mac)); }

String rememberedUser(const uint8_t *mac) {
  String path = "/d/" + macHex(mac);
  File f = LittleFS.open(path, "r");
  if (!f) return "";
  String s = f.readString();
  f.close();
  int comma = s.indexOf(',');
  if (comma < 0) return "";
  String u = s.substring(0, comma);
  uint32_t exp = s.substring(comma + 1).toInt();
  uint32_t now = nowEpoch();
  if (exp && now && now > exp) { LittleFS.remove(path); return ""; }
  return userExists(u) ? u : "";
}

// ---------------------------------------------------------------------------
//  Settings and stats (Preferences / NVS)
// ---------------------------------------------------------------------------
void loadSettings() {
  prefs.begin("bk", true);
  cfg.minPerBottle = prefs.getInt("mpb", DEFAULT_MIN_PER_BOTTLE);
  cfg.guestGraceS  = prefs.getInt("grace", DEFAULT_GUEST_GRACE_S);
  cfg.binCapacity  = prefs.getInt("bincap", DEFAULT_BIN_CAPACITY);
  cfg.testMode     = prefs.getBool("test", false);
  cfg.gateOpenMs   = prefs.getInt("gopen", GATE_OPEN_MS);
  cfg.gateCloseMs  = prefs.getInt("gclose", GATE_CLOSE_MS);
  cfg.gateSpeed    = prefs.getInt("gspd", GATE_SPEED_US);
  cfg.gateDir      = prefs.getInt("gdir", GATE_OPEN_DIR);
  cfg.servo360     = prefs.getBool("s360", SERVO_360);
  cfg.openDeg      = prefs.getInt("odeg", GATE_OPEN_DEG);
  cfg.closeDeg     = prefs.getInt("cdeg", GATE_CLOSED_DEG);
  prefs.end();
}

void saveSettings() {
  prefs.begin("bk", false);
  prefs.putInt("mpb", cfg.minPerBottle);
  prefs.putInt("grace", cfg.guestGraceS);
  prefs.putInt("bincap", cfg.binCapacity);
  prefs.putBool("test", cfg.testMode);
  prefs.putInt("gopen", cfg.gateOpenMs);
  prefs.putInt("gclose", cfg.gateCloseMs);
  prefs.putInt("gspd", cfg.gateSpeed);
  prefs.putInt("gdir", cfg.gateDir);
  prefs.putBool("s360", cfg.servo360);
  prefs.putInt("odeg", cfg.openDeg);
  prefs.putInt("cdeg", cfg.closeDeg);
  prefs.end();
}

void loadStats() {
  prefs.begin("bks", true);
  stats.totalBottles = prefs.getUInt("total", 0);
  stats.todayBottles = prefs.getUInt("today", 0);
  stats.todayRejects = prefs.getUInt("rej", 0);
  stats.todayYday    = prefs.getInt("yday", -1);
  stats.binCount     = prefs.getUInt("bin", 0);
  prefs.end();
}

void saveStats() {
  prefs.begin("bks", false);
  prefs.putUInt("total", stats.totalBottles);
  prefs.putUInt("today", stats.todayBottles);
  prefs.putUInt("rej", stats.todayRejects);
  prefs.putInt("yday", stats.todayYday);
  prefs.putUInt("bin", stats.binCount);
  prefs.end();
}

// Reset the "today" counters when the date changes.
void rollDay() {
  if (!nowEpoch()) return;
  time_t t = time(nullptr);
  struct tm lt;
  localtime_r(&t, &lt);
  if (lt.tm_yday != stats.todayYday) {
    stats.todayYday = lt.tm_yday;
    stats.todayBottles = 0;
    stats.todayRejects = 0;
    saveStats();
  }
}
