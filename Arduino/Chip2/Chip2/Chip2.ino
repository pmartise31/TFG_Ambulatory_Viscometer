//
//    FILE: AD9833_1000_Hz.ino
//  AUTHOR: Rob Tillaart
//    DATE: 2023-08-25
// PURPOSE: create a 1000 Hz reference signal

// descarga de las librerías a usar:
#include <MD_AD9833.h>
#include <SPI.h>   
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//definición de los pines: 
const uint8_t PIN_DATA = 11;	
const uint8_t PIN_CLK = 13;		
const uint8_t PIN_FSYNC = 10;
const int pinPotenciometro = A1; 
const int pinInterruptor1 = 8;
const int pinInterruptor2 = 6;
const int pinInterruptor3 = 7;


MD_AD9833 AD(PIN_FSYNC); // definimos el objeto con el que accederemos a la librería del chip
LiquidCrystal_I2C lcd(0x27,16,2); // definimos el objeto para usar la pantalla LCD

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor1, estadoInterruptor2, estadoInterruptor3;
float frecuencia = 0, freq_init = 32, new_freq;
char freq[4];
int i=0;
unsigned long t0;

void setup()
{
  Serial.begin(115200);
  // definición de los interruptores: 
  pinMode(pinInterruptor1, INPUT);
  pinMode(pinInterruptor2, INPUT);
  pinMode(pinInterruptor3, INPUT);
  lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
  lcd.backlight(); // activa la retroalimentación de la pantalla
  // Cosas que tiene que mostrar en la pantalla por defecto:
  lcd.setCursor(0,0);
  lcd.print("Frecuencia:");
  lcd.setCursor(0,1);
  lcd.print("Direction")
  // inicializar el chip y set el modo:
  AD.begin();
  AD.setMode(MD_AD9833::MODE_SQUARE1);
}

void loop()
{
// leer el valor de los interruptores:
estadoInterruptor1 = digitalRead(pinInterruptor1);
estadoInterruptor2 = digitalRead(pinInterruptor2);
estadoInterruptor3 = digitalRead(pinInterruptor3);
// condición modo SETUP:
if (estadoInterruptor1 == HIGH){                      
  frecuencia = map(analogRead(pinPotenciometro),0,1023,32,3200); //ajusto la salida del potenciometro --> se escoge el rango de velocidades
  dtostrf(frecuencia, 4, 0, freq); // transformación a caracteres 
  lcd.setCursor(12,0); 
  lcd.print(freq); // se imprime la frecuencia máxima
  // Se lee el estado del interruptor que marca la dirección del DRIVER y se imprime por pantalla:
  if (extadoInterruptor3 == HIGH){
    lcd.setCursor(12,1);
    lcd.print('>>>');
  }else{
    lcd.setCursor(12,1);
    lcd.print('<<<');
  }
  // se activa el DRIVER para volver a la posición inicial el émbolo o purgar:
  if (estadoInterruptor2 == HIGH){
    AD.setFrequency(MD_AD9833::CHAN_0, frecuencia);
  }
}
// condición modo RUN: 
else{   
for (int i=0; i <= 50; i++){
    new_freq = freq_init + i*64; // se pone una frecuencia nueva en función de la máxima puesta en el modo SETUP
    AD.setFrequency(MD_AD9833::CHAN_0, new_freq);
    delay(2000);
  }
}
} 
 



//  -- END OF FILE --
