#include <TimerOne.h> // Da installare tramite Sketch -> Include library -> Manages libraries...

#define pinPotenziometro A3
#define pinRele 12
#define pinETM 11 // ETM = Execution Time Measurement
#define setPoint 35
#define offPoint setPoint - 0
#define onPoint setPoint - 2

/* Si sceglie implementazione con variabili globali 
   Le variabili globali si indicano come volatile per disattivare 
   ottimizzazioni lettura/scrittura del compilatore */

volatile int temp_int;
volatile bool heater_on = true; //Predefinita accessione elemento riscaldante

// Variabile che viene modificata all'interno dell'ISR
volatile unsigned long count_secondi = 0;


void setup() 
{
  Serial.begin(9600); // inizializza il collegamento seriale
  // NON E' NECESSARIO INIZIALIZZARE IL PIN DEL CONVERTITORE ANALOGICO DIGITALE

  // configura i pin digitali
  pinMode(pinRele, OUTPUT); 
  pinMode(pinETM, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); 

  // Inizializza il Timer1
  // Il primo parametro è il periodo in microsecondi (1.000.000 us = 1 secondo)
  // Il secondo parametro è il nome della funzione ISR da eseguire
  Timer1.initialize(1000000); 
  Timer1.attachInterrupt(isr_1hz);
}

void loop() 
{

  Serial.print("Valore letto da ADC: ");
  Serial.print(temp_int); // Stampa il valore letto
  Serial.print(" dopo "); 
  Serial.print(count_secondi); // Stampa il numero di volte che è stata eseguita la ISR
  Serial.print(" s\n"); 


  delay(1000); // Aspetta 1000 millisecondi, ossia un secondo prima di fare un'altra misura
  //Attenzione: diversamente dal codice nella isr, il tempo di esecuzione non è completamente 
  //deterministico, in quanto dipende dalla  scrittura su seriale
}

// Questa è la Interrupt Service Routine (ISR)
// Deve essere veloce e non può usare funzioni che bloccano l'esecuzione (come delay() o Serial.print() dirette)
void isr_1hz() {
  // Raise ETM pin to measure execution time
  digitalWrite(pinETM, HIGH);

  // Inverti lo stato del LED per mostrare visivamente l'interruzione (usare per verificare ISR)
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  temp_int = readTemperature(); // legge il valore analogico del potenziometro
  
  DecisionLogic_Heater(); // "decide" se spegnere o accendere la resistenza

  updateHeaterStatus(); // aggiorna l'uscita fisica digitale dell'arduino

  // Incrementa il contatore
  count_secondi++;

  // Lowers ETM pin to measure execution time
  digitalWrite(pinETM, LOW);
}

//DECISION LOGIC
// Implementa il ciclo di isteresi tra offPoint e onPoint
void DecisionLogic_Heater()
{
  if(temp_int >= offPoint) 
  {
    heater_on = false;
  } else if (temp_int <= onPoint)
  {
    heater_on = true;
  }
  else 
  {
    ; //mantiene lo stato
  }
}
// DEVICE DRIVERS
// Leggi temperatura da LM-35 in analogico
int readTemperature()
{
  // Vout = 10 mV/°C \cdot T in [°C]. Offset temperature = 0
  return analogRead(pinPotenziometro)*512/1024; // legge il valore analogico del potenziometro
}

void updateHeaterStatus()
{
  if(heater_on)
  {
    digitalWrite(pinRele, LOW); // LOW level to turn ON
  }
  else
  {
    digitalWrite(pinRele, HIGH); // HIGH level to turn OFF
  }
}