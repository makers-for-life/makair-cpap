#include "blower.h"
#include "IHM.h"

    // Timer qui va générer le signal PPM
    HardwareTimer* timerPPM;

    /// Vitesse actuelle
    uint16_t speedBlower;

    /// Vitesse ciblée
    uint16_t targetSpeed;

static int vitesse = DEFAULT_BLOWER_SPEED;

void Blower_init(){
  pinMode(PIN_ESC_BLOWER, OUTPUT);
  timerPPM = new HardwareTimer(TIM4); //Sélectionne le timer 4 pour la pin D10
  timerPPM->setOverflow(ESC_PPM_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence de la PWM
  timerPPM->refresh(); //réninitialise le timer
  timerPPM->setMode(TIM_CHANNEL_ESC_BLOWER, TIMER_OUTPUT_COMPARE_PWM1, PIN_ESC_BLOWER); //Permet de mettre le mode PWM sur le timer (Autres modes : Input Capture/Ouput Capture/One-pulse
  // Set PPM width to 1ms
  timerPPM->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(DEFAULT_BLOWER_SPEED), MICROSEC_COMPARE_FORMAT); //Définit le rapport cyclique de la PWM
  timerPPM->resume(); //Lance le timer
}

void IHMBlower(){
  timerPPM->attachInterrupt(TIM_CHANNEL_ESC_BLOWER, expiration); //Active la fonction expiration à chaque interruption
}

void expiration() { //Fonction IHM qui permet de commander le blower
  vitesse = debounceLess(PA8,50,vitesse,10,MIN_BLOWER_SPEED);
  vitesse = debounceMore(PB5,50,vitesse,10,MAX_BLOWER_SPEED);
  setVitesse(vitesse);
}

uint16_t getVitesse(){return speedBlower;}

uint16_t getVitesseCible(){return targetSpeed;}

void setVitesse(uint16_t vitesseCible){
  if((vitesseCible >= 0) && (vitesseCible <= MAX_BLOWER_SPEED)){ 
  speedBlower = vitesseCible;
  timerPPM->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(speedBlower), MICROSEC_COMPARE_FORMAT);
 }
 else {
  Serial.println("Vitesse impossible");
 }
}

void setVitesseCourbe(uint16_t vitesseCourbe){
  if((vitesseCourbe >= 0) && (vitesseCourbe <= MAX_BLOWER_SPEED)){
  targetSpeed = vitesseCourbe;
  while(speedBlower <= vitesseCourbe){ //boucle de rampe de vitesse
    speedBlower++;
    timerPPM->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(speedBlower), MICROSEC_COMPARE_FORMAT);
    //delay(10);
  }
 }
 else {
  Serial.println("Vitesse impossible");
 }
}

void arret(){
  timerPPM->setCaptureCompare(TIM_CHANNEL_ESC_BLOWER, BlowerSpeed2MicroSeconds(0), MICROSEC_COMPARE_FORMAT);
  speedBlower = 0;
  targetSpeed = 0;
}

void afficherVitesse(int Time){
  Serial.print("vitesse : ");
  Serial.println(speedBlower);
  delay(Time);
}

void afficherVitesseCible(int Time){
  Serial.print("Vitesse ciblee : ");
  Serial.println(targetSpeed);
  delay(Time);
}
