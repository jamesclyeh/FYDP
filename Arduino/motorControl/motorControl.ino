#include "Arduino.h"
#include <digitalWriteFast.h>  // library for high performance reads and writes by jrraines
                               // see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1267553811/0
                               // and http://code.google.com/p/digitalwritefast/
#include <dcMotorControl.h>

// It turns out that the regular digitalRead() calls are too slow and bring the arduino down when
// I use them in the interrupt routines while the motor runs at full speed creating more than
// 40000 encoder ticks per second per motor.
 
// goalie rod rotational encoder (EN1, 1A, 2A)
#define grod_rot_intA 0
#define grod_rot_intB 1
#define grod_rot_enc_pinA 2
#define grod_rot_enc_pinB 3
#define grod_rot_mot_EN 53
#define grod_rot_mot_pinA 52
#define grod_rot_mot_pinB 51
volatile bool grod_rot_enc_BSet;
volatile long grod_rot_enc_ticks = 0;

// forward rod rotational encoder (EN2, 3A, 4A)
#define frod_rot_intA 3
#define frod_rot_intB 2
#define frod_rot_enc_pinA 20
#define frod_rot_enc_pinB 21
#define frod_rot_mot_EN 50
#define frod_rot_mot_pinA 49
#define frod_rot_mot_pinB 48
volatile bool frod_rot_enc_BSet;
volatile long frod_rot_enc_ticks = 0;

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

void setup()
{
  Serial.begin(115200);
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
      Serial.println("kick");
      rotG.go(700,255,frod_rot_enc_ticks);
      rotF.go(700,255,frod_rot_enc_ticks);
      delay(60);
      rotG.stop();
      rotF.stop();
      rotG.go(-700,255,frod_rot_enc_ticks);
      rotF.go(-700,255,frod_rot_enc_ticks);
      delay(5);
      rotG.stop();
      rotF.stop();
    } else if(input == "a") {
      Serial.println("left");
      transG.go(-700,255,frod_rot_enc_ticks);
      transF.go(-700,255,frod_rot_enc_ticks);
      delay(20);
      transG.stop();
      transF.stop();
    } else if(input == "d") {
      Serial.println("right");
      transG.go(700,255,frod_rot_enc_ticks);
      transF.go(700,255,frod_rot_enc_ticks);
      delay(20);
      transG.stop();
      transF.stop();
    } else if(input == "j") {
      Serial.println("left");
      transF.go(-700,255,frod_rot_enc_ticks);
      delay(20);
      transF.stop();
    } else if(input == "l") {
      Serial.println("right");
      transF.go(700,255,frod_rot_enc_ticks);
      delay(20);
      transF.stop();
    }
    
//  int target = 30;
//  dc2.go(target,255, frod_rot_enc_ticks);
//  while(true) {
  
//  Serial.print(dc2.currentPos);
//  delay(300);
//  target = -30;
//  dc2.go(target,255,frod_rot_enc_ticks);
//  while(dc2.distRemaining > 0) {
//    Serial.print(grod_rot_enc_ticks);
//    Serial.print("\t");
//    Serial.print(dc1.distRemaining);
//    Serial.print("\n");
//  }
//  Serial.print(dc2.currentPos);
//  delay(300);

}
 
// Interrupt service routines for the grod, rot quadrature encoder
void HandleGRodInterrupt()
{
  // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
  grod_rot_enc_BSet = digitalReadFast(grod_rot_enc_pinB);   // read the input pin
 
  // and adjust counter + if A leads B
  grod_rot_enc_ticks += grod_rot_enc_BSet ? +1 : -1;
//  rotG.currentPos += grod_rot_enc_BSet ? +1 : -1;
  rotG.distRemaining -= 1;
    
  if(rotG.distRemaining <= 0) {
    rotG.stop();
  }
}

// Interrupt service routines for the grod, rot quadrature encoder
void HandleFRodInterrupt()
{
  // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
  frod_rot_enc_BSet = digitalReadFast(frod_rot_enc_pinB);   // read the input pin
 
  // and adjust counter + if A leads B
  frod_rot_enc_ticks += frod_rot_enc_BSet ? +1 : -1;
//  rotF.currentPos += frod_rot_enc_BSet ? +1 : -1;
  rotF.distRemaining -= 1;
    
  if(rotF.distRemaining <= 0) {
    rotF.stop();
  }
}
