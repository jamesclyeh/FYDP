#include "Arduino.h"
#include <digitalWriteFast.h>  // library for high performance reads and writes by jrraines
                               // see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1267553811/0
                               // and http://code.google.com/p/digitalwritefast/
#include <dcMotorControl.h>

#include <Servo.h>

// It turns out that the regular digitalRead() calls are too slow and bring the arduino down when
// I use them in the interrupt routines while the motor runs at full speed creating more than
// 40000 encoder ticks per second per motor.
 
// forward rod rotational encoder (EN1, 1A, 2A)
#define frod_rot_intA 3
#define frod_rot_intB 2
#define frod_rot_enc_pinA 20
#define frod_rot_enc_pinB 21
#define frod_rot_mot_EN 53
#define frod_rot_mot_pinA 52
#define frod_rot_mot_pinB 51
volatile bool frod_rot_enc_BSet;
volatile long frod_rot_enc_ticks = 0;
volatile long frod_wait_kick = 0;
bool frod_kicking = false;

// goalie rod rotational encoder (EN2, 3A, 4A)
#define grod_rot_intA 0
#define grod_rot_intB 1
#define grod_rot_enc_pinA 2
#define grod_rot_enc_pinB 3
#define grod_rot_mot_EN 50
#define grod_rot_mot_pinA 49
#define grod_rot_mot_pinB 48
volatile bool grod_rot_enc_BSet;
volatile long grod_rot_enc_ticks = 0;
volatile long grod_wait_kick = 0;
bool grod_kicking = false;

// goalie rod translational encoder (EN1, 1A, 2A)
#define grod_trans_mot_EN 25 
#define grod_trans_mot_pinA 26
#define grod_trans_mot_pinB 27

// forward rod translational encoder (EN2, 3A, 4A)
#define frod_trans_mot_EN 22
#define frod_trans_mot_pinA 23
#define frod_trans_mot_pinB 24

dcMotorControl rotG = dcMotorControl(grod_rot_mot_EN, grod_rot_mot_pinA, grod_rot_mot_pinB);
dcMotorControl rotF = dcMotorControl(frod_rot_mot_EN, frod_rot_mot_pinA, frod_rot_mot_pinB);
dcMotorControl transG = dcMotorControl(grod_trans_mot_EN, grod_trans_mot_pinA, grod_trans_mot_pinB);
dcMotorControl transF = dcMotorControl(frod_trans_mot_EN, frod_trans_mot_pinA, frod_trans_mot_pinB);

Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
 
int pos = 0;    // variable to store the servo position 

void setup()
{
  Serial.begin(115200);
  
  myservo.attach(9);
  
  pinMode(grod_rot_enc_pinA, INPUT); // sets pin A as input
  digitalWrite(grod_rot_enc_pinA, LOW); // turn on pullup resistors
  pinMode(grod_rot_enc_pinB, INPUT); // sets pin B as input
  digitalWrite(grod_rot_enc_pinB, LOW); // turn on pullup resistors
  pinMode(frod_rot_enc_pinA, INPUT); // sets pin A as input
  digitalWrite(frod_rot_enc_pinA, LOW); // turn on pullup resistors
  pinMode(frod_rot_enc_pinB, INPUT); // sets pin B as input
  digitalWrite(frod_rot_enc_pinB, LOW); // turn on pullup resistors
  attachInterrupt(grod_rot_intA, HandleGRodInterrupt, RISING);
  attachInterrupt(frod_rot_intA, HandleFRodInterrupt, RISING);
}
 
void loop()
{
    String input = "";
    while(Serial.available() > 0) {
      input += (char)Serial.read();
      delay(5);
    }
    
    if(input == "w") {
      rotG.go(700,255,frod_rot_enc_ticks);
////      rotF.go(700,255,frod_rot_enc_ticks);
      delay(40);
      rotG.stop();
//      rotF.stop();
//      rotG.go(-700,255,frod_rot_enc_ticks);
//      rotF.go(-700,255,frod_rot_enc_ticks);
//      rotF.stop();
      grod_rot_enc_ticks += 25;
    } else if(input == "a") {
      transG.go(-700,255,frod_rot_enc_ticks);
      delay(80);
      transG.stop();
//      for(pos = 0; pos < 15; pos += 1)  // goes from 0 degrees to 180 degrees 
//      {                                  // in steps of 1 degree 
//        myservo.write(pos);              // tell servo to go to position in variable 'pos' 
//        delay(5);                       // waits 15ms for the servo to reach the position 
//      } 
    } else if(input == "d") {
      transG.go(700,255,frod_rot_enc_ticks);
      delay(80);
      transG.stop();
//      for(pos = 15; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
//      {                                
//        myservo.write(pos);              // tell servo to go to position in variable 'pos' 
//        delay(5);                       // waits 15ms for the servo to reach the position 
//      }
    } else if(input == "j") {
      transF.go(-700,255,frod_rot_enc_ticks);
      delay(40);
      transF.stop();
    } else if(input == "i") {
      rotF.go(700,255,frod_rot_enc_ticks);
      delay(40);
      rotF.stop();
      frod_rot_enc_ticks += 25;
    } else if(input == "l") {
      transF.go(700,255,frod_rot_enc_ticks);
      delay(40);
      transF.stop();
    } else if (input != "") {
      Serial.println(input);
    }
}

//void serialEvent() {
//  while (Serial.available()) {
//    // get the new byte:
//    char inChar = (char)Serial.read(); 
//    // add it to the inputString:
//    inputString += inChar;
//    // if the incoming character is a newline, set a flag
//    // so the main loop can do something about it:
//    if (inChar == '\n') {
//      stringComplete = true;
//    } 
//  }
//}

void centerGRod() {
    grod_kicking = true;
    rotG.go(700,255,grod_rot_enc_ticks);
    delay(1700);
    rotG.stop();
    grod_kicking = false;
}

void centerFRod() {
  frod_kicking = true;
  rotF.go(700,255,frod_rot_enc_ticks);
  delay(1700);
  rotF.stop();
  frod_kicking = false;
} 

// Interrupt service routines for the grod, rot quadrature encoder
void HandleGRodInterrupt()
{
  // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
  grod_rot_enc_BSet = digitalReadFast(grod_rot_enc_pinB);   // read the input pin
  
  grod_rot_enc_ticks += (grod_rot_enc_BSet ? -1 : +1) + 1024;
  grod_rot_enc_ticks = grod_rot_enc_ticks % 1024;
  
  if(!grod_kicking) {
    grod_wait_kick += (grod_rot_enc_BSet ? -1 : +1);
    grod_wait_kick = grod_wait_kick % 40;
    if(grod_wait_kick == -39) {
      grod_kicking = true;
      grod_wait_kick = 0;
      centerGRod();
    }
  } else {
     grod_wait_kick = 0;
  }
}

// Interrupt service routines for the grod, rot quadrature encoder
void HandleFRodInterrupt()
{
  // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
  frod_rot_enc_BSet = digitalReadFast(frod_rot_enc_pinB);   // read the input pin
 
  frod_rot_enc_ticks += (frod_rot_enc_BSet ? -1 : +1) + 1024;
  frod_rot_enc_ticks = frod_rot_enc_ticks % 1024;
  
  if(!frod_kicking) {
    frod_wait_kick += (frod_rot_enc_BSet ? -1 : +1);
    frod_wait_kick = frod_wait_kick % 40;
    if(frod_wait_kick == -39) {
      frod_kicking = true;
      frod_wait_kick = 0;
      centerFRod();
    }
  } else {
     frod_wait_kick = 0;
  }
}
