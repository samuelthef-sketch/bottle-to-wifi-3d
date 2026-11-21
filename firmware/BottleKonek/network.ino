// ============================================================================
//  network: main WiFi + "BottleKonek WiFi" hotspot + internet sharing
//
//  How a phone is kept on the portal until it has minutes:
//   1. Packet filter: every frame a phone sends is checked. Phones without
//      time may only talk to the ESP32 itself (192.168.4.1); everything else
//      is dropped before it can be forwarded to the internet.
//   2. DNS relay: phones without time get 192.168.4.1 for every website name,
//      which makes Android/iPhone pop up the sign-in page. Phones with time
//      get real answers from the main router's DNS.
//   3. NAT (NAPT): traffic from allowed phones goes out through the main WiFi.
// ============================================================================
#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "freertos/queue.h"

static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);
static const IPAddress AP_LEASE_START(192, 168, 4, 2);
static uint32_t apIpRaw;
static bool     testMode = false;   // true when MAIN_WIFI_SSID is not filled in

// ---------------------------------------------------------------------------
//  Packet filter
// ---------------------------------------------------------------------------
static portMUX_TYPE fmux = portMUX_INITIALIZER_UNLOCKED;
static uint8_t allowMacs[MAX_CLIENTS][6];
static int     allowCount = 0;

struct IpMapEntry { uint8_t mac[6]; uint32_t ip; bool used; };
static IpMapEntry ipMap[16];
static uint8_t    ipMapNext = 0;

static netif_input_fn originalInput = nullptr;

// Runs inside the WiFi driver for every frame a phone sends. Keep it short.
static err_t filterInput(struct pbuf *p, struct netif *inp) {
  if (p != nullptr && p->len >= 34) {
    const uint8_t *f = (const uint8_t *)p->payload;
    uint16_t type = ((uint16_t)f[12] << 8) | f[13];
    const uint8_t *src = f + 6;

    if (type == 0x0800 || type == 0x86DD) {
      bool allowed = false;
      uint32_t sip = 0, dip = 0;
      if (type == 0x0800) { memcpy(&sip, f + 26, 4); memcpy(&dip, f + 30, 4); }

      portENTER_CRITICAL(&fmux);
      for (int i = 0; i < allowCount; i++)
        if (memcmp(allowMacs[i], src, 6) == 0) { allowed = true; break; }
      if (sip != 0) {                         // learn which IP belongs to which phone
        int hit = -1;
        for (int i = 0; i < 16; i++)
          if (ipMap[i].used && memcmp(ipMap[i].mac, src, 6) == 0) { hit = i; break; }
        if (hit < 0) { hit = ipMapNext; ipMapNext = (ipMapNext + 1) % 16; memcpy(ipMap[hit].mac, src, 6); ipMap[hit].used = true; }
        ipMap[hit].ip = sip;
      }
      portEXIT_CRITICAL(&fmux);

      if (!allowed) {
        if (type == 0x86DD) { pbuf_free(p); return ERR_OK; }   // no IPv6 for phones without time
        const uint8_t *d = f + 30;
        bool local = (dip == apIpRaw) || dip == 0xFFFFFFFFUL ||
                     (d[0] >= 224 && d[0] <= 239) ||
                     (d[0] == 192 && d[1] == 168 && d[2] == 4 && d[3] == 255);
        if (!local) { pbuf_free(p); return ERR_OK; }           // dropped: no minutes
      }
    }
  }
  return originalInput(p, inp);
}

// Copy the list of phones that may use the internet into the filter.
void filterSync() {
  uint8_t tmp[MAX_CLIENTS][6];
  int n = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    Phone &c = clients[i];
    if (c.used && c.connected && c.online && c.secs > 0) memcpy(tmp[n++], c.mac, 6);
  }
  portENTER_CRITICAL(&fmux);
  memcpy(allowMacs, tmp, sizeof(tmp));
  allowCount = n;
  portEXIT_CRITICAL(&fmux);
}

uint32_t macToIpLookup(const uint8_t *mac) {
  uint32_t ip = 0;
  portENTER_CRITICAL(&fmux);
  for (int i = 0; i < 16; i++)
    if (ipMap[i].used && memcmp(ipMap[i].mac, mac, 6) == 0) { ip = ipMap[i].ip; break; }
  portEXIT_CRITICAL(&fmux);
  return ip;
}

bool ipToMac(uint32_t ip, uint8_t *macOut) {
  bool ok = false;
  portENTER_CRITICAL(&fmux);
  for (int i = 0; i < 16; i++)
    if (ipMap[i].used && ipMap[i].ip == ip) { memcpy(macOut, ipMap[i].mac, 6); ok = true; break; }
  portEXIT_CRITICAL(&fmux);
  return ok;
}

static bool ipAllowed(uint32_t ip) {
  uint8_t mac[6];
  if (!ipToMac(ip, mac)) return false;
  bool ok = false;
  portENTER_CRITICAL(&fmux);
  for (int i = 0; i < allowCount; i++)
    if (memcmp(allowMacs[i], mac, 6) == 0) { ok = true; break; }
  portEXIT_CRITICAL(&fmux);
  return ok;
}

static bool installFilter() {
  esp_netif_t *apNetif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (!apNetif) return false;
  struct netif *n = (struct netif *)esp_netif_get_netif_impl(apNetif);
  if (!n || !n->input) return false;
  if (n->input != filterInput) { originalInput = n->input; n->input = filterInput; }
  return true;
}

String macHex(const uint8_t *m) {
  char b[13];
  snprintf(b, sizeof(b), "%02X%02X%02X%02X%02X%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return String(b);
}
String macTail(const uint8_t *m) {
  char b[16];
  snprintf(b, sizeof(b), "...:%02X:%02X", m[4], m[5]);
  return String(b);
}

// ---------------------------------------------------------------------------
//  Phone connect / disconnect events (queued, handled in loop)
// ---------------------------------------------------------------------------
struct NetEvt { uint8_t type; uint8_t mac[6]; };
static QueueHandle_t evq;

static void onWiFiEvent(arduino_event_id_t e, arduino_event_info_t info) {
  NetEvt ev;
  switch (e) {
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      ev.type = 1; memcpy(ev.mac, info.wifi_ap_staconnected.mac, 6); xQueueSend(evq, &ev, 0); break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      ev.type = 2; memcpy(ev.mac, info.wifi_ap_stadisconnected.mac, 6); xQueueSend(evq, &ev, 0); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Main WiFi connected. ESP32 IP: "); Serial.println(WiFi.localIP()); break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
      static uint32_t lastMsg = 0;          // print at most every 10 s
      if (millis() - lastMsg > 10000) {
        lastMsg = millis();
        uint8_t r = info.wifi_sta_disconnected.reason;
        const char *why =
          (r == 201) ? "WiFi name not found (check spelling, must be 2.4 GHz, move closer)" :
          (r == 15 || r == 202 || r == 204 || r == 2) ? "wrong password (or router security not supported)" :
          (r == 203) ? "router refused the ESP32 (check MAC filter / max devices)" :
          "other";
        Serial.printf("Main WiFi not connected (reason %u: %s), retrying...\n", r, why);
      }
      break;
    }
    default: break;
  }
}

// ---------------------------------------------------------------------------
//  DNS relay
// ---------------------------------------------------------------------------
static WiFiUDP dnsServer;     // phones ask here (port 53)
static WiFiUDP dnsUpstream;   // we ask the main router's DNS from here
struct PendingQ { bool used; uint16_t upId, cliId; IPAddress ip; uint16_t port; uint32_t t; };
static PendingQ pend[16];
static uint16_t nextUpId = 1;

static void dnsAnswerPortal(uint8_t *buf, int len, IPAddress ip, uint16_t port) {
  // walk the question name to find where the question ends
  int i = 12;
  while (i < len && buf[i] != 0) {
    if ((buf[i] & 0xC0) == 0xC0) { i += 1; break; }
    i += buf[i] + 1;
  }
  i += 1;                                  // zero byte
  if (i + 4 > len) return;
  uint16_t qtype = ((uint16_t)buf[i] << 8) | buf[i + 1];
  int qend = i + 4;

  uint8_t out[600];
  if (qend > 500) return;
  memcpy(out, buf, qend);
  out[2] = 0x84 | (buf[2] & 0x01);         // response, authoritative, keep RD
  out[3] = 0x80;                           // recursion available, no error
  out[4] = 0; out[5] = 1;                  // 1 question
  out[6] = 0; out[7] = (qtype == 1) ? 1 : 0;
  out[8] = out[9] = out[10] = out[11] = 0;
  int n = qend;
  if (qtype == 1) {                        // A record -> 192.168.4.1, TTL 10 s
    const uint8_t ans[] = { 0xC0, 0x0C, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x04,
                            192, 168, 4, 1 };
    memcpy(out + n, ans, sizeof(ans)); n += sizeof(ans);
  }
  dnsServer.beginPacket(ip, port);
  dnsServer.write(out, n);
  dnsServer.endPacket();
}

static void dnsLoop() {
  uint8_t buf[512];
  for (int guard = 0; guard < 8; guard++) {
    int sz = dnsServer.parsePacket();
    if (sz <= 0) break;
    int len = dnsServer.read(buf, sizeof(buf));
    IPAddress rip = dnsServer.remoteIP();
    uint16_t rport = dnsServer.remotePort();
    if (len < 12) continue;

    if (ipAllowed((uint32_t)rip) && WiFi.status() == WL_CONNECTED) {
      int k = -1;
      for (int j = 0; j < 16; j++) if (!pend[j].used || millis() - pend[j].t > 5000) { k = j; break; }
      if (k < 0) continue;
      pend[k].used = true;
      pend[k].cliId = ((uint16_t)buf[0] << 8) | buf[1];
      pend[k].upId = nextUpId++;
      if (nextUpId == 0) nextUpId = 1;
      pend[k].ip = rip; pend[k].port = rport; pend[k].t = millis();
      buf[0] = pend[k].upId >> 8; buf[1] = pend[k].upId & 0xFF;
      IPAddress up = WiFi.dnsIP(0);
      if ((uint32_t)up == 0) up = IPAddress(8, 8, 8, 8);
      dnsUpstream.beginPacket(up, 53);
      dnsUpstream.write(buf, len);
      dnsUpstream.endPacket();
    } else {
      dnsAnswerPortal(buf, len, rip, rport);
    }
  }
  for (int guard = 0; guard < 8; guard++) {
    int sz = dnsUpstream.parsePacket();
    if (sz <= 0) break;
    int len = dnsUpstream.read(buf, sizeof(buf));
    if (len < 12) continue;
    uint16_t id = ((uint16_t)buf[0] << 8) | buf[1];
    for (int j = 0; j < 16; j++) {
      if (pend[j].used && pend[j].upId == id) {
        buf[0] = pend[j].cliId >> 8; buf[1] = pend[j].cliId & 0xFF;
        dnsServer.beginPacket(pend[j].ip, pend[j].port);
        dnsServer.write(buf, len);
        dnsServer.endPacket();
        pend[j].used = false;
        break;
      }
    }
  }
}

// ---------------------------------------------------------------------------
void networkBegin() {
  apIpRaw = (uint32_t)AP_IP;
  evq = xQueueCreate(16, sizeof(NetEvt));
  WiFi.onEvent(onWiFiEvent);

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);                   // faster, steadier hotspot
  WiFi.softAPConfig(AP_IP, AP_IP, AP_MASK, AP_LEASE_START, AP_IP);   // phones use us as DNS
  WiFi.softAP(PORTAL_WIFI_NAME, nullptr, 1, 0, MAX_PHONES);

  if (strcmp(MAIN_WIFI_SSID, "PUT_MAIN_WIFI_NAME_HERE") == 0 || strlen(MAIN_WIFI_SSID) == 0) {
    // Main WiFi not filled in yet: run the hotspot + website only (no internet), good for testing parts
    testMode = true;
    Serial.println("TEST MODE: main WiFi name not set in config.h. Hotspot and website work, no internet.");
  } else {
    WiFi.setAutoReconnect(true);
    WiFi.setMinSecurity(WIFI_AUTH_WEP);      // also accept older routers (WPA/WPA2 TKIP)
    WiFi.begin(MAIN_WIFI_SSID, MAIN_WIFI_PASSWORD);
    Serial.print("Joining main WiFi \"" MAIN_WIFI_SSID "\"");
    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) { delay(250); Serial.print("."); }
    Serial.println(WiFi.status() == WL_CONNECTED ? " OK" : " not yet (will keep trying)");
  }

  if (!WiFi.AP.enableNAPT(true)) Serial.println("WARNING: could not enable internet sharing (NAPT)");
  if (!installFilter()) Serial.println("WARNING: packet filter not installed");

  dnsServer.begin(53);
  dnsUpstream.begin(53530);
  configTzTime("PHT-8", "pool.ntp.org", "time.google.com");   // Philippine time for history
}

void networkLoop() {
  NetEvt ev;
  while (xQueueReceive(evq, &ev, 0) == pdTRUE) {
    if (ev.type == 1) onPhoneConnected(ev.mac);
    else onPhoneDisconnected(ev.mac);
  }
  dnsLoop();
  static uint32_t lastCheck = 0;           // re-attach the filter if WiFi restarted
  if (millis() - lastCheck > 5000) { lastCheck = millis(); installFilter(); }
}
