// Space for libraries and variables types:
const int pinLED = 7;

bool estadoLED, LED_encendido = false;
unsigned long t0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinLED, OUTPUT);
  t0 = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
if (millis()-t0 > 5000 && LED_encendido == false){
  digitalWrite(pinLED, HIGH);
  t0 = millis();
  LED_encendido = true;
} else if (millis()-t0 > 5000 && LED_encendido == true){
  digitalWrite(pinLED, LOW);
  t0 = millis();  
  LED_encendido = false;
}

}
