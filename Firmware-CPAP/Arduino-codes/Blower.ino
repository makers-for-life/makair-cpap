#define ESC_PPM_PERIOD 20000 // 50Hz
#define MIN_BLOWER_SPEED 300u
#define MAX_BLOWER_SPEED 1800u
#define DEFAULT_BLOWER_SPEED 900u
#define PIN_ESC_BLOWER  PB6  // D10 / TIM4_CH1
#define TIM_CHANNEL_ESC_BLOWER 1 //Channel 1 de la pin D10
#define BlowerSpeed2MicroSeconds(value) map(value, 0, 1800, 1000, 1950) //Intervalle [1ms à 1,95 ms] sur la variation de vitesse du blower, on élargit cette intervalle pour avoir une plus grande plage de commande du Blower
#include "HardwareTimer.h"
#include <LiquidCrystal.h>

HardwareTimer* hardwareTimer1;  // Timer Blower command

LiquidCrystal lcd(PA10, PB4, PA9, PC7, PA7, PA6);

////Variables Blower////
static int speedBlower = DEFAULT_BLOWER_SPEED;

///////Anti-rebond/////////
int reading, reading2 = 0;
int buttonState = 1;            // état courant du bouton 
int lastButtonState = 0;  // état précédent du bouton
int buttonState2 = 1;            
int lastButtonState2 = 0;  
unsigned long lastDebounceTime = 50;  // le temps précédent du changement d'état du bouton
unsigned long debounceDelay = 0;    // Delai de l'anti rebond, l'augmenter si il y a encore des rebonds sur le bouton
unsigned long lastDebounceTime2 = 50;  
unsigned long debounceDelay2 = 0;   
////////////////////////

void setup() {
  Serial.begin(115200);
  lcd.begin(20, 4);
  pinMode(PA8, INPUT_PULLUP); //D7
  pinMode(PB5, INPUT_PULLUP); //D4
  PWM_Blower();
}

void loop() {

}

void PWM_Blower() { // Cette fonction va permettre de générer le signal PPM qui va contrôler le blower, cette génération est sous la forme d'un timer pour créer un signal PWM
  pinMode(PIN_ESC_BLOWER, OUTPUT);
  hardwareTimer1 = new HardwareTimer(TIM4); //Sélectionne le timer 4 pour la pin D10
  hardwareTimer1->setOverflow(ESC_PPM_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence de la PWM
  hardwareTimer1->refresh(); //réninitialise le timer
  hardwareTimer1->setMode(TIM_CHANNEL_ESC_BLOWER, TIMER_OUTPUT_COMPARE_PWM1, PIN_ESC_BLOWER); //Permet de mettre le mode PWM sur le timer (Autres modes : Input Capture/Ouput Capture/One-pulse
  // Set PPM width to 1ms
  hardwareTimer1->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(DEFAULT_BLOWER_SPEED), MICROSEC_COMPARE_FORMAT); //Définit le rapport cyclique de la PWM
  hardwareTimer1->attachInterrupt(TIM_CHANNEL_ESC_BLOWER, expiration); //Active la fonction à chaque interruption
  hardwareTimer1->resume(); //Lance le timer
}

void expiration() {
  reading = digitalRead(PA8);
  if (reading != lastButtonState) {
    // réinitialisation de l'anti-rebond
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Condition du changement d'état du bouton
    if (reading != buttonState) {
      buttonState = reading;
      // Instruction exécutée si le bouton est LOW
      if (buttonState == 0) {
        speedBlower -= 10;
      }
    }
  }
  lastButtonState = reading;

  reading2 = digitalRead(PB5);
  if (reading2 != lastButtonState2) {
    // réinitialisation de l'anti-rebond
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay2) {
    // Condition du changement d'état du bouton
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      // Instruction exécutée si le bouton est LOW
      if (buttonState2 == 0) {
        speedBlower += 10;
      }
    }
  }
  lastButtonState2 = reading2;

  if (speedBlower <= 100)  speedBlower = 100; //Limitation de la valeur min
  if (speedBlower >= 1800)  speedBlower = 1800; //Limitation de la valeur max
  lcd.setCursor(0, 0);
  lcd.print(speedBlower);
  hardwareTimer1->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(speedBlower), MICROSEC_COMPARE_FORMAT); //Changement du rapport cyclique
}
