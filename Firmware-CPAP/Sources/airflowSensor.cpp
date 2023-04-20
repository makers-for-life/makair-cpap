#include <Arduino.h>
#include <Wire.h>
#include "HardwareTimer.h"
#include "airflowSensor.h"

TwoWire myWire1(PIN_I2C_SDA, PIN_I2C_SCL);

HardwareTimer* hardwareTimer2;  // Timer Debitmetre command

int32_t mfmInspiratoryLastValue = 0; //données brutes du débit d'air
int32_t mfmInspiratoryAirFlow = 0; //données du débit d'air
union {
  uint16_t i;
  int16_t si;
  unsigned char c[2];
} mfmLastData;
uint32_t serialNumber;

void Airflow_init(){
  myWire1.begin();
  myWire1.beginTransmission(MFM_HONEYWELL_HAF_I2C_ADDRESS);
  myWire1.write(0x02);  // Reset
  uint8_t txOk = myWire1.endTransmission();
  myWire1.end();
  delay(30);

  uint32_t sn = 0;
  myWire1.begin();
  myWire1.beginTransmission(MFM_HONEYWELL_HAF_I2C_ADDRESS);
  uint8_t rxcount = myWire1.requestFrom(MFM_HONEYWELL_HAF_I2C_ADDRESS, 2); // Lire les 2 octets de données du capteur
  sn = myWire1.read();
  sn <<= 8;
  sn |= myWire1.read();  // Première transmission du numéro de série register 0
  sn <<= 8;
  delay(2);  // delay obligatoire pour recevoir le register 1
  rxcount += myWire1.requestFrom(MFM_HONEYWELL_HAF_I2C_ADDRESS, 2);
  sn |= myWire1.read();
  sn <<= 8;
  sn |= myWire1.read();  // Seconde transmission du numéro de série register 1
  Serial.print("Numéro de série : ");
  Serial.println(sn);
  myWire1.end();
}

void Airflow_Timer(int periode) { // Cette fonction va permettre de générer le timer pour la fonction du débitmètre
  hardwareTimer2 = new HardwareTimer(TIM1); //Sélectionne le timer 1 
  hardwareTimer2->setOverflow(periode, MICROSEC_FORMAT); //Définit la période/fréquence du timer
  hardwareTimer2->refresh(); //réninitialise le timer
  hardwareTimer2->setMode(TIM_CHANNEL_AIRFLOW, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode OC sur le timer (Autres modes : Input Capture/PWM/One-pulse
  hardwareTimer2->setCaptureCompare(TIM_CHANNEL_AIRFLOW, 0 , PERCENT_COMPARE_FORMAT); //Définit le début du compteur
  hardwareTimer2->attachInterrupt(TIM_CHANNEL_AIRFLOW, Airflow_read); //Active la fonction à chaque interruption
  hardwareTimer2->resume(); //Lance le timer
}

void Airflow_read(void){
  myWire1.begin();
  uint8_t readCount = myWire1.requestFrom(MFM_HONEYWELL_HAF_I2C_ADDRESS, 2);
  mfmLastData.c[0] = myWire1.read(); //Lecture des données
  mfmLastData.c[1] = myWire1.read();
  myWire1.end();

  mfmInspiratoryLastValue = (uint32_t)(mfmLastData.c[1] & 0xFFu); // masquage 0b00000000000000000000000011111111 & données du registre 1 (falcutatif)
  mfmInspiratoryLastValue |= (((uint32_t)mfmLastData.c[0]) << 8) & 0x0000FF00u; // masquage 0b00000000000000001111111100000000 & données du registre 0 (falcutatif)

  // Unité des données récupérées : Standard Litre Par Minute
  mfmInspiratoryAirFlow = 200 * (((uint32_t)mfmInspiratoryLastValue / 16384.0) - 0.1) / 0.8;

  // Autre formule : Unité des données récupérées : mSLPM
  //mfmInspiratoryAirFlow = (((10 * mfmInspiratoryLastValue) - 16384) * 1526);

}

uint32_t getSerialNumber(){return serialNumber;}
int32_t getAirflow(){return mfmInspiratoryAirFlow;}
int32_t getAirflowRaw(){return mfmInspiratoryLastValue;}

void afficherAirflow(int Time){
  Serial.print("debit d'air : ");
  Serial.print(mfmInspiratoryAirFlow);
  Serial.println(" SLPM");
  delay(Time);
}

void afficherAirflowRaw(int Time){
  Serial.print("données du capteur : ");
  Serial.print(mfmInspiratoryLastValue);
  delay(Time);
}

void afficherSerialNumber(int Time){
  Serial.print("Numéro de série : ");
  Serial.println(serialNumber);
  delay(Time);
}
