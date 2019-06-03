#include <LiquidCrystal.h>
#include "Encoder.h"

// Inicializar pantalla LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int pinBMas = 8; 
int pinBMenos = 7;
int pwmPin = 6;
int tPin = A0;
float t;
float tInput = 20;

unsigned long startMillis;
unsigned long currentMillis;

//Asignar los pines al Encoder de Encoder.h
Encoder ruedita(pinBMas, pinBMenos);

//Función inicial para asignar tiempo inicial, y modos de los pines
void setup(){
  Serial.begin(9600);
  lcd.begin(8,2);
  
  startMillis = millis();
  
  pinMode(pinBMenos, INPUT);
  digitalWrite(pinBMenos, HIGH);
  pinMode(pinBMas, INPUT);
  digitalWrite(pinBMas, HIGH);
}


void loop(){
  t = get_temp();
  //Corrigiendo el valor de read() para que nos de 0
  tInput = 20.0 + ruedita.read()*0.1;
  //La diferencia de la temperatura deseada y la temperatura real
  float error = t - tInput;
  //Ajustando el valor del ventilador proporcionalmente
  int fanSpeed = 50*error;
  //Esta función actua como tope para valores fuera del rango 0-255
  fanSpeed = constrain(fanSpeed, 0, 255); 
  //Invertir proporcionalmente el valor para que lo lea el ventilador
  fanSpeed = map(fanSpeed, 0, 255, 255, 0); 
  //Escribir valor en el ventilador
  analogWrite(pwmPin, fanSpeed);
  //Actualizar el valor de la pantalla cada 200 ms
  currentMillis = millis();
  if((currentMillis - startMillis) > 200){
    startMillis = currentMillis;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(t);lcd.print("C ");
    lcd.setCursor(0,1);
    lcd.print(tInput);lcd.print("C ");
    lcd.display();
  }
}
//Función para obtener el valor de la temperatura actual
float get_temp (void){
  int sensorValue = analogRead(A0);
  //Transformación del valor del sensor a un valor de voltaje
  float voltage = sensorValue * (5.0 / 1023.0);
  //Transformación del voltaje en temperatura
  return (voltage / 10.4) * 100;
}
