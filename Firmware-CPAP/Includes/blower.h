#include "Arduino.h"
#include "HardwareTimer.h"

#define ESC_PPM_PERIOD 20000 // 50Hz
#define MIN_BLOWER_SPEED 300u
#define MAX_BLOWER_SPEED 1800u
#define DEFAULT_BLOWER_SPEED 900u
#define PIN_ESC_BLOWER  PB6  // D10 / TIM4_CH1
#define TIM_CHANNEL_ESC_BLOWER 1 //Channel 1 de la pin D10
#define BlowerSpeed2MicroSeconds(value) map(value, 0, 1800, 1000, 1950) //Intervalle [1ms à 1,95 ms] sur la variation de vitesse du blower, on élargit cette intervalle pour avoir une plus grande plage de commande du Blower

  
    void Blower_init(); //Initialisation de la génération du signal PPM pour le blower
    void IHMBlower(); //Fonction prédéfini permettant de contrôler le blower avec 2 boutons
    void expiration(); //Fonction en sortie de IHMBlower
    
    uint16_t getVitesse();
    uint16_t getVitesseCible();

    void setVitesse(uint16_t vitesseCible); //Fonction permettant de configurer une vitesse fixe au blower
    void setVitesseCourbe(uint16_t vitesseCourbe); //Fonction permettant de configurer une rampe de vitesse au blower
    void arret(); //Fonction permettant d'arrêter le blower

    void afficherVitesse(int Time);
    void afficherVitesseCible(int Time);
