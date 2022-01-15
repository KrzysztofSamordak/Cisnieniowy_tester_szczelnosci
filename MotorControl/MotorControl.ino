#include <Wire.h>

#define pumpControlFromUnoSignalPin 13
#define STEP_SIGNAL_PIN 12

#define TIME_TO_RUN_MOTOR LOW
#define PUMP_SPEED 10

int delayTime = PUMP_SPEED;;
void runMotor (int _delayTime);

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode(pumpControlFromUnoSignalPin, INPUT_PULLUP);
  pinMode(STEP_SIGNAL_PIN, OUTPUT);
  digitalWrite(STEP_SIGNAL_PIN, LOW);
}

void loop()
{
  while( digitalRead(pumpControlFromUnoSignalPin) == TIME_TO_RUN_MOTOR)
  {
    runMotor(delayTime);
  }
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
  while(Wire.available())
  {
    delayTime = Wire.read();    // receive delay for step motor signal
  }        
}
