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
const int pinInterruptor3 = 4;
// declarar los distintos sensores:
LiquidCrystal_I2C lcd(0x27,16,2);

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor1, estadoInterruptor2, estadoInterruptor;
int  frecuencia, velocidad = 0;
float T_micros, T_milis; 
char vel[3];
char freq[5];
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(pinInterruptor1, INPUT);
pinMode(pinInterruptor2, INPUT);
pinMode(pinInterruptor3, INPUT);
pinMode(pinEnable, OUTPUT);
pinMode(pinDirection, OUTPUT);
pinMode(pinPulse, OUTPUT);
lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
lcd.backlight(); // activa la retroalimentación de la pantalla
lcd.setCursor(0,0);
lcd.print("Velocidad:");
lcd.setCursor(0,1);
lcd.print("Frecuencia:");
}

void loop() {
  // put your main code here, to run repeatedly:
estadoInterruptor = digitalRead(pinInterruptor3);
if (estadoInterruptor == HIGH){
  velocidad = map(analogRead(pinPotenciometro),0,1023,0,11); //ajusto la salida del potenciometro entre un rango de pulsos/revolucion
  dtostrf(velocidad, 3, 0, vel);
  lcd.setCursor(11,0);
  lcd.print(vel);
  frecuencia  = velocidad*(200.0/1.0);// esta relación indica que la frecuencia será el producto de la velocidad fijada por la relación de pulsos/revolución y mm/s
  dtostrf(frecuencia, 5, 0, freq);
  lcd.setCursor(12,1); 
  lcd.print(freq);
  T_micros = 500000/frecuencia; // se fija la mitad del periodo de la onda cuadrada en microsegundos, para frecuencias altas 
  T_milis = 500/frecuencia;// se fija la mitad del periodo de la onda cuadrada en milisegundos, para frecuencias bajas
  if (estadoInterruptor1 == HIGH){
    digitalWrite(pinEnable, LOW);
} else {
    digitalWrite(pinEnable, HIGH); 
}
if (estadoInterruptor2 == HIGH){
    digitalWrite(pinDirection, HIGH);
} else {
    digitalWrite(pinDirection, LOW);
  }
} 
else{
    if (frecuencia < 250) {
    digitalWrite(pinPulse, HIGH);
    delay(T_milis);
    digitalWrite(pinPulse, LOW);
    delay(T_milis);
} else {
  digitalWrite(pinPulse, HIGH);
    delayMicroseconds(T_micros);
    digitalWrite(pinPulse, LOW);
    delay(T_micros); 
}
}

}