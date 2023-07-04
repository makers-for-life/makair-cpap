#include "PPC.h"

#define AIRFLOW_PERIOD 200000 // 5Hz
#define PRESSURE_PERIOD 1000000 //1Hz


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Airflow_init();
  Pressure_init();
  Blower_init();
  Airflow_Timer(AIRFLOW_PERIOD);
  Pressure_Timer(PRESSURE_PERIOD);
  IHM_Timer();
  PPC_init(0);
}

void loop() {
  // put your main code here, to run repeatedly:
//cmdAirflow(0,110, 10000);
//cmdVitesse();
//cmdPressure(0,20.5,50000,0.3);
//Test_caracteristique ();
//setVitesseCourbe(1620);
autoPilot(0,18,40000,0.2, 40); //double minimum, double maximum, int acceleration, double intervalle, int frequence
}
