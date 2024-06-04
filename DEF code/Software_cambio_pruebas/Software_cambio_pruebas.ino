//
//    FILE: Software Ambulatory Viscometer
//  AUTHOR: Pablo Martínez Serrano
//    DATE: 04/06/2024
// PURPOSE: Design the software for the ambulatory viscometer project

// Libraries implemented in the code: 
#include <MD_AD9833.h>
#include <SPI.h>   
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Analogue and digital Pins used in Arduino Nano Board for the connections:
const uint8_t PIN_DATA = 11;	
const uint8_t PIN_CLK = 13;		
const uint8_t PIN_FSYNC = 10;
const int pinPotenciometro = A1; 
const int pinInterruptor1 = 8;
const int pinInterruptor2 = 6;
const int pinInterruptor3 = 7;
const int pinPresion = A0; 

// Access to the chip's library as an object:
MD_AD9833 AD(PIN_FSYNC); 
// Access to the LCD's library as an object:
LiquidCrystal_I2C lcd(0x27,16,2); 

// Definition of all the variables implmented in the code: 
bool estadoInterruptor1, estadoInterruptor2, estadoInterruptor3, printed = false, condicion = false, printed2 = false, printed3 = false, printed4 = false, printed5 = false;
char vel[4],pres[4];
int i=0, up_lim, down_lim,contador, n_data=0, n_puntosn = 32;
unsigned long t0;
float new_freq, K, tau_0, G_prime, G_2prime, frecuencia, presion, presion_mean, cum_pres, presion_cor,viscosity_sum;
float Gprime_sum,G_2prime_sum,mu,shear_stress,pres_sum,step, shear_val, shear_comp, square_shear_stress, term1; 
float vel_max, presion_mbar, vel_it, presion_Pa, vel_ms, freq_init,n_puntosf= 32.0, radius = 3.0, length = 25.0; 
float shear_rate_data[32], G2primas[32],  shear_stress_data[32], viscosity_vals[32], Gprimas[32];


//Function defined to round the values to the desired decimal value in the measurements:
float roundToDecimals(float num, int decimals) {
  float factor = pow(10, decimals);
  return round(num * factor) / factor;
}

// Functions defined to perform the Casson model: 

// Function to compute the yield point in the model: 
float compute_tau_0() {
  float pres_sum;
  float tau_0;
  for (int i=0; i<n_puntosn; i++){
    shear_val = shear_stress_data[i];
  if (shear_val>0){
    shear_comp = shear_val;
    pres_sum += shear_comp;
    }
  else{
    pres_sum += 0;
  }
  if (!printed3){
    if (pres_sum>50){
      tau_0 = shear_comp;
      printed3 = true;
    }
  }
}
return tau_0;
}

// Funtion to compute the apparent viscosity:
float compute_viscosity(float tau_0){
  float viscosity_sum = 0;
  for (int i=0; i<n_puntosn; i++){
    square_shear_stress = sqrt(shear_stress_data[i]);
    term1 = square_shear_stress-sqrt(tau_0);
    viscosity_vals[i] = (pow(term1,2))/shear_rate_data[i];
    viscosity_sum += viscosity_vals[i];
  }
  float viscosity = viscosity_sum/n_puntosn;
  return viscosity;
}

// Function to compute the storage modulus G':
float compute_Gprime(float tau_0){
  float Gprime_sum = 0;
  for (int i=0; i<n_puntosn; i++){
    Gprime_sum += tau_0/shear_rate_data[i];
  }
  float G_prime = Gprime_sum/n_puntosn;
  return G_prime;
}

// Function to compute the loss modulus G'':
float compute_G2prime(){
  float G_2prime_sum = 0;
  for (int i=0; i<n_puntosn; i++){
    G_2prime_sum += shear_stress_data[i]/shear_rate_data[i];
  }
  float G_2prime = G_2prime_sum/n_puntosn;
  return G_2prime;
}

// Setup() Function: 
void setup()
{
Serial.begin(115200); // inttialize the Serial monitor 

// The switches states are defined as INPUTS:
pinMode(pinInterruptor1, INPUT); // Interrupter controlling Setup/Run modes
pinMode(pinInterruptor2, INPUT); // Push button to move the actuator and adjust it to its initial position 
pinMode(pinInterruptor3, INPUT); // Interrupter to set the direction of the actuator
// Initializes the LCD display module:
lcd.init(); 
lcd.backlight();

// Initializes the AD 9833 frequency generator chip:
AD.begin();
AD.setMode(MD_AD9833::MODE_SQUARE1);
}

// Loop() Function:
void loop()
{

estadoInterruptor1 = digitalRead(pinInterruptor1); // Read the Setup/Run mode interrupter state:

// Entering in the Setup mode
if (estadoInterruptor1 == HIGH){ 

// Up and down limits definition:  
  up_lim = 3200;
  down_lim = 32;   

// Reading the frequencies set from the potentiometer:                 
  float frecuencia = map(analogRead(pinPotenciometro),0,1023,down_lim,up_lim); // values are mapped towards the imposed limits
  vel_max = frecuencia*0.3; // transformation from the frequency value to the speed of the actuator
  dtostrf(vel_max, 4, 0, vel); // transformation of the numerical value to characters for the display
  step = (frecuencia-down_lim)/n_puntosn; // step calculated to apply the velocity ramp
  freq_init = down_lim; // initial frequency computed as the down limit
  presion = map(analogRead(pinPresion), 95, 921, 0, 2000); // pressure calibrated and transformed to mbar
  dtostrf(presion, 4, 0, pres); // transformation of the numerical value to characters for the display

  // Displaying the velocity and the pressure:
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

// Reinizializing the conditionals to start again the measurements:
  contador = 0;
  printed=false;
  printed2 = false;
  printed3 = false;
  printed4 = false;
  printed5 = false;
  condicion=false;
  cum_pres=0;
  pres_sum = 0;

  estadoInterruptor3 = digitalRead(pinInterruptor3); // Interrupter controlling the direction of the actuator

  // Displaying the direction of the actuator
  if (estadoInterruptor3 == HIGH){
    lcd.setCursor(0,1);
    lcd.print(">>>");
  }else{
    lcd.setCursor(0,1);
    lcd.print("<<<");   
  }

  estadoInterruptor2 = digitalRead(pinInterruptor2);// Push button to control the position of the actuator
  
  if (estadoInterruptor2 == HIGH){
    AD.setFrequency(MD_AD9833::CHAN_0, frecuencia); // moves the actuator at the set frequency
  }
  else{
    AD.setFrequency(MD_AD9833::CHAN_0, 0); // the actuator doesn't move
  }
}

// Entering in the Run Mode
else{   

// Cleaning the LCD display:
  if (!condicion){ 
    lcd.setCursor(0, 0);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");  
    }

// Calibrating the sensor:
    lcd.setCursor(0,0);
    lcd.print("P Calibration:");
    for (int n=0; n<16; n++){
      presion = map(analogRead(pinPresion), 0, 1023, 0, 2000); // Taking pressure sensor readings 
      cum_pres += presion; // accumulating the pressure values
      lcd.setCursor(n,1);
      lcd.print('#');
      delay(500);
    }
    presion_mean = cum_pres/16; // computing the mean offset pressure

// Cleaning the LCD display:
    lcd.setCursor(0, 0);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");  
    }
    // Displaying the offset pressure for 3 seconds:
    lcd.setCursor(0,0); 
    lcd.print("Pressure Set:");
    lcd.setCursor(0,1);
    lcd.print("P:");
    lcd.setCursor(2,1);
    lcd.print(presion_mean);
    delay(3000);
    condicion = true;
  }

// Entering if the counter is under the number of measurments:  
  if (contador<=n_puntosn){ 

    estadoInterruptor3 = digitalRead(pinInterruptor3); // Displaying the direction of the actuator
    if (estadoInterruptor3 == HIGH){
      lcd.setCursor(0,1);
      lcd.print(">>>   ");
    }else{
        lcd.setCursor(0,1);
        lcd.print("<<<   "); 
      }
   
// Performing the measurements every 2 seconds:     
    if (millis()-t0 >= 2000){ 
      t0 = millis();
      new_freq = freq_init+(step*contador); //Imposing the new frequencies by addding the step to the initial frequency in each iteration
      vel_it = new_freq*0.3; // tranforming the frequency to velocity values    
      presion = map(analogRead(pinPresion),0,1023,0,2000); // reading the pressure sensor 
      presion_cor = (presion-presion_mean); // correcting the read through the mean offset pressure 
      shear_stress = ((presion_cor*radius)/(2*length))*100; // computing the shear stress though the pressure and the geometrical characteristics of the channel
    
// Displaying the velocity and the pressure: 
      lcd.setCursor(0,0);
      lcd.print("RUN   ");
      lcd.setCursor(4,0);
      lcd.print("V:");
      lcd.setCursor(10,0);
      lcd.print("  um/s");
      lcd.setCursor(6,0);
      lcd.print(vel_it);
      lcd.setCursor(4,1);
      lcd.print("P:");
      lcd.setCursor(6,1);
      lcd.print(presion_cor);
      lcd.setCursor(12,1);
      lcd.print("mbar");

// Sending the given frquency to the driver:
      AD.setFrequency(0, new_freq); 

//Storing and printing the shear rate and shear stress values:       
      shear_rate_data[contador] = new_freq; // se añade el shear rate 
      shear_stress_data[contador] = shear_stress; // se añade el shear stress
      Serial.println(shear_rate_data[contador]);
      Serial.println(shear_stress_data[contador]);

      contador++; // summing one step to the counter at each iteration
      }
      }

// Performing the model and delivering the final results:
    else{ 
    AD.setFrequency(0, 0); // stopping the movement of the actuator

// Computing all the rheological properties: 
    tau_0 = compute_tau_0();
    mu = compute_viscosity(tau_0);
    G_prime = compute_Gprime(tau_0);
    G_2prime = compute_G2prime();

// Cleaning the LCD display:
    if (!printed){
    lcd.setCursor(0, 0);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");  
    } 

// Delivering the results in the LCD display in 2 phases:
    lcd.setCursor(0,0);
    lcd.print("Tau_0:");
    lcd.setCursor(6,0);
    lcd.print(roundToDecimals(tau_0,4));
    lcd.setCursor(12,0);
    lcd.print("Pa");
    lcd.setCursor(0,1);
    lcd.print("Mu_app:");
    lcd.setCursor(7,1);
    lcd.print(roundToDecimals(mu,4));
    lcd.setCursor(12,1);
    lcd.print("Pa s");
    delay(5000);
    lcd.setCursor(0, 0); // Establecer la posición del cursor en la primera columna, primera fila
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }
    lcd.setCursor(0, 1); // Establecer la posición del cursor en la primera columna, primera fila
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");  
    } 
    lcd.setCursor(0,0);
    lcd.print("G':");
    lcd.setCursor(3,0);
    lcd.print(roundToDecimals(G_prime,4));
    lcd.setCursor(12,0);
    lcd.print("Pa s");
    lcd.setCursor(0,1);
    lcd.print("G'':");
    lcd.setCursor(4,1);
    lcd.print(roundToDecimals(G_2prime,4));
    lcd.setCursor(12,1);
    lcd.print("Pa s");
    Serial.println(tau_0);
    Serial.println(mu); 
    Serial.println(G_prime);
    Serial.println(G_2prime); 

// Printing each value of viscosity, G' and G'' computed through the model: 
    if (!printed2){
      for (int i=0; i<n_puntosn; i++){
          square_shear_stress = sqrt(shear_stress_data[i]);
          term1 = square_shear_stress-sqrt(tau_0);
          viscosity_vals[i] = (pow(term1,2))/shear_rate_data[i];
          Serial.println( viscosity_vals[i]);
      }
      printed2 = true;
    }
      if (!printed4){
        for (int i=0; i<n_puntosn; i++){
          Gprimas[i] = tau_0/shear_rate_data[i];
          Serial.println(Gprimas[i]);
        }
        printed4 = true;
      }
      if (!printed5){
        for (int i=0; i<n_puntosn; i++){
            G2primas[i] = shear_stress_data[i]/shear_rate_data[i];
            Serial.println(G2primas[i]);
        }
        printed5 = true;
      }
    printed = true;
    }  
    }
    }
    }






//  -- END OF FILE --
