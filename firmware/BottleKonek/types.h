// Shared types and function declarations for all BottleKonek tabs.
#pragma once
#include <Arduino.h>
#include <IPAddress.h>
#include <WiFi.h>
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

#define MAX_CLIENTS 10
#define USER_MAX    15   // username length limit

// One phone connected to (or recently on) BottleKonek WiFi.
struct Phone {
  bool     used;
  uint8_t  mac[6];
  uint32_t ip;            // learned from its packets (network byte order as in IPAddress)
  bool     connected;
  uint32_t goneAtMs;      // when it disconnected (guests keep time for the grace period)
  char     user[USER_MAX + 1];   // "" = guest
  bool     remembered;
  int32_t  secs;          // guest balance, or the account's balance while logged in here
  bool     online;        // internet allowed (only while secs > 0)
  uint32_t onlineSinceMs;
  int32_t  secsAtStart;   // to log how many minutes were used
  uint32_t lastSaveMs;
  uint16_t visitBottles;
  uint16_t visitMinutes;
};

// A drop-bottles session (one person at a time).
struct DropEvent { bool ok; char text[40]; };
struct DropSession {
  bool     active;
  int      slot;          // index in clients[]
  uint8_t  mac[6];
  uint16_t accepted;
  uint16_t rejected;
  uint32_t lastActivityMs;
  DropEvent ev[6];        // newest first
  uint8_t  evCount;
};

// One account as stored in flash (accounts tab).
struct UserRec { String h, s; int32_t b = 0; uint32_t t = 0, m = 0, c = 0; };

struct Settings {
  int minPerBottle;
  int guestGraceS;
  int binCapacity;      // bottles until the bin counts as full
  bool testMode;        // admin switch: shows the Test drop button on the website
  int gateOpenMs;       // 360 servo: spin time to open
  int gateCloseMs;      // 360 servo: spin time to close
  int gateSpeed;        // 360 servo: spin speed (us away from 1500)
  int gateDir;          // 1 or -1
  bool servo360;        // true = 360 servo (timed), false = 180 servo (angles)
  int openDeg;          // 180 servo: open angle
  int closeDeg;         // 180 servo: closed angle
};

struct Stats {
  uint32_t totalBottles;
  uint32_t todayBottles;
  uint32_t todayRejects;
  int      todayYday;     // day-of-year the "today" counters belong to
  uint32_t binCount;      // bottles dropped since the bin was last emptied
};

// ---- globals (defined in BottleKonek.ino) ----
extern Phone       clients[MAX_CLIENTS];
extern DropSession drop;
extern Settings    cfg;
extern Stats       stats;

// ---- network.ino ----
void networkBegin();
void networkLoop();
void filterSync();
uint32_t macToIpLookup(const uint8_t *mac);
bool ipToMac(uint32_t ip, uint8_t *macOut);
String macTail(const uint8_t *mac);
String macHex(const uint8_t *mac);

// ---- clients (BottleKonek.ino) ----
int  findClientByMac(const uint8_t *mac);
int  findClientByIp(uint32_t ip);
int  clientForRequestIp(uint32_t ip);
void onPhoneConnected(const uint8_t *mac);
void onPhoneDisconnected(const uint8_t *mac);
void tickClients();
void setOnline(int slot, bool on);
void saveClientAccount(int slot);
void unbindUser(int slot, bool forget);

// ---- accounts.ino ----
void storageBegin();
bool validUsername(const String &u);
bool userExists(const String &u);
bool createUser(const String &u, const String &p, int32_t startSecs);
bool checkPassword(const String &u, const String &p);
bool setPassword(const String &u, const String &p);
bool loadUserNumbers(const String &u, int32_t &secs, uint32_t &bottles, uint32_t &minutes);
bool saveUserSecs(const String &u, int32_t secs);
bool addUserTotals(const String &u, uint32_t bottles, uint32_t minutes);
void logHistory(const String &u, const String &line);
String historyJson(const String &u);
void rememberDevice(const uint8_t *mac, const String &u);
void forgetDevice(const uint8_t *mac);
String rememberedUser(const uint8_t *mac);
String usersJson();
bool deleteUser(const String &u);
int  countUsers();
void loadSettings();
void saveSettings();
void loadStats();
void saveStats();
uint32_t nowEpoch();
void rollDay();

// ---- machine.ino ----
void machineBegin();
void machineLoop();
void lcdShow(const String &l1, const String &l2);
void beep(int times);
bool dropStart(int slot);
void dropEnd(const char *why, int useNow);   // useNow: 1 = start internet, 0 = save, -1 = automatic
bool machineBusy();
bool binIsFull();
bool testStart(int slot);
String testJson();
bool adminGate(const String &action);
String sensorsJson();
String demoJson();
void demoSet(bool on);
bool demoAccept();
bool demoReject();
void recalibrateDrop();

// ---- web.ino ----
void webBegin();
void webLoop();
