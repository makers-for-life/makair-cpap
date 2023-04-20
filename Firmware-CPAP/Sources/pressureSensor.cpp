#include<Arduino.h>
#include<Wire.h>
#include "HardwareTimer.h"
#include "pressureSensor.h"

TwoWire myWire2(PRESSURE_SDA, PRESSURE_SCL);

HardwareTimer* hardwareTimer3;  // Timer Pressure command

uint8_t id = 0x28; // Adresse I2C du capteur de pression
uint8_t data[7]; // Données en sortie
uint8_t cmd[3] = {0xAA, 0x00, 0x00}; // La commande envoyée au capteur
double press_counts = 0; // Données brutes de la pression
double temp_counts = 0; // Données brutes de la température
double pressure = 0; // Données de la pression
double temperature = 0; // Données de la température
double outputmax = 15099494; // Plage des données de la pression (max)
double outputmin = 1677722; // Plage des données de la pression (min)
double pmax = 60; // Plage de mesure de la pression (max)
double pmin = -60; // Plage de mesure de la pression (min)
double percentage = 0; // Pourcentage de la pression mesurée en fonction de la plage de mesure
char printBuffer[200], cBuff[20], percBuff[20], pBuff[20], tBuff[20];

void Pressure_init(){
  myWire2.begin();
  sprintf(printBuffer, "\nStatus Register, 24 - bit Sensor data, Digital Pressure Counts,\
  Percentage of full scale pressure, Pressure Output, Temperature\n");
  Serial.println(printBuffer);
}

void Pressure_Timer(int periode) { // Cette fonction va permettre de générer le du timer pour la fonction du capteur de pression
  hardwareTimer3 = new HardwareTimer(TIM2); //Sélectionne le timer 2
  hardwareTimer3->setOverflow(periode, MICROSEC_FORMAT); //Définit la période/fréquence du timer
  hardwareTimer3->refresh(); //réninitialise le timer
  hardwareTimer3->setMode(TIM_CHANNEL_PRESSURE, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode OC sur le timer (Autres modes : Input Capture/PWM/One-pulse
  hardwareTimer3->setCaptureCompare(TIM_CHANNEL_PRESSURE, 0 , PERCENT_COMPARE_FORMAT); //Définit le début du compteur
  hardwareTimer3->attachInterrupt(TIM_CHANNEL_PRESSURE, Pressure_measure); //Active la fonction à chaque interruption
  hardwareTimer3->resume(); //Lance le timer
}

void Pressure_measure(){
  myWire2.beginTransmission(id);
  int stat = myWire2.write (cmd, 3); // Ecriture pour commander le capteur
  stat |= myWire2.endTransmission();
  delay(10);
  myWire2.requestFrom(id, 7); // Lire les 7 octets de données du capteur
  int i = 0;
  for (i = 0; i < 7; i++) {
    data [i] = myWire2.read();
  }
  press_counts = data[3] + data[2] * 256 + data[1] * 65536; // Calculer les données brutes de la pression
  temp_counts = data[6] + data[5] * 256 + data[4] * 65536; // Calculer les données brutes de la température
  temperature = (temp_counts * 200 / 16777215) - 50; // Calculer la température en °C
  percentage = (press_counts / 16777215) * 100; // Calculer le pourcentage de la pression mesurée en fonction de la plage de mesure
  pressure = ((press_counts - outputmin) * (pmax - pmin)) / (outputmax - outputmin) + pmin; // Formule pour calculer la pression, trouvable sur la datasheet du capteur
  dtostrf(press_counts, 4, 1, cBuff); 
  dtostrf(percentage, 4, 3, percBuff);
  dtostrf(pressure, 4, 3, pBuff);
  dtostrf(temperature, 4, 3, tBuff);

  sprintf(printBuffer, " % x\t % 2x % 2x % 2x\t % s\t % s\t % s\t % s \n", data[0], data[1], data[2], data[3], cBuff, percBuff, pBuff, tBuff); // Structuration des données dans un Serial
}
  
double getTemp(){return temp_counts;}
double getPres(){return pressure;}
double getPerc(){return percentage;}
double getPresRaw(){return press_counts;}

void afficher(int Time){
  Serial.print(pBuff);
  Serial.print(" KPa; ");
  Serial.print(percBuff);
  Serial.print(" %; ");
  Serial.print(tBuff);
  Serial.println(" °C; ");
  delay(Time);
}
void afficherPressure(int Time){
  Serial.print(pBuff);
  Serial.print(" KPa; ");
  Serial.print(percBuff);
  Serial.println(" %; ");
  delay(Time);
}
void afficherTemperature(int Time){
  Serial.print(tBuff);
  Serial.println(" °C; ");
  delay(Time);
}
void afficherPressureRaw(int Time){
  Serial.print(" Données du capteur : ");
  Serial.println(cBuff);
  delay(Time);
}
