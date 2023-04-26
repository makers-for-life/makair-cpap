#include "IHM.h"

HardwareTimer* hardwareTimer4;  // Timer Pressure command
LiquidCrystal lcd(PA10, PB4, PA9, PC7, PA7, PA6); //Attribution de l'Afficheur
///////Anti-rebond/////////
int reading, reading2, reading3 = 0;

int buttonState = 1;            // état courant du bouton
int lastButtonState = 0;  // état précédent du bouton
int buttonState2 = 1;
int lastButtonState2 = 0;
int buttonState3 = 1;
int lastButtonState3 = 0;

unsigned long lastDebounceTime = 0;  // le temps précédent du changement d'état du bouton
unsigned long debounceDelay = 0;    // Delai de l'anti rebond, l'augmenter si il y a encore des rebonds sur le bouton
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long debounceDelay3 = 0;
////////////////////////

static double less, more, configuration = 0; //Valeurs des modes des bouttons

//////Affichage///////
int affichage = 0;
String affichageNom = " ";
double affichageValeur = 0;
String affichageUnite = " ";
String affichageNom2 = " ";
double affichageValeur2 = 0;
String affichageUnite2 = " ";

double debounceLess(int buttonPin, int debounceTime, double value, double variation, double limite) {
  pinMode(buttonPin, INPUT_PULLUP);
  lastDebounceTime = debounceTime;
  reading = digitalRead(buttonPin);
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
        value -= variation;
      }
    }
  }
  lastButtonState = reading;
  if (value <= limite)  value = limite; //Limitation de la valeur min
  less = value;
  return less;
}

double debounceMore(int buttonPin, int debounceTime, double value, double variation, double limite) {
  pinMode(buttonPin, INPUT_PULLUP);
  lastDebounceTime2 = debounceTime;
  reading2 = digitalRead(buttonPin);
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
        value += variation;
      }
    }
  }
  lastButtonState2 = reading2;
  if (value >= limite)  value = limite; //Limitation de la valeur min
  more = value;
  return more;
}

int debounceConfig(int buttonPin, int debounceTime, int etat, int etatfutur) {
  pinMode(buttonPin, INPUT_PULLUP);
  lastDebounceTime3 = debounceTime;
  reading3 = digitalRead(buttonPin);
  if (reading3 != lastButtonState3) {
    // réinitialisation de l'anti-rebond
    lastDebounceTime3 = millis();
  }
  if ((millis() - lastDebounceTime3) > debounceDelay3) {
    // Condition du changement d'état du bouton
    if (reading3 != buttonState3) {
      buttonState3 = reading3;
      // Instruction exécutée si le bouton est LOW
      if (buttonState3 == 0) {
        etat = etatfutur;
      }
    }
  }
  lastButtonState3 = reading3;
  configuration = etat;
  return configuration;
}

void IHM_Timer() { // Cette fonction va permettre de générer le du timer pour la fonction du capteur de pression
  lcd.begin(20, 4); //Initialisation de l'afficheur LCD
  hardwareTimer4 = new HardwareTimer(TIM3); //Sélectionne le timer 2
  hardwareTimer4->setOverflow(IHM_PERIOD, MICROSEC_FORMAT); //Définit la période/fréquence du timer
  hardwareTimer4->refresh(); //réninitialise le timer
  hardwareTimer4->setMode(TIM_CHANNEL_IHM, TIMER_OUTPUT_COMPARE); //Permet de mettre le mode OC sur le timer (Autres modes : Input Capture/PWM/One-pulse
  hardwareTimer4->setCaptureCompare(TIM_CHANNEL_IHM, 0 , PERCENT_COMPARE_FORMAT); //Définit le début du compteur
  hardwareTimer4->attachInterrupt(TIM_CHANNEL_IHM, Affichage); //Active la fonction à chaque interruption
  hardwareTimer4->resume(); //Lance le timer
}

void setAffichagex2(String nom, double valeur, String unite) { //affichage sur la 3ème ligne du LCD
  affichage = 1;
  affichageNom = nom;
  affichageValeur = valeur;
  affichageUnite = unite;
}

void setAffichagex3(String nom, double valeur, String unite) { //affichage sur la 4ème ligne du LCD
  affichage = 2;
  affichageNom2 = nom;
  affichageValeur2 = valeur;
  affichageUnite2 = unite;
}

void Affichage() {
  lcd.setCursor(0, 0);
  lcd.print("Debit: ");
  lcd.print(getAirflow());
  lcd.print(" SLPM");
  lcd.setCursor(0, 1);
  lcd.print("Pression: ");
  lcd.print(getPres());
  lcd.print(" mbar");
  
  if (affichage >= 1) {
    lcd.setCursor(0, 2);
    lcd.print(affichageNom);
    lcd.print(affichageValeur);
    lcd.print(affichageUnite);
  }
  
  if (affichage == 2) {
    lcd.setCursor(0, 3);
    lcd.print(affichageNom2);
    lcd.print(affichageValeur2);
    lcd.print(affichageUnite2);
  }
  
}
