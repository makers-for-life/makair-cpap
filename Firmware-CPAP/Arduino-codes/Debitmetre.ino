#define ESC_PPM_PERIOD 20000 // 50Hz
#define AIRFLOW_PERIOD 200000 // 5Hz
#define MIN_BLOWER_SPEED 300u
#define MAX_BLOWER_SPEED 1800u
#define DEFAULT_BLOWER_SPEED 900u
#define PIN_ESC_BLOWER  PB6  // D10 / TIM4_CH1
#define TIM_CHANNEL_ESC_BLOWER 1 //Channel 1 de la pin D10
#define TIM_CHANNEL_AIRFLOW 4 //Channel 4 de la pin D15
#define BlowerSpeed2MicroSeconds(value) map(value, 0, 1800, 1000, 1950) //Intervalle [1ms à 1,95 ms] sur la variation de vitesse du blower, on élargit cette intervalle pour avoir une plus grande plage de commande du Blower
#define MFM_HONEYWELL_HAF_I2C_ADDRESS 0x49
#define PIN_I2C_SDA PB9 //I2C1
#define PIN_I2C_SCL PB8 //I2C1

#include <Wire.h>
#include "HardwareTimer.h"

HardwareTimer* hardwareTimer1;  // Timer Blower command
HardwareTimer* hardwareTimer2;  // Timer Debitmetre command

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

void setup() {
  Serial.begin(115200);
  Wire.setSDA(PIN_I2C_SDA);
  Wire.setSCL(PIN_I2C_SCL);
  Wire.begin();
  sensor_Init();
  PWM_Blower();
  Airflow_Timer();
}

void loop() {

}

void airFlow() { // Cette fonction va permettre de lire le débit d'air mesuré par le débitmètre, elle va être activée dans un timer configuré
  Wire.begin();
  uint8_t readCount = Wire.requestFrom(MFM_HONEYWELL_HAF_I2C_ADDRESS, 2);
  mfmLastData.c[0] = Wire.read(); //Lecture des données
  mfmLastData.c[1] = Wire.read();
  Wire.end();

  mfmInspiratoryLastValue = (uint32_t)(mfmLastData.c[1] & 0xFFu); // masquage 0b00000000000000000000000011111111 & données du registre 1 (falcutatif)
  mfmInspiratoryLastValue |= (((uint32_t)mfmLastData.c[0]) << 8) & 0x0000FF00u; // masquage 0b00000000000000001111111100000000 & données du registre 0 (falcutatif)

  // Unité des données récupérées : Standard Litre Par Minute
  mfmInspiratoryAirFlow = 200 * (((uint32_t)mfmInspiratoryLastValue / 16384.0) - 0.1) / 0.8;

  // Autre formule : Unité des données récupérées : mSLPM
  //mfmInspiratoryAirFlow = (((10 * mfmInspiratoryLastValue) - 16384) * 1526);
  Serial.print("debit d'air : ");
  Serial.print(mfmInspiratoryAirFlow);
  Serial.println(" SLPM");
}
void sensor_Init() { // Cette fonction va permettre de lire le numéro de série du débitmètre, elle va être activée une seule fois dans setup()

  Wire.begin();
  Wire.beginTransmission(MFM_HONEYWELL_HAF_I2C_ADDRESS);
  Wire.write(0x02);  // Reset
  uint8_t txOk = Wire.endTransmission();
  Wire.end();
  delay(30);

  uint32_t sn = 0;
  Wire.begin();
  Wire.beginTransmission(MFM_HONEYWELL_HAF_I2C_ADDRESS);
  uint8_t rxcount = Wire.requestFrom(MFM_HONEYWELL_HAF_I2C_ADDRESS, 2); // Lire les 2 octets de données du capteur
  sn = Wire.read();
  sn <<= 8;
  sn |= Wire.read();  // Première transmission du numéro de série register 0
  sn <<= 8;
  delay(2);  // delay obligatoire pour recevoir le register 1
  rxcount += Wire.requestFrom(MFM_HONEYWELL_HAF_I2C_ADDRESS, 2);
  sn |= Wire.read();
  sn <<= 8;
  sn |= Wire.read();  // Seconde transmission du numéro de série register 1
  Serial.print("Numéro de série : ");
  Serial.println(sn);
  Wire.end();
}

void PWM_Blower() { // Cette fonction va permettre de générer le signal PPM qui va contrôler le blower, cette génération est sous la forme d'un timer pour créer un signal PWM
  pinMode(PIN_ESC_BLOWER, OUTPUT);
  hardwareTimer1 = new HardwareTimer(TIM4); //Sélectionne le timer 4 pour la pin D10
  hardwareTimer1->setOverflow(ESC_PPM_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence de la PWM
  hardwareTimer1->refresh(); //réninitialise le timer
  hardwareTimer1->setMode(TIM_CHANNEL_ESC_BLOWER, TIMER_OUTPUT_COMPARE_PWM1, PIN_ESC_BLOWER); //Permet de mettre le mode PWM sur le timer (Autres modes : Input Capture/Ouput Capture/One-pulse
  // Set PPM width to 1ms
  hardwareTimer1->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(DEFAULT_BLOWER_SPEED), MICROSEC_COMPARE_FORMAT); //Définit le rapport cyclique de la PWM
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
