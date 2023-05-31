#include <Arduino.h>

#include "airflowSensor.h"
#include "blower.h"
#include "IHM.h"
#include "pressureSensor.h"
#include "PPC.h"


int configuration = 0;
int vitesse = 0;
int vitessePilot = 0;
int Speed = map(vitesse, 180, 1800, 0, 100);
int seuil = 0;
double pressionLimite = 0.00;
int asservissement = 0;
int respiration = 0;
double traitement = 0;
int pilote = 1;

void PPC_init(int vitesseInit) {
  vitesse = map(vitesseInit, 0, 100, 180, 1800);
  setVitesse(vitesse);
}

void cmdVitesse() {
  Speed = map(vitesse, 180, 1800, 0, 100); //Valeur de la vitesse en % (0 à 100%)
  setAffichagex2("Vitesse: ", Speed, " %"); //Fonction de l'affichage de la vitesse sur la 3ème ligne du LCD
  configuration = debounceConfig(PA5, 50, configuration, 1); //Fonction de configuration pour changer d'état
  if (configuration == 1) {
    arret();
    vitesse = debounceMore(PB5, 50, vitesse, 16, 1800); //Fonction de configuration pour augmenter la valeur
    vitesse = debounceLess(PA8, 50, vitesse, 16, 180); //Fonction de configuration pour diminuer la valeur
    configuration = debounceConfig(PA5, 50, configuration, 2);
    Serial.println(vitesse);
  }
  else if (configuration == 2) {
    setVitesse(vitesse); //Mise en place de la nouvelle vitesse
    configuration = 0;
  }
}

void cmdAirflow(int minimun, int maximum, int acceleration) {
  setAffichagex2("Seuil: ", seuil, " SLPM"); //Fonction de l'affichage de la vitesse sur la 3ème ligne du LCD
  configuration = debounceConfig(PA5, 50, configuration, 1); //Fonction de configuration pour changer d'état
  if (configuration == 1) {
    arret();
    vitesse = 0;
    seuil = debounceMore(PB5, 50, seuil, 1, maximum); //Fonction de configuration pour augmenter la valeur
    seuil = debounceLess(PA8, 50, seuil, 1, minimun); //Fonction de configuration pour diminuer la valeur
    configuration = debounceConfig(PA5, 50, configuration, 2);
  }
  else if (configuration == 2) {
    while (seuil != getAirflow()) { //Boucle de régulation modifiant la vitesse jusqu'à atteindre le seuil configuré
      if (seuil > getAirflow()) vitesse++;
      if (seuil < getAirflow()) vitesse--;
      setVitesse(vitesse); //Mise en place de la nouvelle vitesse
      delayMicroseconds(acceleration);
    }
    asservissement++;
    if (asservissement >= 200) { //dès que le seuil est bien atteint, la configuration est finie
      configuration = 0;
      asservissement = 0;
    }
  }
}

void cmdPressure(double minimum, double maximum, int acceleration, double intervalle) {
  setAffichagex2("Limite: ", pressionLimite, " mbar"); //Fonction de l'affichage de la vitesse sur la 3ème ligne du LCD
  configuration = debounceConfig(PA5, 50, configuration, 1); //Fonction de configuration pour changer d'état
  if (configuration == 1) {
    arret();
    vitesse = 0;
    pressionLimite = debounceMore(PB5, 50, pressionLimite, 0.1 , maximum); //Fonction de configuration pour augmenter la valeur
    pressionLimite = debounceLess(PA8, 50, pressionLimite, 0.1, minimum); //Fonction de configuration pour diminuer la valeur
    configuration = debounceConfig(PA5, 50, configuration, 2);
  }
  else if (configuration == 2) {
    while ((pressionLimite - intervalle) >= getPres() || (pressionLimite + intervalle) <= getPres()) { //Boucle de régulation modifiant la vitesse jusqu'à atteindre le seuil configuré
      if (pressionLimite > getPres()) vitesse++;
      if (pressionLimite < getPres()) vitesse--;
      setVitesse(vitesse); //Mise en place de la nouvelle vitesse
      delayMicroseconds(acceleration);
    }
    asservissement++;
    if (asservissement >= 50) { //dès que le seuil est bien atteint, la configuration est finie
      configuration = 0;
      asservissement = 0;
    }
  }
}

void autoPilot(double minimum, double maximum, int acceleration, double intervalle, int frequence) {
  // commencer en appuyant sur le bouton, ensuite configurer la moyenne de la pression qui va sortir et appuyer sur le bouton, ensuite autopiloter la pression et le débit envoyé et enfin pouvoir appuyer sur le bouton pour reconfigurer
  Serial.println(configuration);

  if (configuration == 0) {
    arret();
    configuration = debounceConfig(PA5, 50, configuration, 1);
  }

  //  if (configuration > 0) {
  //    setAffichagex2("Seuil : ", pressionLimite, " mbar");
  //    setAffichagex3("Pilot : ", traitement, " mbar");
  //  }

  if (configuration == 1) {
    setAffichagex2("Seuil: ", pressionLimite, " mbar");
    pressionLimite = debounceMore(PB5, 50, pressionLimite, 0.5 , maximum);
    pressionLimite = debounceLess(PA8, 50, pressionLimite, 0.5, minimum);
    configuration = debounceConfig(PA5, 50, configuration, 2);
  }

  if (configuration == 2) {
    setAffichagex3("Pilot: ", traitement, " mbar");
    traitement = debounceMore(PB5, 50, traitement, 0.5 , pressionLimite);
    traitement = debounceLess(PA8, 50, traitement, 0.5, minimum);
    configuration = debounceConfig(PA5, 50, configuration, 3);
  }

  else if (configuration == 3) {
    while ((pressionLimite - intervalle) >= getPres() || (pressionLimite + intervalle) <= getPres()) { //Boucle modifiant la vitesse jusqu'à atteindre le seuil configuré
      if ((pressionLimite - intervalle) > getPres()) vitesse++;
      if ((pressionLimite + intervalle) < getPres()) vitesse--;
      setVitesse(vitesse);
      delayMicroseconds(acceleration);
    }
    asservissement++;
    if (asservissement >= 2000) { //dès que le seuil est bien atteint, la configuration est finie
      vitessePilot = vitesse;
      configuration = 4;
      seuil = getAirflow();
      asservissement = 0;
    }
  }
  else if (configuration == 4) {
    configuration = debounceConfig(PA5, 50, configuration, 5);
    if (seuil - 5 >= getAirflow()) {
      respiration++;
      delay(100);
    }
    if (respiration >= frequence && pilote == 1) {
      while ((traitement - intervalle) >= getPres() || (traitement + intervalle) <= getPres()) { //Boucle modifiant la vitesse jusqu'à atteindre le seuil configuré
        if ((traitement - intervalle) > getPres()) vitessePilot++;
        if ((traitement + intervalle) < getPres()) vitessePilot--;
        setVitesse(vitessePilot);
        delayMicroseconds(acceleration);
      }
      asservissement++;
    }

    if (asservissement >= 2000 && pilote == 1) { //dès que le seuil est bien atteint, la configuration est finie
      asservissement = 0;
      seuil = getAirflow();
      respiration = 0;
      pilote = 0;
    }

    if (respiration >= frequence && pilote == 0) {
      while ((pressionLimite - intervalle) >= getPres() || (pressionLimite + intervalle) <= getPres()) { //Boucle modifiant la vitesse jusqu'à atteindre le seuil configuré
        if ((pressionLimite - intervalle) > getPres()) vitessePilot++;
        if ((pressionLimite + intervalle) < getPres()) vitessePilot--;
        setVitesse(vitessePilot);
        delayMicroseconds(acceleration);
      }
      asservissement++;
    }

    if (asservissement >= 3000 && pilote == 0) { //dès que le seuil est bien atteint, la configuration est finie
      asservissement = 0;
      seuil = getAirflow();
      respiration = 0;
      pilote = 1;
    }
  }
  else if (configuration == 5) {
    arret();
    asservissement = 0;
    respiration = 0;
    configuration = 1;
  }
}

//void asservissement(){
//      // Calcul de l'erreur
//    int frequence_codeuse = frequence_echantillonnage*tick_codeuse;
//    float nb_tour_par_sec = 50/(float)tick_par_tour_codeuse/(float)rapport_reducteur;
//    float erreur = consigne_moteur_nombre_tours_par_seconde - nb_tour_par_sec;
//
//    // Réinitialisation du nombre de tick de la codeuse
//    tick_codeuse=0;
//
//    // P : calcul de la commande
//    vitesse = kp*erreur;
//
//    // Normalisation et contrôle du moteur
//    if(vitesse <= 300) vitesse = 300;
//    else if(vitesse >= 1800) vitesse = 1800;
//    setVitesse(vitesse);
//
//    // DEBUG
//    /*
//    Serial.print(nb_tour_par_sec,8);
//    Serial.print(" : ");
//    Serial.print(erreur,4);
//    Serial.println();
//    //*/
//}

void Test_caracteristique () {
  if (asservissement <= 100) {
    if (vitesse <= 300) vitesse = 300;
    if (vitesse >= 1800) vitesse = 1800;
    vitesse += 15;
    setVitesse(vitesse);
    Serial.println(getAirflow());
    delay(500);
    asservissement++;
  }
  else {
    arret();
  }
}
