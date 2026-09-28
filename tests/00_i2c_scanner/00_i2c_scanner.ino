#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n--- ESP32 I2C Scanner Starting ---");
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus (GPIO 21 & 22)...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("SUCCESS: I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("ERROR: No I2C devices found! Check SDA, SCL, VCC, and GND wiring.");
  } else {
    Serial.println("Scan complete.");
  }

  delay(3000);
}
