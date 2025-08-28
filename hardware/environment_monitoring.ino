#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "ArduinoJson.h"
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Constants
const unsigned long WIFI_TIMEOUT = 30000; // 30 seconds timeout for WiFi connection

// Wi-Fi configuration
const char *WIFI_SSID = "david";
const char *WIFI_PASSWORD = "1234567890";

// Server configuration
const char *SERVER_URL = "http://172.20.10.2:3000/";

// Global objects
WiFiClient espClient;
Adafruit_BME280 bme;
#define BME280_ADDRESS 0x76 // Change to 0x77 if needed

// Initialize OLED (128x64 I2C)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  Wire.begin(); // SDA: GPIO21, SCL: GPIO22 on ESP32

  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, "Initializing...");
  u8g2.sendBuffer();

  if (!bme.begin(BME280_ADDRESS)) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 12, "BME280 not found!");
    u8g2.sendBuffer();
    Serial.println("BME280 sensor not found. Check wiring!");
    while (1);
  }

  if (!connectWiFi()) {
    setLed(ERROR);
    while (1)
    delay(100); // Halt if WiFi connection fails
  }

  delay(1000);
}

void loop() {
  float temp = bme.readTemperature();        // °C
  float hum  = bme.readHumidity();           // %
  float pres = bme.readPressure() / 100.0F;  // hPa
  float alt  = bme.readAltitude(1013.25);    // m

  sendData(temp, hum, pres, alt);

  char line1[32], line2[32], line3[32], line4[32];
  snprintf(line1, sizeof(line1), "Temp: %.2f C", temp);
  snprintf(line2, sizeof(line2), "Hum : %.2f %%", hum);
  snprintf(line3, sizeof(line3), "Pres: %.2f hPa", pres);
  snprintf(line4, sizeof(line4), "Alt : %.2f m", alt);

  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, line1);
  u8g2.drawStr(0, 24, line2);
  u8g2.drawStr(0, 36, line3);
  u8g2.drawStr(0, 48, line4);
  u8g2.sendBuffer();

  delay(1000);
}

bool connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi");
    return false;
  }

  Serial.println("\nConnected to WiFi");
  Serial.println("IP address: " + WiFi.localIP().toString());
  return true;
}

void sendData( float temp, float hum, float pres, float alt ) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi not connected");
    return;
  }


  // Build JSON with static RAM

  StaticJsonDocument<200> doc;
  doc["temperature"] = temp;
  doc["humidity"]    = hum;
  doc["pressure"]    = pres;
  doc["altitude"]    = alt;

  // Serialize JSON to String (POST payload)
  String payload;
  serializeJson(doc, payload);

  // (optional) print JSON on serial monitor
  Serial.print("[HTTP] Payload:");
  serializeJson(doc, Serial);
  Serial.println()
  
  // Prepare HTTP Client
  HTTPClient http;
  http.setTimeout(2000);

  if(!http.begin(espClient, SERVER_URL)){
  Serial.println("[HTTP] begin() falló (URL inválida?)");
  return;
  }

  http.addHeader("Content-Type", "application/json");

  //Send Post
  int httpCode = http.POST(payload);

  // Check server answer
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST -> código: %d\n", httpCode);
    String resp = http.getString();
    Serial.println("[HTTP] Respuesta del servidor:");
    Serial.println(resp);
  } else {
    Serial.printf("[HTTP] POST falló: %s\n", http.errorToString(httpCode).c_str());
  }

  // Release HTTP client resources
  http.end();
}