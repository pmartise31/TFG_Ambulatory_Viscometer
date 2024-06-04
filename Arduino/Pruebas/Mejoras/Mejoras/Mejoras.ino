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
const int pinPresion = A0; 



MD_AD9833 AD(PIN_FSYNC); // definimos el objeto con el que accederemos a la librería del chip
LiquidCrystal_I2C lcd(0x27,16,2); // definimos el objeto para usar la pantalla LCD

// Definir las variables que se obtienen o fijan desde el propio programa
bool estadoInterruptor1, estadoInterruptor2, estadoInterruptor3, printed = false;
char vel[4],pres[4],vel_n[4], pres_n[4];
int i=0;
unsigned long t0;
float new_freq, K, tau_0, G_prime, G_2prime, frecuencia, presion; 
float vel_max, presion_mbar, vel_it, presion_Pa, vel_ms, freq_init,n_puntosf= 32.0;
int contador, n_data=0, n_puntosn = 32;
float v_data[32]; // tasa de deformación 
float P_data[32]; // valores de shear stress (presión)

// Vamos a definir las funciones de los parámetros que nos serán útiles para calcula G' y G'', así como dar la K (constante del modelo) y viscosidad aparente del esputo:

// Función para el yield point del modelo de Casson:
float compute_tau_0() {
  float pres_sum = 0;
  for (int i=0; i<n_puntosn; i++){
    pres_sum += P_data[i];
  }
  float tau_0 = pres_sum / n_puntosn;
  return tau_0;
}

// Función para calcular el índice de consistencia del modelo de Casson:
float compute_K(float tau_0){
  float vel_sum  = 0;
  for (int i=0; i<n_puntosn; i++){
    vel_sum += v_data[i];
  }
  float K = (tau_0*tau_0) / vel_sum;
  return K;
}
// Función para calcular la viscosidad aparente del modelo de Casson:
float compute_viscosity(float K, float tau_0){
  float viscosity = sqrt(K*tau_0);
  return viscosity;
}

// Función para calcular el módulo de almacenamiento G':
float compute_Gprime(float K, float tau_0){
  float G_prime = sqrt(K)*sqrt(tau_0);
  return G_prime;
}

// Función para calcular el módulo de pérdida G'':
float compute_G2prime(float K, float tau_0){
  float G_2prime = sqrt(K*0.5)*sqrt(tau_0);
  return G_2prime;
}


void setup()
{
  Serial.begin(115200);
  // definición de los interruptores: 
  pinMode(pinInterruptor1, INPUT); // Interruptor para el cambio de modo (SETUP o RUN)
  pinMode(pinInterruptor2, INPUT); // Pulsador para hacer funcionar el actuador y poder moverlo para reajustar su posición
  pinMode(pinInterruptor3, INPUT); // Interruptor para marcar la dirección que debe adoptar el actuador
  lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
  lcd.backlight(); // activa la retroalimentación de la pantalla
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
  float frecuencia = map(analogRead(pinPotenciometro),0,1023,32,3200); //ajusto la salida del potenciometro --> se pone el rango de frecuencias útiles
  vel_max = frecuencia*0.3; // este es el valor de la velocidad máxima en micrometros/s en función de la frecuencia pautada
  dtostrf(vel_max, 4, 0, vel);
  freq_init = frecuencia/n_puntosf; // la frecuencia inicial será aquella que equivalente a la frecuencia máxima
  //presion = map(analogRead(pinPresion), 0, 1023, 0, 2000); // ajuste de de presion en milibares directamente
  presion = 1000;
  dtostrf(presion, 4, 0, pres);
  lcd.home();
  lcd.setCursor(0,0);
  lcd.print("SETUP ");
  lcd.setCursor(6,0);
  lcd.print("V:");
  lcd.setCursor(8,0);
  lcd.print(vel);
  lcd.setCursor(12,0);
  lcd.print("um/s");
  lcd.setCursor(3,1);
  lcd.print("  ");
  lcd.setCursor(4,1);
  lcd.print("  P:");
  lcd.setCursor(8,1);
  lcd.print(pres);
  lcd.setCursor(12,1);
  lcd.print("mbar");
  lcd.home();
  // Se lee el estado del interruptor que marca la dirección del DRIVER y se imprime por pantalla:
  if (estadoInterruptor3 == HIGH){
    lcd.setCursor(0,1);
    lcd.print('>');
    lcd.setCursor(1,1);
    lcd.print('>');
    lcd.setCursor(2,1);
    lcd.print('>');
  }else{
    lcd.setCursor(0,1);
    lcd.print('<');
    lcd.setCursor(1,1);
    lcd.print('<');
    lcd.setCursor(2,1);
    lcd.print('<');
  }
  // se activa el DRIVER para volver a la posición inicial el émbolo o purgar:
  if (estadoInterruptor2 == HIGH){
    AD.setFrequency(MD_AD9833::CHAN_0, frecuencia);
  }
  else{
    AD.setFrequency(MD_AD9833::CHAN_0, 0);
  }
}
// condición modo RUN: 
else{   // se entra en el modo run:
    if (contador<=n_puntosn){ 
  // se hace la comprobación cuando se usa la intrucción millis():
    if (millis()-t0 >= 2000){ //  se hace para conseguir un cambio gradual de frecuencias cada 2 segundos con la instrucción millis()
      t0 = millis();

      new_freq = freq_init*contador; // aquí se hace que coincida el valor mínimo de la frecuencia con el paso de frecuencias en la rampa
      vel_it = new_freq*0.3; // este es el valor de la velocidad máxima en micrometros/s en función de la frecuencia pautada    
      dtostrf(vel_it, 4, 0, vel_n);   
      //presion = map(analogRead(pinPresion),0,1023,0,2000); // aquí se hace la lectura de la presión, de manera reescalada a la salida en tensión del sensor
      presion = 1000;
      dtostrf(presion, 4, 0, pres_n);
      // Se añaden a los arrays los valores de velocidad y de presion
      lcd.setCursor(0,0);
      lcd.print("RUN  ");
      lcd.setCursor(6,0);
      lcd.print("V:");
      lcd.setCursor(8,0);
      lcd.print(vel_it);
      lcd.setCursor(12,0);
      lcd.print("um/s");
      lcd.setCursor(6,1);
      lcd.print("P:");
      lcd.setCursor(8,1);
      lcd.print(presion);
      lcd.setCursor(12,1);
      lcd.print("mbar");
      // Displaying info in the Serial monitor:
      Serial.print("Cambio en el contador: ");
      Serial.println(contador);
      Serial.print("F:");
      Serial.println(new_freq);
      Serial.print("vel_it:");
      Serial.println(vel_it);
      AD.setFrequency(0, new_freq); // se envía la frecuencia en cada punto al chip
      if (estadoInterruptor3 == HIGH){
        lcd.setCursor(0,1);
        lcd.print('>');
        lcd.setCursor(1,1);
        lcd.print('>');
        lcd.setCursor(2,1);
        lcd.print('>');
      }else{
        lcd.setCursor(0,1);
        lcd.print('<');
        lcd.setCursor(1,1);
        lcd.print('<');
        lcd.setCursor(2,1);
        lcd.print('<');
      }
      v_data[contador] = new_freq; // se añade el gradiente de velocidad en s-1
      P_data[contador] = presion; // se añade la presión
      Serial.println(v_data[contador]);
      Serial.println(P_data[contador]);
      contador++; // se añade el valor de 1 al contador para seguir aumentando la frecuencia de 32 Hz hasta llegar a 3200
      }
      }
       else{ 
    tau_0 = compute_tau_0();
    K = compute_K(tau_0);
    float mu = compute_viscosity(K, tau_0);
    G_prime = compute_Gprime(K, tau_0);
    G_2prime = compute_G2prime(K, tau_0);
    // Se limpia la pantalla e imprimen los valores:
    if (!printed){
    lcd.setCursor(0, 0); // Establecer la posición del cursor en la primera columna, primera fila
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }
    lcd.setCursor(0, 1); // Establecer la posición del cursor en la primera columna, primera fila
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");  
    } 
    lcd.setCursor(0,0);
    lcd.print("K:");
    lcd.setCursor(2,0);
    lcd.print(round(K));
    lcd.setCursor(7,0);
    lcd.print("T:");
    lcd.setCursor(9,0);
    lcd.print(round(mu));
    lcd.setCursor(0,1);
    lcd.print("G':");
    lcd.setCursor(3,1);
    lcd.print(round(G_prime));
    lcd.setCursor(7,1);
    lcd.print("G'':");
    lcd.setCursor(11,1);
    lcd.print(round(G_2prime));
    Serial.println(K);
    Serial.println(mu); 
    Serial.println(G_prime);
    Serial.println(G_2prime); 
    printed = true;
  }
  }
  }
  }






//  -- END OF FILE --