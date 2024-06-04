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
const int pinInterruptor3 = 8;

MD_AD9833 AD(PIN_FSYNC); // definimos el objeto con el que accederemos a la librería del chip
LiquidCrystal_I2C lcd(0x27,16,2);

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor;
float frecuencia = 0, freq_init = 32, new_freq;
char freq[4];
int i=0;
int num_iteraciones = 50;
unsigned long t0 = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(pinInterruptor3, INPUT);
  lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
  lcd.backlight(); // activa la retroalimentación de la pantalla
  lcd.setCursor(0,0);
  lcd.print("Frecuencia:");
  AD.begin();
  AD.setMode(MD_AD9833::MODE_SQUARE1);

}

void loop()
{
estadoInterruptor = digitalRead(pinInterruptor3);
if (estadoInterruptor == HIGH){                       // se entra en el modo setup
  frecuencia = map(analogRead(pinPotenciometro),0,1023,32,3200); //ajusto la salida del potenciometro --> se escoge el rango de velocidades
  dtostrf(frecuencia, 4, 0, freq); // transformación a caracteres 
  lcd.setCursor(12,0); 
  lcd.print(freq);
}
else{   // se entra en el modo run:
 // se hace la comprobación cuando se usa la instrucción delay():
    for (int i=0; i <= num_iteraciones; i++){ 
      new_freq = freq_init + i*64;
      AD.setFrequency(MD_AD9833::CHAN_0, new_freq);
      delay(2000);
  }


} 
}



//  -- END OF FILE --
