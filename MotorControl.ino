#include <Wire.h>
#define STEP_SIGNAL_PIN 12

#define FORWARD HIGH
#define BACKWARD LOW
#define TIME_TO_RUN_MOTOR HIGH
#define PUMP_SPEED 100 // rpm - start speed

int calculateStepSignalFrequency(int _rpm);
int delayTime;
int rpm = 50;
void runMotor (int _delayTime);

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode(STEP_SIGNAL_PIN, OUTPUT);
  delayTime = calculateStepSignalFrequency(rpm);  // calculate delay for step motor signal
}

void loop()
{
  while( digitalRead(STEP_SIGNAL_PIN) == TIME_TO_RUN_MOTOR)
  {
    runMotor(delayTime);
  }
}

int calculateStepSignalFrequency(int _rpm)
{
  float a = 1;
  return ( (a / _rpm) * 1000);
}

void runMotor (int _delayTime)
{
  digitalWrite(STEP_SIGNAL_PIN, LOW);
  delay(_delayTime);
  digitalWrite(STEP_SIGNAL_PIN, HIGH);
  delay(_delayTime);
}

void receiveEvent()
{
  while(Wire.available()) // loop through all but the last
  {
    rpm = Wire.read();    // receive rpm
    delayTime = calculateStepSignalFrequency(rpm);    // calculate delay for step motor signal
  }        
}
