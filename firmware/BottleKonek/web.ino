// ============================================================================
//  web: portal website, captive-portal redirects, student API, admin API
// ============================================================================
#include <WebServer.h>
#include "portal_html.h"
#include "admin_html.h"
#include "logo_jpg.h"

static WebServer server(80);
static String adminToken;

// ---------------------------------------------------------------------------
static int reqSlot() { return clientForRequestIp((uint32_t)server.client().remoteIP()); }

static void sendStatic(const char *type, const uint8_t *data, size_t len, bool cache) {
  server.sendHeader("Cache-Control", cache ? "max-age=86400" : "no-store");
  server.setContentLength(len);
  server.send(200, type, "");
  WiFiClient c = server.client();
  size_t sent = 0;
  while (sent < len) {
    size_t n = c.write(data + sent, min((size_t)1460, len - sent));
    if (n == 0) break;
    sent += n;
  }
}

static void sendJson(int code, const String &body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", body);
}
static void sendErr(int code, const String &msg) { sendJson(code, "{\"error\":\"" + msg + "\"}"); }

// Everything the phone needs to draw its screen.
static String stateJson(int s) {
  Phone &c = clients[s];
  uint32_t bottles = 0, minutes = 0;
  if (c.user[0]) { int32_t dummy; loadUserNumbers(c.user, dummy, bottles, minutes); }
  bool mine = drop.active && drop.slot == s && memcmp(drop.mac, c.mac, 6) == 0;
  String j = "{";
  j += "\"user\":\"" + String(c.user) + "\"";
  j += ",\"secs\":" + String(c.secs);
  j += ",\"online\":" + String(c.online ? "true" : "false");
  j += ",\"mpb\":" + String(cfg.minPerBottle);
  j += ",\"grace\":" + String(cfg.guestGraceS);
  j += ",\"binFull\":" + String(binIsFull() ? "true" : "false");
  j += ",\"test\":" + testJson();
  j += ",\"code\":\"" MACHINE_CODE "\"";
  j += ",\"mac\":\"" + macTail(c.mac) + "\"";
  j += ",\"machine\":\"" + String(mine ? "mine" : drop.active ? "busy" : "free") + "\"";
  j += ",\"visit\":{\"b\":" + String(c.visitBottles) + ",\"m\":" + String(c.visitMinutes) + "}";
  j += ",\"stats\":{\"b\":" + String(bottles) + ",\"m\":" + String(minutes) + "}";
  if (mine) {
    int left = DROP_IDLE_TIMEOUT_S - (int)((millis() - drop.lastActivityMs) / 1000);
    j += ",\"drop\":{\"acc\":" + String(drop.accepted) + ",\"min\":" + String(drop.accepted * cfg.minPerBottle) +
         ",\"left\":" + String(max(left, 0)) + ",\"ev\":[";
    for (int i = 0; i < drop.evCount; i++) {
      if (i) j += ",";
      j += "[" + String(drop.ev[i].ok ? 1 : 0) + ",\"" + String(drop.ev[i].text) + "\"]";
    }
    j += "]}";
  }
  return j + "}";
}

static void sendOk(int s) { sendJson(200, "{\"ok\":1,\"state\":" + stateJson(s) + "}"); }

// ---------------------------------------------------------------------------
//  Student API
// ---------------------------------------------------------------------------
static void apiState() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  sendJson(200, stateJson(s));
}

static void bindAccount(int s, const String &u, bool remember, int32_t accountSecs) {
  Phone &c = clients[s];
  int32_t guestSecs = c.user[0] ? 0 : c.secs;
  bool wasOnline = c.online;
  strlcpy(c.user, u.c_str(), sizeof(c.user));
  c.secs = accountSecs + guestSecs;          // guest minutes move into the account
  c.secsAtStart = c.secs;
  c.remembered = remember;
  c.online = wasOnline && c.secs > 0;
  saveClientAccount(s);
  if (remember) rememberDevice(c.mac, u); else forgetDevice(c.mac);
  filterSync();
}

static void apiSignup() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  String u = server.arg("u"); u.toLowerCase(); u.trim();
  String p = server.arg("p");
  if (clients[s].user[0]) return sendErr(400, "Log out first");
  if (!validUsername(u)) return sendErr(400, "Username: 3-15 letters, numbers, dot or underscore");
  if (p.length() < 4) return sendErr(400, "Password must be at least 4 characters");
  if (userExists(u)) return sendErr(409, "That username is taken");
  if (!createUser(u, p, 0)) return sendErr(500, "Could not save the account");
  bindAccount(s, u, server.arg("remember") == "1", 0);
  Serial.printf("New account: %s\n", u.c_str());
  sendOk(s);
}

static void apiLogin() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  String u = server.arg("u"); u.toLowerCase(); u.trim();
  if (!checkPassword(u, server.arg("p"))) return sendErr(401, "Wrong username or password");
  if (clients[s].user[0] && !u.equals(clients[s].user)) unbindUser(s, true);
  // one phone per account: pause it on any other phone first (this saves its balance)
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (i != s && clients[i].used && u.equals(clients[i].user)) unbindUser(i, true);
  if (u.equals(clients[s].user)) { sendOk(s); return; }
  int32_t secs; uint32_t b, m;
  loadUserNumbers(u, secs, b, m);
  bindAccount(s, u, server.arg("remember") == "1", secs);
  sendOk(s);
}

static void apiLogout() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  if (drop.active && drop.slot == s) dropEnd("logout", 0);
  unbindUser(s, true);
  sendOk(s);
}

static void apiConnect() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  if (clients[s].secs <= 0) return sendErr(400, "No minutes left. Drop bottles first.");
  setOnline(s, true);
  sendOk(s);
}

static void apiPause() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  if (!clients[s].user[0]) return sendErr(400, "Log in to pause and save minutes");
  setOnline(s, false);
  saveClientAccount(s);
  sendOk(s);
}

static void apiDropStart() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  if (binIsFull()) return sendErr(409, "Bin is full. Please tell the admin.");
  if (drop.active && drop.slot != s) return sendErr(409, "Machine in use. Wait a moment.");
  dropStart(s);
  sendOk(s);
}

static void apiDropEnd() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  // use=1: start internet now, use=0: keep minutes as balance
  if (drop.active && drop.slot == s) dropEnd("done", server.arg("use") == "1" ? 1 : 0);
  sendOk(s);
}

static void apiTestDrop() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  if (!cfg.testMode) return sendErr(400, "Test mode is off");
  if (!testStart(s)) return sendErr(409, "Machine is busy. Wait a moment.");
  sendOk(s);
}

static void apiHistory() {
  int s = reqSlot();
  if (s < 0) return sendErr(503, "Reconnect to BottleKonek WiFi");
  if (!clients[s].user[0]) return sendErr(401, "Log in to see your history");
  sendJson(200, "{\"items\":" + historyJson(clients[s].user) + "}");
}

// ---------------------------------------------------------------------------
//  Admin API
// ---------------------------------------------------------------------------
static bool isAdmin() {
  if (!adminToken.length()) return false;
  String ck = server.header("Cookie");
  return ck.indexOf("bka=" + adminToken) >= 0;
}

static void apiAdminLogin() {
  if (server.arg("p") != ADMIN_PASSWORD) { delay(800); return sendErr(401, "Wrong admin password"); }
  char t[17];
  snprintf(t, sizeof(t), "%08x%08x", (unsigned)esp_random(), (unsigned)esp_random());
  adminToken = t;
  server.sendHeader("Set-Cookie", "bka=" + adminToken + "; Path=/; HttpOnly; SameSite=Strict");
  sendJson(200, "{\"ok\":1}");
}

static void apiAdminStats() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  int online = 0;
  String phones = "[";
  bool first = true;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    Phone &c = clients[i];
    if (!c.used) continue;
    if (c.online) online++;
    if (!first) phones += ",";
    first = false;
    phones += "{\"mac\":\"" + macTail(c.mac) + "\",\"u\":\"" + String(c.user) + "\",\"on\":" + (c.online ? "1" : "0") +
              ",\"c\":" + (c.connected ? "1" : "0") + ",\"s\":" + String(c.secs) + "}";
  }
  phones += "]";
  String j = "{\"bin\":" + String(stats.binCount) + ",\"today\":" + String(stats.todayBottles) + ",\"rej\":" + String(stats.todayRejects) +
             ",\"total\":" + String(stats.totalBottles) + ",\"online\":" + String(online) + ",\"max\":" + String(MAX_PHONES) +
             ",\"accounts\":" + String(countUsers()) + ",\"sta\":" + (WiFi.status() == WL_CONNECTED ? "true" : "false") +
             ",\"ip\":\"" + WiFi.localIP().toString() + "\",\"up\":" + String(millis() / 1000) +
             ",\"cfg\":{\"mpb\":" + String(cfg.minPerBottle) + ",\"grace\":" + String(cfg.guestGraceS) +
             ",\"bincap\":" + String(cfg.binCapacity) +
             ",\"test\":" + String(cfg.testMode ? "true" : "false") + ",\"gopen\":" + String(cfg.gateOpenMs) +
             ",\"gclose\":" + String(cfg.gateCloseMs) + ",\"gspd\":" + String(cfg.gateSpeed) + ",\"gdir\":" + String(cfg.gateDir) + "},\"phones\":" + phones + "}";
  sendJson(200, j);
}

static void apiAdminSettings() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  cfg.minPerBottle = constrain(server.arg("mpb").toInt(), 1, 120);
  cfg.guestGraceS  = constrain(server.arg("grace").toInt(), 0, 3600);
  cfg.binCapacity  = constrain(server.arg("bincap").toInt(), 5, 1000);
  if (server.hasArg("gopen")) {
    cfg.gateOpenMs  = constrain(server.arg("gopen").toInt(), 50, 3000);
    cfg.gateCloseMs = constrain(server.arg("gclose").toInt(), 50, 3000);
    cfg.gateSpeed   = constrain(server.arg("gspd").toInt(), 50, 500);
    cfg.gateDir     = server.arg("gdir").toInt() < 0 ? -1 : 1;
  }
  saveSettings();
  sendJson(200, "{\"ok\":1}");
}

static void apiAdminBinEmptied() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  stats.binCount = 0;
  saveStats();
  Serial.println("Bin emptied (count reset to 0)");
  sendJson(200, "{\"ok\":1}");
}

static void apiAdminTestMode() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  cfg.testMode = server.arg("on") == "1";
  saveSettings();
  Serial.printf("Test mode %s\n", cfg.testMode ? "ON" : "OFF");
  sendJson(200, "{\"ok\":1}");
}

static void apiAdminGate() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  if (!adminGate(server.arg("a"))) return sendErr(409, "Machine is busy, try again in a moment");
  sendJson(200, "{\"ok\":1}");
}

static void apiAdminSensors() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  sendJson(200, sensorsJson());
}

static void apiAdminRecal() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  recalibrateDrop();
  sendJson(200, sensorsJson());
}

static void apiAdminUsers() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  sendJson(200, usersJson());
}

static void apiAdminUser() {
  if (!isAdmin()) return sendErr(401, "Please log in");
  String u = server.arg("u"), a = server.arg("a");
  if (!userExists(u)) return sendErr(404, "No such account");
  int slot = -1;
  for (int i = 0; i < MAX_CLIENTS; i++) if (clients[i].used && u.equals(clients[i].user)) slot = i;
  if (a == "add") {
    int m = constrain(server.arg("m").toInt(), 1, 600);
    if (slot >= 0) { clients[slot].secs += m * 60; saveClientAccount(slot); filterSync(); }
    else { int32_t secs; uint32_t b, mm; loadUserNumbers(u, secs, b, mm); saveUserSecs(u, secs + m * 60); }
  } else if (a == "reset") {
    if (!setPassword(u, server.arg("p"))) return sendErr(400, "Password must be at least 4 characters");
  } else if (a == "delete") {
    if (slot >= 0) unbindUser(slot, true);
    deleteUser(u);
  } else return sendErr(400, "Unknown action");
  sendJson(200, "{\"ok\":1}");
}

// ---------------------------------------------------------------------------
//  Captive portal: any unknown address goes to the portal page
// ---------------------------------------------------------------------------
static void redirectToPortal() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.sendHeader("Cache-Control", "no-store");
  server.send(302, "text/plain", "");
}

void webBegin() {
  const char *keys[] = { "Cookie" };
  server.collectHeaders(keys, 1);

  server.on("/", HTTP_GET, []() {
    if (server.hostHeader() != "192.168.4.1" && server.hostHeader().length()) return redirectToPortal();
    sendStatic("text/html; charset=utf-8", (const uint8_t *)PORTAL_HTML, strlen(PORTAL_HTML), false);
  });
  server.on("/admin", HTTP_GET, []() {
    sendStatic("text/html; charset=utf-8", (const uint8_t *)ADMIN_HTML, strlen(ADMIN_HTML), false);
  });
  server.on("/logo.jpg", HTTP_GET, []() { sendStatic("image/jpeg", LOGO_JPG, LOGO_JPG_LEN, true); });

  server.on("/api/state", HTTP_GET, apiState);
  server.on("/api/signup", HTTP_POST, apiSignup);
  server.on("/api/login", HTTP_POST, apiLogin);
  server.on("/api/logout", HTTP_POST, apiLogout);
  server.on("/api/connect", HTTP_POST, apiConnect);
  server.on("/api/pause", HTTP_POST, apiPause);
  server.on("/api/drop/start", HTTP_POST, apiDropStart);
  server.on("/api/drop/end", HTTP_POST, apiDropEnd);
  server.on("/api/history", HTTP_GET, apiHistory);
  server.on("/api/test/drop", HTTP_POST, apiTestDrop);

  server.on("/api/admin/login", HTTP_POST, apiAdminLogin);
  server.on("/api/admin/stats", HTTP_GET, apiAdminStats);
  server.on("/api/admin/settings", HTTP_POST, apiAdminSettings);
  server.on("/api/admin/users", HTTP_GET, apiAdminUsers);
  server.on("/api/admin/bin-emptied", HTTP_POST, apiAdminBinEmptied);
  server.on("/api/admin/testmode", HTTP_POST, apiAdminTestMode);
  server.on("/api/admin/gate", HTTP_POST, apiAdminGate);
  server.on("/api/admin/sensors", HTTP_GET, apiAdminSensors);
  server.on("/api/admin/recal", HTTP_POST, apiAdminRecal);
  server.on("/api/admin/user", HTTP_POST, apiAdminUser);

  server.onNotFound(redirectToPortal);     // generate_204, hotspot-detect.html, etc.
  server.begin();
}

void webLoop() { server.handleClient(); }
