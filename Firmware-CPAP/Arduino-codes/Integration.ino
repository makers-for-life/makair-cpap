#define ESC_PPM_PERIOD 20000 // 50Hz
#define AIRFLOW_PERIOD 100000 // 10Hz
#define PRESSURE_PERIOD 200000 // 5Hz
#define MIN_BLOWER_SPEED 300u
#define MAX_BLOWER_SPEED 1800u
#define DEFAULT_BLOWER_SPEED 900u
#define PIN_ESC_BLOWER  PB6  // D10 / TIM4_CH1
#define TIM_CHANNEL_ESC_BLOWER 1 //Channel 1 de la pin D10
#define TIM_CHANNEL_AIRFLOW 4 //Channel 4 de la pin D15
#define TIM_CHANNEL_PRESSURE 2 //Channel 2 de la pin D3
#define BlowerSpeed2MicroSeconds(value) map(value, 0, 1800, 1000, 1950) //Intervalle [1ms à 1,95 ms] sur la variation de vitesse du blower, on élargit cette intervalle pour avoir une plus grande plage de commande du Blower
#define MFM_HONEYWELL_HAF_I2C_ADDRESS 0x49 //Adresse du debitmetre
#define PIN_I2C_SDA PB9 //I2C1
#define PIN_I2C_SCL PB8 //I2C1
#define PRESSURE_SDA PB3 //I2C2
#define PRESSURE_SCL PB10 //I2C2

#include <Wire.h>
#include "HardwareTimer.h"
#include <LiquidCrystal.h>

HardwareTimer* hardwareTimer1;  // Timer Blower command
HardwareTimer* hardwareTimer2;  // Timer Debitmetre command
HardwareTimer* hardwareTimer3;  // Timer Pressure command

LiquidCrystal lcd(PA10, PB4, PA9, PC7, PA7, PA6); //Attribution de l'Afficheur

//Communication I2C multi-slaves
TwoWire myWire1(PIN_I2C_SDA, PIN_I2C_SCL); 
TwoWire myWire2(PRESSURE_SDA, PRESSURE_SCL);

////Variables Blower////
static int speedBlower = MAX_BLOWER_SPEED;

////Variables Débitmètre////
int32_t mfmInspiratoryLastValue = 0; //données brutes du débit d'air
int32_t mfmInspiratoryAirFlow = 0; //données du débit d'air
union {
  uint16_t i;
  int16_t si;
  unsigned char c[2];
} mfmLastData;

////Variables Pressure////
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


void setup() {
  Serial.begin(115200);
  myWire1.begin(); //Initialisation de la communication I2C avec le débitmètre
  myWire2.begin();//Initialisation de la communication I2C avec le capteur de pression
  lcd.begin(20, 4); //Initialisation de l'afficheur LCD
  sensor_Init();
  PWM_Blower();
  Airflow_Timer();
  Pressure_Timer();
}

void loop() {

}

void airFlow() { // Cette fonction va permettre de lire le débit d'air mesuré par le débitmètre, elle va être activée dans un timer configuré
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
  Serial.print("debit d'air : "); 
  Serial.print(mfmInspiratoryAirFlow);
  Serial.println(" SLPM");
  lcd.setCursor(0, 0); //affichage du débit sur le LCD 
  lcd.print("Airflow : ");
  lcd.print(mfmInspiratoryAirFlow);
  lcd.print(" SLPM");
}
void sensor_Init() { // Cette fonction va permettre de lire le numéro de série du débitmètre, elle va être activée une seule fois dans setup()

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

void Pressure() { // Cette fonction va permettre de lire la pression mesurée par le capteur de pression, elle va être activée dans un timer configuré
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
  pressure = ((press_counts - outputmin) * (pmax - pmin)) / (outputmax - outputmin) + pmin;  // Formule pour calculer la pression, trouvable sur la datasheet du capteur
  dtostrf(press_counts, 4, 1, cBuff); 
  dtostrf(percentage, 4, 3, percBuff);
  dtostrf(pressure, 4, 3, pBuff);
  dtostrf(temperature, 4, 3, tBuff);

  sprintf(printBuffer, " % x\t % 2x % 2x % 2x\t % s\t % s\t % s\t % s \n", data[0], data[1], data[2], data[3], cBuff, percBuff, pBuff, tBuff); //Structuration des données dans un Serial
  Serial.print(pBuff);//affichage de la pression
  Serial.print(" mbar; ");
  Serial.print(percBuff); //affichage du pourcentage de la pression mesurée en fonction de la plage de mesure
  Serial.println(" % ");
  lcd.setCursor(0, 1);//affichage de la pression sur le LCD 
  lcd.print("Pressure : ");
  lcd.print(pBuff);
  lcd.print(" mbar");
  delay(10);
}

void PWM_Blower() { // Cette fonction va permettre de générer le signal PPM qui va contrôler le blower, cette génération est sous la forme d'un timer pour créer un signal PWM
  pinMode(PIN_ESC_BLOWER, OUTPUT);
  hardwareTimer1 = new HardwareTimer(TIM4); //Sélectionne le timer 4 pour la pin D10
  hardwareTimer1->setOverflow(ESC_PPM_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence de la PWM
  hardwareTimer1->refresh(); //réninitialise le timer
  hardwareTimer1->setMode(TIM_CHANNEL_ESC_BLOWER, TIMER_OUTPUT_COMPARE_PWM1, PIN_ESC_BLOWER); //Permet de mettre le mode PWM sur le timer (Autres modes : Input Capture/Ouput Capture/One-pulse
  // Set PPM width to 1ms
  hardwareTimer1->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(500), MICROSEC_COMPARE_FORMAT); //Définit le rapport cyclique de la PWM
  hardwareTimer1->attachInterrupt(TIM_CHANNEL_ESC_BLOWER, expiration); //Active la fonction à chaque interruption
  hardwareTimer1->resume(); //Lance le timer
}

void Airflow_Timer() { // Cette fonction va permettre de générer le timer pour la fonction du débitmètre
  hardwareTimer2 = new HardwareTimer(TIM1); //Sélectionne le timer 1 
  hardwareTimer2->setOverflow(AIRFLOW_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence du timer
  hardwareTimer2->refresh(); //réninitialise le timer
  hardwareTimer2->setMode(TIM_CHANNEL_AIRFLOW, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode OC sur le timer (Autres modes : Input Capture/PWM/One-pulse
  hardwareTimer2->setCaptureCompare(TIM_CHANNEL_AIRFLOW, 0 , PERCENT_COMPARE_FORMAT); //Définit le début du compteur
  hardwareTimer2->attachInterrupt(TIM_CHANNEL_AIRFLOW, airFlow); //Active la fonction à chaque interruption
  hardwareTimer2->resume(); //Lance le timer
}

void Pressure_Timer() { // Cette fonction va permettre de générer le du timer pour la fonction du capteur de pression
  hardwareTimer3 = new HardwareTimer(TIM2); //Sélectionne le timer 2
  hardwareTimer3->setOverflow(PRESSURE_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence du timer
  hardwareTimer3->refresh(); //réninitialise le timer
  hardwareTimer3->setMode(TIM_CHANNEL_PRESSURE, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode OC sur le timer (Autres modes : Input Capture/PWM/One-pulse
  hardwareTimer3->setCaptureCompare(TIM_CHANNEL_PRESSURE, 0 , PERCENT_COMPARE_FORMAT); //Définit le début du compteur
  hardwareTimer3->attachInterrupt(TIM_CHANNEL_PRESSURE, Pressure); //Active la fonction à chaque interruption
  hardwareTimer3->resume(); //Lance le timer
}
