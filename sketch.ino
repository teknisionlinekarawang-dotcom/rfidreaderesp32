#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22
#define RELAY_PIN 26
#define BUZZER_PIN 27
#define LED_GREEN 14
#define LED_RED 12
#define BUZZER_PIN 27

MFRC522 rfid(SS_PIN, RST_PIN);
WebServer server(80);
Preferences prefs;

const char* ssid = "ESP32-RFID";
const char* password = "12345678";

String uidList[20];
bool enrollMode = false;
bool relayState = false;
int uidCount = 0;

void beepShort() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepLong() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(600);
  digitalWrite(BUZZER_PIN, LOW);
}

bool isUIDRegistered(String uid) {
  for (int i = 0; i < uidCount; i++) {
    if (uidList[i] == uid) return true;
  }
  return false;
}

void handleRoot() {
  String page = "<!DOCTYPE html><html><head><style>";
  page += "body{font-family:Arial;background:#f4f4f4;text-align:center;}";
  page += "button{padding:10px 20px;margin:5px;font-size:16px;}";
  page += "ul{list-style:none;padding:0;}li{background:#fff;margin:5px;padding:5px;}";
  page += "</style></head><body>";

  page += "<h2>ESP32 RFID Access Control</h2>";

  page += "<form action='/enroll' method='POST'>";
  page += "<button>Mode Tambah Kartu (Scan)</button></form>";

  page += "<form action='/relayOn' method='POST'>";
  page += "<button>Relay ON</button></form>";

  page += "<form action='/relayOff' method='POST'>";
  page += "<button>Relay OFF</button></form>";

  page += "<h3>UID Terdaftar</h3><ul>";
  for (int i = 0; i < uidCount; i++) page += "<li>" + uidList[i] + "</li>";
  page += "</ul></body></html>";

  server.send(200, "text/html", page);
}

void handleEnroll() {
  enrollMode = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

void relayOn() {
  relayState = true;
  digitalWrite(RELAY_PIN, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void relayOff() {
  relayState = false;
  digitalWrite(RELAY_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
pinMode(BUZZER_PIN, OUTPUT);
pinMode(LED_GREEN, OUTPUT);
pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();

  prefs.begin("rfid", false);
  uidCount = prefs.getUInt("count", 0);
  for (int i = 0; i < uidCount; i++) {
    uidList[i] = prefs.getString(String(i).c_str(), "");
  }

  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
server.on("/enroll", HTTP_POST, handleEnroll);
server.on("/relayOn", HTTP_POST, relayOn);
server.on("/relayOff", HTTP_POST, relayOff);
  server.on("/add", HTTP_POST, handleAdd);
  server.begin();
}

void loop() {
  server.handleClient();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }

  if (enrollMode) {
    uidList[uidCount] = uid;
    prefs.putString(String(uidCount).c_str(), uid);
    uidCount++;
    prefs.putUInt("count", uidCount);
    enrollMode = false;
    beepShort();
}

else if (isUIDRegistered(uid)) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    beepShort();
}

else {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    beepLong();
}

  rfid.PICC_HaltA();
}
