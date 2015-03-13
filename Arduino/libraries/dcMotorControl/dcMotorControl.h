#ifndef dcMotorControl_h
#define dcMotorControl_h

class dcMotorControl {
  public:
    int pinE;
    int pinL;
    int pinR;
    

    int goal;
    int currentPos;
    volatile int distRemaining;
    int speed;
    int direction;
    dcMotorControl(int ppinE, int ppinL, int ppinR);
    void go(int pGoal, int pSpeed, const int& encoderTicks);
    void stop();
    void reset();
};

#endif
