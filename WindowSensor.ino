#include <LowPower.h>

//Window Motor Auto System

const int winUp= 2; //window up pin
const int winDownAuto = 3; //window auto down signal pin
const int winDown = 4; //window down move

const int currentInPin = 0; //Analog pin 0
const int sensitivity = 100;
const int offsetV = 2500;
const double MAX_CURRENT = 15;
const unsigned long TIMEOUT_OFFSET = 6000;
const double LOW_THRESHOLD = .95;
const double HIGH_THRESHOLD = 1.05;

volatile boolean movingUp = false;
volatile boolean movingDown = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(currentInPin, INPUT);
  pinMode(winUp, INPUT); //this will change on the fly
  pinMode(winDownAuto, INPUT);
  pinMode(winDown, OUTPUT);

  //active when pulled to ground
  attachInterrupt(winUp, autoUp, FALLING);
  attachInterrupt(winDownAuto, autoDown, FALLING);
  
}

void loop() {
  //set a time to watch how long to be running
  unsigned long startTime = millis();
  if(movingUp){
    pinMode(winUp, OUTPUT);
    digitalWrite(winUp, LOW);
    double lowBound = getCurrent() * LOW_THRESHOLD;
    double highBound = getCurrent() * HIGH_THRESHOLD;
    while(millis() < startTime + TIMEOUT_OFFSET && getCurrent() < MAX_CURRENT && getCurrent() > lowBound && getCurrent() < highBound){
      //wait here til its reached max
      if(movingDown){
        movingUp = false;
        movingDown = false;
        break;
      }
    }
    digitalWrite(winUp, HIGH);
    pinMode(winUp, INPUT);
    movingUp = false;
  }else if(movingDown){
    pinMode(winDown, OUTPUT);
    digitalWrite(winDown, LOW);
    double lowBound = getCurrent() * LOW_THRESHOLD;
    double highBound = getCurrent() * HIGH_THRESHOLD;
    while(millis() < startTime + TIMEOUT_OFFSET && getCurrent() < MAX_CURRENT && getCurrent() > lowBound && getCurrent() < highBound){
      //wait here til its reached max or see if cancelled
      if(movingUp){
        //user said stop
        movingUp = false;
        movingDown = false;
        break;
      }
    }
    digitalWrite(winDown, HIGH);
    pinMode(winDown, INPUT);
    movingDown = false;
  }

  //sleep til next interrupt
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void autoUp(){
  movingUp = true;
}

void autoDown(){
  movingDown = true;
}

double getCurrent(){
  int adc = analogRead(currentInPin);
  double volt = (adc / 1024.0) * 5000; //mV reading
  return ((volt - offsetV) / sensitivity); //return current
}
