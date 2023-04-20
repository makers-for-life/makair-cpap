#include<Arduino.h>
#include<Wire.h>
#include "HardwareTimer.h"
#include "pressureSensor.h"

#define PRESSURE_PERIOD 200000 // 5Hz
#define TIM_CHANNEL_PRESSURE 2 //Channel 2 de la pin D3

HardwareTimer* hardwareTimer3;  // Timer Pressure command

void setup() {

  Serial.begin(115200);
  
  while (!Serial) {
    delay(10);
  }
  Pressure_init();
  setTimerPressure();
}

void loop() {

}

void Pressure(){
Pressure_measure();
afficher();
}

void setTimerPressure(){
  hardwareTimer3 = new HardwareTimer(TIM2); //Sélectionne le timer 4 pour la pin D10
  hardwareTimer3->setOverflow(PRESSURE_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence de la PWM
  hardwareTimer3->refresh(); //réninitialise le timer
  hardwareTimer3->setMode(TIM_CHANNEL_PRESSURE, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode PWM sur le timer (Autres modes : Input Capture/Ouput Capture/One-pulse
  // Set PPM width to 1ms
  hardwareTimer3->setCaptureCompare(TIM_CHANNEL_PRESSURE, 0 , PERCENT_COMPARE_FORMAT); //Définit le rapport cyclique de la PWM
  hardwareTimer3->attachInterrupt(TIM_CHANNEL_PRESSURE, Pressure); //Active la fonction à chaque interruption
  hardwareTimer3->resume(); //Lance le timer
}
