#include <Arduino.h>
#include <Wire.h>
#include "HardwareTimer.h"
#include "airflowSensor.h"

#define AIRFLOW_PERIOD 200000 // 5Hz
#define TIM_CHANNEL_AIRFLOW 4 //Channel 4 de la pin D15

HardwareTimer* hardwareTimer2;  // Timer Debitmetre command


void setup() {
  Serial.begin(115200);
  Airflow_init();
  Airflow_Timer();
}

void loop() {

}

void airFlow(){
  Airflow_read();
  afficherAirflow();
}

void Airflow_Timer() {
  hardwareTimer2 = new HardwareTimer(TIM1); //Sélectionne le timer 4 pour la pin D10
  hardwareTimer2->setOverflow(AIRFLOW_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence de la PWM
  hardwareTimer2->refresh(); //réninitialise le timer
  hardwareTimer2->setMode(TIM_CHANNEL_AIRFLOW, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode PWM sur le timer (Autres modes : Input Capture/Ouput Capture/One-pulse
  // Set PPM width to 1ms
  hardwareTimer2->setCaptureCompare(TIM_CHANNEL_AIRFLOW, 0 , PERCENT_COMPARE_FORMAT); //Définit le rapport cyclique de la PWM
  hardwareTimer2->attachInterrupt(TIM_CHANNEL_AIRFLOW, airFlow); //Active la fonction à chaque interruption
  hardwareTimer2->resume(); //Lance le timer
}
