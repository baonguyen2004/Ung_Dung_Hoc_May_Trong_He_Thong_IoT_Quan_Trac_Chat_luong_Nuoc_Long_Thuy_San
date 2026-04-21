//======================================================================//
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "wifiConfig.h"
//======================================================================//

// ---------------- LoRa pins ----------------
#define PIN_LORA_COPI   23
#define PIN_LORA_CIPO   19
#define PIN_LORA_SCK    18
#define PIN_LORA_CS     5
#define PIN_LORA_RST    17
#define PIN_LORA_DIO0   4

#define LORA_FREQUENCY  433E6

// ---------------- OLED ----------------
#define I2C_SDA         21
#define I2C_SCL         22
#define OLED_ADDR       0x3C
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- MQTT ----------------
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT   1883

WiFiClient espClient;
PubSubClient mqttClient(espClient);

//======================================================================//
// ---------------- PARSE JSON ----------------
bool extractJsonFloat(const String& s, const char* key, float &out) {
  String k = String("\"") + key + "\":";
  int i = s.indexOf(k);
  if (i < 0) return false;

  i += k.length();

  int end = s.indexOf(',', i);
  if (end < 0) end = s.indexOf('}', i);
  if (end < 0) return false;

  out = s.substring(i, end).toFloat();
  return !isnan(out);
}

bool parseJSON(const String& s, float &temp, float &ph, float &sal) {
  return extractJsonFloat(s, "temperature", temp) &&
         extractJsonFloat(s, "pH", ph) &&
         extractJsonFloat(s, "salinity", sal);
}

//======================================================================//
// ---------------- OLED DISPLAY ----------------
void oledShow(float temp, float ph, float sal, int rssi, float snr) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ===== Dòng trên: RF info (nhỏ) =====
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RSSI:");
  display.print(rssi);

  display.setCursor(70, 0);
  display.print("SNR:");
  display.print(snr, 1);

  // ===== Nhiệt độ (to) =====
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print("T:");
  display.print(temp, 1);

  // ===== pH (to) =====
  display.setCursor(0, 34);
  display.print("pH:");
  display.print(ph, 1);

  // ===== Salinity (to) =====
  display.setCursor(0, 54);
  display.print("S:");
  display.print(sal, 1);

  display.display();
}

//======================================================================//
// ---------------- MQTT RECONNECT ----------------
void mqttReconnect() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32_LoRa_RX_";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

//======================================================================//
void setup() {
  Serial.begin(115200);

  // OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("SYSTEM START...");
  display.display();

  // WiFi config
  wifiConfig.begin();

  // LoRa
  SPI.begin(PIN_LORA_SCK, PIN_LORA_CIPO, PIN_LORA_COPI, PIN_LORA_CS);
  LoRa.setPins(PIN_LORA_CS, PIN_LORA_RST, PIN_LORA_DIO0);
  LoRa.setSPIFrequency(8000000);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    display.println("LoRa FAIL");
    display.display();
    while (1);
  }

  // MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  display.clearDisplay();
  display.println("SYSTEM READY");
  display.display();
}

//======================================================================//
void loop() {
  wifiConfig.run();

  if (wifiMode == 1) {
    if (!mqttClient.connected()) mqttReconnect();
    mqttClient.loop();
    handleLoRaTask();
  }
}

//======================================================================//
// ---------------- HANDLE LORA ----------------
void handleLoRaTask() {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  String incoming = "";
  while (LoRa.available()) incoming += (char)LoRa.read();
  incoming.trim();

  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();

  float temp, ph, sal;

  bool ok = parseJSON(incoming, temp, ph, sal);

  if (ok) {
    oledShow(temp, ph, sal, rssi, snr);

    // Convert string với 2 số thập phân
    char tempStr[10], phStr[10], salStr[10];
    dtostrf(temp, 6, 2, tempStr);
    dtostrf(ph, 6, 2, phStr);
    dtostrf(sal, 6, 2, salStr);

    // MQTT từng biến
    mqttClient.publish("lora/sensor/temperature", tempStr);
    mqttClient.publish("lora/sensor/ph", phStr);
    mqttClient.publish("lora/sensor/salinity", salStr);

    // MQTT JSON tổng (chuẩn 2 số thập phân)
    String payload = String("{\"temperature\":") + String(temp, 2) +
                     ",\"pH\":" + String(ph, 2) +
                     ",\"salinity\":" + String(sal, 2) + "}";

    mqttClient.publish("lora/sensor/all", payload.c_str());

    Serial.println("Received: " + incoming);
    Serial.println("MQTT -> " + payload);
  } else {
    Serial.println("Parse FAIL: " + incoming);
  }
}