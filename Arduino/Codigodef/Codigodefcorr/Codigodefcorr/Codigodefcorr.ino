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
float freq_init, new_freq, freq_max= 0; // valor de la frecuencia en float para que pueda ser aceptado por las funciones de la librería
char freq[4];
int contador=1;  // valor inicial del contador a 1
int n_puntos= 32; // valor del número de valores que se generarán en la rampa 
unsigned long t0 = 0; 

void setup()
{
  Serial.begin(115200);
  pinMode(pinInterruptor3, INPUT);
  lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
  lcd.backlight(); // activa la retroalimentación de la pantalla
  lcd.setCursor(0,0);
  lcd.print("Frecuencia:");
  AD.begin(); // incializa el chip AD_9833
  AD.setMode(MD_AD9833::MODE_SQUARE1); // marca en el chip el tipo de señal que se debe emplear, en este caso una señal cuadrada 

}

void loop()
{
estadoInterruptor = digitalRead(pinInterruptor3);
if (estadoInterruptor == HIGH){                       // se entra en el modo setup
  freq_max = map(analogRead(pinPotenciometro),0,1023,32,3200); //ajusto la salida del potenciometro --> se escoge el rango de velocidades
  dtostrf(freq_max, 4, 0, freq); // transformación a caracteres 
  lcd.setCursor(12,0); 
  lcd.print(freq);
  freq_init = freq_max/n_puntos; // la frecuencia inicial será aquella que equivalente a la frecuencia máxima 
}
else{   // se entra en el modo run:
 // se hace la comprobación cuando se usa la intrucción millis(): 
    if (millis()-t0 >= 2000){ //  se hace para conseguir un cambio gradual de frecuencias cada 2 segundos con la instrucción millis()
      new_freq = freq_init*contador; // aquí se hace que coincida el valor mínimo de la frecuencia con el paso de frecuencias en la rampa
      //Serial.print("Frecuencia:"); 
      //Serial.println(new_freq);
      AD.setFrequency(0, new_freq); // se envía la frecuencia en cada punto al chip
      if(new_freq <= freq_max){ // se limita el valor de la frecuencia por debajo de la frecuencia máxima entregada por el potenciometro
        t0 = millis();
        if (contador < n_puntos){
        contador = contador+1; // se añade el valor de 1 al contador para seguir aumentando la frecuencia de 32 Hz hasta llegar a 3200
        //Serial.print("Cambio en el contador: ");
        //Serial.println(contador);
        }else{
          return;
        }
     }else {
        return; // acaba el loop

    }
  }

}
} 




//  -- END OF FILE --
