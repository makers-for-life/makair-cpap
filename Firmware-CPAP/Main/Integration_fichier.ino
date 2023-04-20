#define AIRFLOW_PERIOD 200000 // 5Hz
#define PRESSURE_PERIOD 1000000 //1Hz

#include <LiquidCrystal.h>

#include "airflowSensor.h"
#include "pressureSensor.h"
#include "blower.h"
#include "IHM.h"

LiquidCrystal lcd(PA10, PB4, PA9, PC7, PA7, PA6); //Attribution de l'Afficheur

int configuration = 0;
int vitesse = DEFAULT_BLOWER_SPEED;
int Speed = 0;

void setup() {
  Serial.begin(115200);
  lcd.begin(20, 4); //Initialisation de l'afficheur LCD
  Airflow_init();
  Pressure_init();
  Blower_init();
  Airflow_Timer(AIRFLOW_PERIOD);
  Pressure_Timer(PRESSURE_PERIOD);
  Speed = map(vitesse, 300, 1800, 0, 100);
}

void loop() {
lcd.setCursor(0,0);
lcd.print("Debit : ");
lcd.print(getAirflow());
lcd.print(" SLPM");
lcd.setCursor(0,1);
lcd.print("Pression : ");
lcd.print(getPres());
lcd.print(" mbar");
lcd.setCursor(0,2);
lcd.print("Vitesse : ");
lcd.print(Speed);
lcd.print(" %");
configuration = debounceConfig(PC13,50,configuration,1);
if(configuration == 1){
  arret();
  vitesse = debounceMore(PB5,50,vitesse,15,1800); 
  vitesse = debounceLess(PA8,50,vitesse,15,300);
  configuration = debounceConfig(PC13,50,configuration,2);
  Speed = map(vitesse, 300, 1800, 0, 100);
  Serial.println(vitesse);
 }
else if(configuration == 2){
  setVitesse(vitesse);
  configuration = 0;
 }
}
