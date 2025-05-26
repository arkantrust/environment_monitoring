o#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <U8g2lib.h>

// Initialize BME280
#define BME280_ADDRESS 0x76 // Change to 0x77 if needed
Adafruit_BME280 bme;

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

  delay(1000);
}

void loop() {
  float temp = bme.readTemperature();        // °C
  float hum  = bme.readHumidity();           // %
  float pres = bme.readPressure() / 100.0F;  // hPa
  float alt  = bme.readAltitude(1013.25);    // m

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

