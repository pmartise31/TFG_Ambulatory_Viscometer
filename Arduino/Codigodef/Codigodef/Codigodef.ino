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
#define Max_size 32 // tamaño de los arrays de velocidad y de presion
//definición de los pines: 
const uint8_t PIN_DATA = 11;	
const uint8_t PIN_CLK = 13;		
const uint8_t PIN_FSYNC = 10;
const int pinPotenciometro = A1; 
const int pinInterruptor1 = 8;
const int pinInterruptor2 = 6;
const int pinInterruptor3 = 7;
const int pinPresion = A0; 



MD_AD9833 AD(PIN_FSYNC); // definimos el objeto con el que accederemos a la librería del chip
LiquidCrystal_I2C lcd(0x27,16,2); // definimos el objeto para usar la pantalla LCD

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor1, estadoInterruptor2, estadoInterruptor3;
float frecuencia = 0, presion = 0, freq_init = 32;
char freq[4];
int i=0;
unsigned long t0;
float v_data[] = {Max_size};
float P_data[] = {Max_size};
float new_freq, K, tau_0, G_prime, G_2prime, freq_max= 0; 
int contador=1, n_data=0, n_puntos= 32;

// Vamos a definir las funciones de los parámetros que nos serán útiles para calcula G' y G'', así como dar la K (constante del modelo) y viscosidad aparente del esputo:

// Función para la K(coeficiente del modelo de Casson)
float K_f(){
  float v_sum=0, P_sum=0;
  for(int i=0; i<n_puntos; i++){
    v_sum += v_data[i];
    P_sum += P_data[i];
  }
  float K = P_sum/v_sum;
  return K;
}

// Función para la viscosidad aparente
float tau_0_f(float K){
  float tanh_sum=0;
  for(int i=0; i<n_puntos; i++){
    tanh_sum += tanh(sqrt(v_data[i]/K));    
  }
  float tau_0 = 0.5 * K*sqrt(tanh_sum/n_puntos);
  return tau_0;
}

// Función para calcular G'
float G_prime_f(float K, float tau_0){
  float sum_diff = 0;
  for (int i=0; i< n_puntos; i++){
    sum_diff += P_data[i] - tau_0*sqrt(v_data[i]/K);
  }
  float G_prime = sum_diff / n_puntos;
  return G_prime;
  }

// Función para calcular G_2prime
float G_2prime_f(float K, float tau_0){
  float diff_sum_squared = 0;
  for (int i=0; i< n_puntos; i++){
  diff_sum_squared += pow(P_data[i]-tau_0* sqrt(v_data[i]/K), 2);
  }
  float G_2prime = diff_sum_squared/n_puntos;
  return G_2prime;
}


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
  lcd.print("Direction");
  // inicializar el chip y set el modo:
  AD.begin();
  AD.setMode(MD_AD9833::MODE_SQUARE1);
}
// Módulo creado para poder ir añadiendo los valores de presión y de velocidad en las matrices:
void push_back(float valor_v, float valor_P) {
  if (n_data < Max_size){
    v_data[n_data] = valor_v; 
    P_data[n_data] = valor_P;
    n_data++;
  } 
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
  lcd.setCursor(0,0);
  lcd.print("Frecuencia:");
  lcd.setCursor(0,1);
  lcd.print("Direction");
  lcd.setCursor(12,0); 
  lcd.print(freq); // se imprime la frecuencia máxima
  // Se lee el estado del interruptor que marca la dirección del DRIVER y se imprime por pantalla:
  if (estadoInterruptor3 == HIGH){
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
else{   // se entra en el modo run:
 // se hace la comprobación cuando se usa la intrucción millis():
 if (contador<n_puntos){
    if (millis()-t0 >= 2000){ //  se hace para conseguir un cambio gradual de frecuencias cada 2 segundos con la instrucción millis()
      new_freq = freq_init*contador; // aquí se hace que coincida el valor mínimo de la frecuencia con el paso de frecuencias en la rampa
      presion = analogRead(pinPresion); // aquí se hace la lectura de la presión, de manera reescalada a la salida en tensión del sensor
      // Se insertan los valores de presión y de velocidad en las matrices:
      push_back(new_freq, presion);
      AD.setFrequency(0, new_freq); // se envía la frecuencia en cada punto al chip
      if(new_freq <= freq_max){ // se limita el valor de la frecuencia por debajo de la frecuencia máxima entregada por el potenciometro
        t0 = millis();
        if (contador < n_puntos){
        contador = contador+1; // se añade el valor de 1 al contador para seguir aumentando la frecuencia de 32 Hz hasta llegar a 3200
        }
     }
  }
 } else{
    K = K_f();
    tau_0 = tau_0_f(K);
    G_prime = G_prime_f(K,tau_0);
    G_2prime = G_2prime_f(K,tau_0);
    lcd.setCursor(0,0);
    lcd.print("K:");
    lcd.setCursor(0,4);
    lcd.print(K);
    lcd.setCursor(0,7);
    lcd.print("T:");
    lcd.setCursor(0,10);
    lcd.print(tau_0);
    lcd.setCursor(1,0);
    lcd.print("G'");
    lcd.setCursor(1,4);
    lcd.print(G_prime);
    lcd.setCursor(1,7);
    lcd.print("G'':");
    lcd.setCursor(1,11);
    lcd.print(G_2prime); 
 }
}
} 



//  -- END OF FILE --
