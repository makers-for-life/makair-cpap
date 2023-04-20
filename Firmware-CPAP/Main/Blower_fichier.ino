#include "Arduino.h"
#include "HardwareTimer.h"
#include "blower.h"


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initialisation();
  setVitesseCourbe(1200);
}

void loop() {
  // put your main code here, to run repeatedly:
afficherVitesse();
}
