#include <Arduino.h>

#define MFM_HONEYWELL_HAF_I2C_ADDRESS 0x49 //adresse du débitmètre
#define TIM_CHANNEL_AIRFLOW 4 //Channel 1 de la pin D15
#define PIN_I2C_SDA PB9 //I2C1
#define PIN_I2C_SCL PB8 //I2C1 

void Airflow_init(); //Fonction d'iniatialisation du débitmètre
void Airflow_Timer(int periode);
void Airflow_read(void); //Fonction de lecture du débit d'air mesuré par le débitmètre

uint32_t getSerialNumber(void);
int32_t getAirflow(void);
int32_t getAirflowRaw(void); 

void afficherAirflow(int Time); //Serial : Fonction affichant le débit d'air
void afficherAirflowRaw(int Time); //Serial : Fonction affichant les données brutes du débit d'air
void afficherSerialNumber(int Time); //Serial : Fonction affichant le numéro de série du débitmètre
