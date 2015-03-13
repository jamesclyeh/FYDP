#include "Arduino.h"
#include "dcMotorControl.h"

#include <digitalWriteFast.h>

dcMotorControl::dcMotorControl(int ppinE, int ppinL, int ppinR) {
  pinE = ppinE;
  pinL = ppinL;
  pinR = ppinR;
  
  pinMode(pinE,OUTPUT);
  pinMode(pinL,OUTPUT);
  pinMode(pinR,OUTPUT);
  digitalWrite(pinE,LOW);

  goal = 0;
  currentPos = 0;
  distRemaining = 0;
  speed = 0;
  direction = false;
}

void dcMotorControl::go(int pGoal, int pSpeed, const int& encoderTicks) {
  goal = pGoal;
  distRemaining = abs(goal);
  speed = pSpeed;
  if(encoderTicks != currentPos) {
    distRemaining += abs(abs(encoderTicks)-abs(currentPos));
  }
  if (goal >= 0) {
    Serial.write("forward");
    digitalWriteFast(pinL,LOW);
    digitalWriteFast(pinR,HIGH);
  } else {
    Serial.write("reverse");
    digitalWriteFast(pinR,LOW);
    digitalWriteFast(pinL,HIGH);
  }
  digitalWriteFast(pinE,HIGH);
}

void dcMotorControl::stop() {
  digitalWriteFast(pinE,LOW);
  digitalWriteFast(pinL,LOW);
  digitalWriteFast(pinR,LOW);
  distRemaining = 0;
}

void dcMotorControl::reset() {
  this->stop();
  goal = 0;
  currentPos = 0;
  distRemaining = 0;
}