#include <PID_v1.h>
#include "encoderPolling.h"
#include "motor.h"
#include "ranger.h"

#define TRAVEL_FEETS 40
#define STEERINGTRIM -1.5

#define THROTTLE_START 100
#define THROTTLE_MAX 200
#define THROTTLE_STEP 5
#define THROTTLE_STEP_TIME 200

#define INCH_COUNT_CONVERSION 20.0/7.5

int phase = 0;
int steeringTrim;
double Kp = 2, Ki = 0, Kd = 0;
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

Encoder encoder;
Motor motor;
Ranger range;

void moveForward(float inches)
{
   encoder.resetCount();
   int throttle = THROTTLE_START;
   int moved = 0;
   float inchesToCount = (INCH_COUNT_CONVERSION); // 20 counts / correction
   int distance = (int)(inchesToCount * inches);
   unsigned long lastTime = micros();
   
   while (moved < distance)
  {
    
    encoder.poll();
    float lspd = encoder.getLeftSpeed();
    float rspd = encoder.getRightSpeed(); // read this side

    Input = lspd - rspd;
    if ( 
      (lspd == 0 && rspd > 0) ||
      (lspd > 0 && rspd == 0)
    )
    {
      Input = Input * 4;
    }
    myPID.Compute();

    if (throttle < 0)
    {
      motor.left(0);
      motor.right(0);
    }
    else
    {
      motor.left(throttle - Output);
      motor.right(throttle + Output);
    }

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastTime;

    moved = (encoder.getRightCount() + encoder.getLeftCount())/2;

    if (elapsedTime > THROTTLE_STEP_TIME)
    {
      throttle += THROTTLE_STEP;
      if (throttle > THROTTLE_MAX) throttle = THROTTLE_MAX;
      lastTime = currentTime;
    }
  }
  motor.stop();
}



void setup() {
  Serial.begin(115200);
  steeringTrim = STEERINGTRIM;
  Setpoint = steeringTrim;
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(10);
  myPID.SetOutputLimits(-255.0, 255.0);
  encoder.begin();
  motor.begin();
  range.begin();
  delay(1000);
}


void turn(float degree)
{
  bool clockwise = true;
  if (degree < 0)
  {
    clockwise = false;
    degree = abs(degree);
  }
   encoder.resetCount();
   int rotation = 140;
   int avg_moved = 0;
   float degreeToCount = (20.0/160.0); // 20 counts / correction
   int turnCount = (int)(degreeToCount * degree);
   Setpoint = 0;
   Serial.print("TurnCount: "); Serial.println(turnCount);
   while (avg_moved < turnCount)
  {

    encoder.poll();
    float lspd = encoder.getLeftSpeed();
    float rspd = encoder.getRightSpeed(); // read this side

    Input = lspd - rspd;
    if ( 
      (lspd == 0 && rspd > 0) ||
      (lspd > 0 && rspd == 0)
    )
    {
      Input = Input * 4;
    }
    
    myPID.Compute();

    if (rotation < THROTTLE_START)
    {
      motor.left(0);
      motor.right(0);
    }
    else if (clockwise)
    {
      motor.left(rotation - Output);
      motor.right(-(rotation + Output));
    }
    else
    {
      motor.left(-(rotation - Output));
      motor.right((rotation + Output));
    }

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastTime;

    avg_moved = (encoder.getRightCount() + encoder.getLeftCount())/2;

    if (elapsedTime > THROTTLE_STEP_TIME)
    {
      if (speedup) rotation += 5;
      if (rotation > THROTTLE_MAX) rotation = THROTTLE_MAX;
      lastTime = currentTime;
    }
  }
  motor.stop();
}

void scan()
{
  
}

void loop() {
  Serial.println(range.getDist());
}
