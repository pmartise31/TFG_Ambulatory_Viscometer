// incluir las librerias:

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Definir el uso de pines 
const int pinEnable = 12;
const int pinDirection = 11;
const int pinPulse = 10;
const int pinPotenciometro= A1;
const int pinInterruptor3 = 8;
LiquidCrystal_I2C lcd(0x27,16,2); // para poder usar la pantalla LCD

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor;
float T_micros, T_milis,frecuencia, velocidad = 0; // importante definir todas estas variables como floats, sino el código no funciona 
char vel[3];
char freq[4]; // para poder hacer el display en el LCD sin tener problemas de superposición de dígitos hay que transformarlos a caracteres

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
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
estadoInterruptor = digitalRead(pinInterruptor3); // se lee el estado del interruptor de control
if (estadoInterruptor == HIGH){                       // se entra en el modo setup
  velocidad = map(analogRead(pinPotenciometro),0,1023,10,1000); //ajusto la salida del potenciometro --> se escoge el rango de velocidades
  dtostrf(velocidad, 3, 0, vel); // transformación a caracteres 
  lcd.setCursor(11,0); 
  lcd.print(vel);
  frecuencia  = 3.2*velocidad;// esta relación indica que la frecuencia será el producto de la velocidad fijada por la relación de pulsos/revolución y mm/s
  dtostrf(frecuencia, 4, 0, freq);
  lcd.setCursor(12,1); 
  lcd.print(freq);
  T_micros = 500000/frecuencia; // se fija la mitad del periodo de la onda cuadrada en microsegundos, para frecuencias altas 
} 
  else{   // se entra en el modo run
      digitalWrite(pinPulse, HIGH);
      delayMicroseconds(T_micros);
      digitalWrite(pinPulse, LOW);
      delayMicroseconds(T_micros);
}

}