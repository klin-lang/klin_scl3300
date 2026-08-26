// WeAct STM32F411CE Black Pill + Murata SCL3300 (Arduino).
// Wiring and Klin twin: README.md in this folder.
// Library: https://github.com/DavidArmstrong/SCL3300

#include <SPI.h>
#include "SCL3300.h"

#define CS_PIN PB12

SCL3300 inclinometer(CS_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  // For acceleration in g, use MODE1 (library begin() may default to inclinometer).
  if (!inclinometer.begin()) {
    Serial.println("SCL3300 begin failed");
    return;
  }
  inclinometer.setMode(1);
}

void loop() {
  float ax = inclinometer.readX();
  float ay = inclinometer.readY();
  float az = inclinometer.readZ();

  Serial.print("X: ");
  Serial.print(ax);
  Serial.print("\tY: ");
  Serial.print(ay);
  Serial.print("\tZ: ");
  Serial.println(az);

  delay(500);
}
