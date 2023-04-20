#include <Arduino.h>
#include <LiquidCrystal.h>
#include "pressureSensor.h"
#include "airflowSensor.h"

#define IHM_PERIOD 200000
#define TIM_CHANNEL_IHM 1


double debounceLess(int buttonPin,int debounceTime, double value, double variation, double limite); //Fonction anti-rebond permettant de diminuer une valeur en appuyant sur le bouton
double debounceMore(int buttonPin,int debounceTime, double value, double variation, double limite); //Fonction anti-rebond permettant d'augmenter une valeur en appuyant sur le bouton
int debounceConfig(int buttonPin,int debounceTime, int etat, int etatfutur); //Fonction anti-rebond permettant de configurer une valeur en appuyant sur le bouton
void IHM_Timer();
void setAffichagex2(String nom, double valeur, String unite); //affichage sur la 4ème ligne du LCD
void setAffichagex3(String nom, double valeur, String unite); //affichage sur la 4ème ligne du LCD
void Affichage();
