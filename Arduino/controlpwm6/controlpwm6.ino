



#include <LiquidCrystal_I2C.h>
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

#define fin 8
#define out 9                //define digital 8 as led.
#define set 10
#define zero 11
#define forback 12
#define analog_freq A4//frequency potentiometer adjustment is connected to the analog input 4
#define analog_mx_um A5



int freq;

unsigned long t0;

int tx;

int vel,vel_1=0,mx_um,mx_um_1=0,tfin;
int ctrol,dir_1=-1,dir_2=-1,adjzero_1=-1;
byte dir,adjzero;
char vel_string[4];
char mx_um_string[2];
void setup() {
lcd.init();
lcd.backlight(); 
Serial.begin(115000);
lcd.setCursor(0,0);
lcd.print("Gradient Machine");
lcd.setCursor(0,1);
lcd.print("Biophysics Unit");
delay(5000);
lcd.setCursor(0,0);
lcd.print("Speed:    u/s   ");
lcd.setCursor(0,1);
lcd.print("Length:  mm    ");
pinMode(out,OUTPUT);            //define led as output
pinMode(fin,OUTPUT);
pinMode(set,INPUT);
pinMode(zero,INPUT);
pinMode(forback,INPUT);


                    } //fin setup

void loop() {

ctrol =digitalRead(set);
adjzero=digitalRead(zero);
if (ctrol==1 & adjzero==0 ) {

long int mx_um=map (analogRead(analog_mx_um),0,1023,0,50000);
freq = analogRead(analog_freq);
int vel= map(freq,0,1023,10,4000);

Serial.println(tfin);
if ( (vel >= 1.003*vel_1) ||(vel <= 0.997*vel_1 )) {
lcd.setCursor(6,0);

dtostrf(vel,4,0, vel_string);                     // convierte P_sensor_abs en un char compuesto por dos enteros y 0 decimales
lcd.print(vel_string);
vel_1=vel;
  
}
if ( mx_um_1 != mx_um)      {
lcd.setCursor(7,1);

dtostrf(mx_um/1000,2,0, mx_um_string);                     // convierte P_sensor_abs en un char compuesto por dos enteros y 0 decimales
lcd.print(mx_um_string);


mx_um_1=mx_um;
  
}

tx=156000/vel;
tfin=mx_um/vel;
digitalWrite(fin , 1);
t0=millis();




}


if (ctrol==0){
digitalWrite(fin , 0);

      digitalWrite(out , 1);
      delayMicroseconds(tx);
     
      digitalWrite(out , 0);         
      delayMicroseconds(tx);

if (millis()-t0>=1000*tfin) {
  
  digitalWrite(fin , 1);
  lcd.setCursor(12,1);
lcd.print("Stop");
  
  }

}                                 

dir= digitalRead(forback);

if (dir_1 != dir){

dir_1=dir;

if (dir==1) {

lcd.setCursor(13,0);
lcd.print(" <<");
  }
}

if (dir_2 != dir){

dir_2=dir;

if (dir==0) {

lcd.setCursor(13,0);
lcd.print(" >>");
  }
}











if (adjzero==1){
digitalWrite(fin , 0);
if (adjzero_1 != adjzero){
lcd.setCursor(12,1);
lcd.print(" Run");
adjzero_1=adjzero;
  }



   
   digitalWrite(out , 1);
   delayMicroseconds(tx);
   digitalWrite(out , 0);         
   delayMicroseconds(tx);
 }
else{
  adjzero_1= 0;
  lcd.setCursor(12,1);
lcd.print("Stop");
  
  
  
  }



}    


   

   
   


        //a 1250 1mm/s, a 312 4mm/s, a 12500 0.1mm/s, (400pul/rev)

//  a 156  4mm/s  a 625 1mm/s a 6250 0.1mm/s  (800pul/rev)

// a 39 4mm/s, a 156 1mm/s   a 1562 0.1mm/2  a 15625 0.01mm/s  (3200pul/rev)
                      
// el avance es de 1mm/rev
  
  

       
            
         


              
