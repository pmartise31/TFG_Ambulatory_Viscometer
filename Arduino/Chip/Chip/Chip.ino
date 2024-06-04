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
const int pinSDATA = 7;
const int pinFSYNC = 6;
const int pinSCLK = 13;
const int pinPotenciometro = A1; 
const int pinInterruptor3 = 8;

MD_AD9833 AD(pinFSYNC); // definimos el objeto con el que accederemos a la librería del chip
MD_AD9833::mode_t mode;
LiquidCrystal_I2C lcd(0x27,16,2);

//  AD(10);      //  HW SPI 

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor;
float frecuencia = 0; 
char freq[4];

void setup()
{
  Serial.begin(115200);
  pinMode(pinInterruptor3, INPUT);
  lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
  lcd.backlight(); // activa la retroalimentación de la pantalla
  lcd.setCursor(0,0);
  lcd.print("Frecuencia:");
  AD.begin();


}

void loop()
{
AD.setMode(MD_AD9833::MODE_OFF);
estadoInterruptor = digitalRead(pinInterruptor3);
if (estadoInterruptor == HIGH){                       // se entra en el modo setup
  frecuencia = map(analogRead(pinPotenciometro),0,1023,32,3200); //ajusto la salida del potenciometro --> se escoge el rango de velocidades
  dtostrf(frecuencia, 4, 0, freq); // transformación a caracteres 
  lcd.setCursor(12,0); 
  lcd.print(freq);
}
else{   // se entra en el modo run
AD.setMode(MD_AD9833::MODE_SQUARE1);
AD.setFrequency(0, frecuencia);
} 
}
 



//  -- END OF FILE --
