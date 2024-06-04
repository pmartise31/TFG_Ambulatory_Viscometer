#include <SparkFun_SHTC3.h>

// Primer paso es añadir las librerías que se van a usar:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
#include <Wire.h> 
#include <SparkFun_SHTC3.h> // librería para sensor de temperatura SHTC3
#include <DHT22.h> // librería para el sensor de temperatura DHT22
#include <LiquidCrystal_I2C.h>
// Definir el uso de pines: 
const int pinInterruptor = 8;
const int pinLED = 7;
const int pinDHT = 11;
const int pinPotenciometro= A1; 

// Inicializar el sensor Adafruit_SHTC3
SHTC3 mySHTC3; // se crea una nueva variable dentro de la clase Adafruit_SHTC3 que se llamará shtc3
DHT22 dht22(pinDHT); // se crea una nueva variable dentro de la clase DHT que se llamará dht
LiquidCrystal_I2C lcd(0x27,16,2);

// Segundo paso es definir las variables que se quieren obtener o fijar en el programa:
bool estadoInterruptor;
int setpoint_init;
float setpot,temp1,temp2;
unsigned long t0, t1;

void errorDecoder(SHTC3_Status_TypeDef message) {                           
  switch(message)
  {
    case SHTC3_Status_Nominal : Serial.print("Nominal"); break;
    case SHTC3_Status_Error : Serial.print("Error"); break;
    case SHTC3_Status_CRC_Fail : Serial.print("CRC Fail"); break;
    default : Serial.print("Unknown return code"); break;
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);// inicio de la transmisión de datos a 9600 baudios
  // configurar los pines en función de si son entradas o salidas: 
  pinMode(pinInterruptor, INPUT); // se configura el botón como entrada 
  pinMode(pinLED, OUTPUT); // se configura el LED como salida
  Wire.begin();
  errorDecoder(mySHTC3.begin());
  t0,t1 = millis();
  lcd.init(); // inicializa la pantalla LCD de 2 filas y 16 columnas
  lcd.backlight(); // activa la retroalimentación de la pantalla
  lcd.setCursor(0,0);
  lcd.print("Temp1:");
  lcd.setCursor(0,1);
  lcd.print("Setpoint:");
}

void loop() {
  // put your main code here, to run repeatedly:
  SHTC3_Status_TypeDef result = mySHTC3.update();             
  if(mySHTC3.lastStatus == SHTC3_Status_Nominal){                      
    temp1= mySHTC3.toDegC();
} else {
    Serial.print("Update failed, error: "); 
    errorDecoder(mySHTC3.lastStatus);
    Serial.println();
  } 

  temp2 = dht22.getTemperature(); // se lee la temperatura del sensor DHT22
  lcd.setCursor(7,0);
  lcd.print(temp1);

  if (millis()-t0 > 5000){
    t0= millis();
    Serial.print("Temperatura SHTC3 = "); 
    Serial.print(temp1);                           
    Serial.println("deg C");
    Serial.print("Temperature DHT22:");
    Serial.print(temp2);
    Serial.println("C");

    
  }

  estadoInterruptor = digitalRead(pinInterruptor); // se lee el estado del botón

  if (estadoInterruptor == HIGH) {
    setpoint_init = map(analogRead(pinPotenciometro), 0,1023,0,500);
    setpot = setpoint_init/10;
    lcd.setCursor(10,1);
    lcd.print(setpot);
    if (millis()-t1 > 1000){
      Serial.print("Setpoint cambiado a:"); // se muestra en el serial monitor el estado del LED
      Serial.print(setpot);
      Serial.println("C");
      t1=millis();
    }
  }
  if (temp1 > setpot || temp2 > setpot){
    digitalWrite(pinLED, LOW);
  }else {
    digitalWrite(pinLED, HIGH);
  }
} // se genera una condición que indica que si el botón está encendido el LED también se enciende



