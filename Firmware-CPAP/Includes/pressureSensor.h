#include<Arduino.h>
#include <HardwareTimer.h>

#define TIM_CHANNEL_PRESSURE 2 //Channel 1 de la pin D3
#define PRESSURE_SDA PB3 //I2C2
#define PRESSURE_SCL PB10 //I2C2

void Pressure_init(void); // Cette fonction va permettre d'initialiser le capteur de pression
void Pressure_Timer(int periode);
void Pressure_measure(); // Cette fonction va permettre de lire la pression mesurée par le capteur de pression

double getTemp(); //Obtenir la température
double getPres(); //Obtenir la pression
double getPerc(); //Obtenir le pourcentage de la pression mesurée en fonction de la plage de mesure
double getPresRaw(); //Obtenir les données brutes de la pression

void afficher(int Time); //Afficher toutes les données du capteur
void afficherPressure(int Time);
void afficherTemperature(int Time);
void afficherPressureRaw(int Time);
