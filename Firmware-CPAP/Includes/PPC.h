#include <Arduino.h>

#include "airflowSensor.h"
#include "blower.h"
#include "IHM.h"
#include "pressureSensor.h"

void PPC_init(int vitesseInit);
void cmdVitesse();
void cmdAirflow(int minimun, int maximum, int acceleration);
void cmdPressure(double minimum, double maximum, int acceleration, double intervalle);
void autoPilot(double minimum, double maximum, int acceleration, double intervalle, int frequence);
//void asservissement();

void Test_caracteristique ();
