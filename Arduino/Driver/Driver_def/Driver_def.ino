// incluir las librerias:

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Definir el uso de pines 
const int pinInterruptor1 = 2;
const int pinInterruptor2 = 3;
const int pinEnable = 12;
const int pinDirection = 11;
const int pinPulse = 10;
const int pinPotenciometro= A1;
const int pinPulsador = 8;
// declarar los distintos sensores:
LiquidCrystal_I2C lcd(0x27,16,2);

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor1, estadoInterruptor2, estadoPulsador;
unsigned long t0;
float pulso, t_delay,frecuencia,T;
int velocidad =0;
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(pinInterruptor1, INPUT);
pinMode(pinInterruptor2, INPUT);
pinMode(pinPulsador, INPUT);
pinMode(pinEnable, OUTPUT);
pinMode(pinDirection, OUTPUT);
pinMode(pinPulse, OUTPUT);
lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
lcd.backlight(); // activa la retroalimentación de la pantalla
lcd.setCursor(0,0);
lcd.print("Velocidad:");
}

void loop() {
  // put your main code here, to run repeatedly:






//Serial.println(T);
//if (millis()-t0 < 2000){ # se genera la señal cuadrada durante un tiempo arbitrario 
  digitalWrite(pinPulse, HIGH);
  delayMicroseconds(310);
  digitalWrite(pinPulse, LOW);
  delayMicroseconds(310);
//}

}
