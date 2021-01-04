#include <LowPower.h>

//Window Motor Auto System

const int winUp = 3; //window up pin
const int winDownAuto = 2; //window auto down signal pin
const int winDown = 4; //window down move

const int currentInPin = A0; //Analog pin 0
const int sensitivity = 100;
const int offsetV = 2500;
const double MAX_CURRENT = 9;
const unsigned long TIMEOUT_OFFSET = 6000;
const int AUTO_UP_TIMER = 500;
const int SLEEP_TIME = 5000;
unsigned long stopTime = 0;

volatile boolean awake = false;

void setup() {
	// put your setup code here, to run once:
	pinMode(currentInPin, INPUT);
	pinMode(winUp, INPUT_PULLUP); //this will change on the fly
	pinMode(winDownAuto, INPUT_PULLUP);
	pinMode(winDown, OUTPUT);
	digitalWrite(winDown, HIGH);
	Serial.begin(9600);

	//active when pulled to ground
	attachInterrupt(digitalPinToInterrupt(winUp), wakeUp, FALLING);
	attachInterrupt(digitalPinToInterrupt(winDownAuto), wakeUp, FALLING);
}

void loop() {
	while (awake) {
		Serial.println("Woke");
		//start timer and read pins
		unsigned long startTime = millis();

		float lowBound = .0f;
		float highBound = .0f;

		if (!digitalRead(winUp)) {
			//long press
			while (!digitalRead(winUp)) {
				unsigned long currentTime = millis();
				Serial.println(currentTime - startTime);
				//wait a few millis before engaging auto up
				if ((currentTime - startTime) > AUTO_UP_TIMER) {
					pinMode(winUp, OUTPUT);
					pinMode(winDown, INPUT_PULLUP);
					delay(200);
					digitalWrite(winUp, LOW);

					//make sure dont trip watchdog timer/current
					while (millis() < (currentTime + TIMEOUT_OFFSET) && (getCurrent() < MAX_CURRENT)) {
						Serial.println("Up");
						if (!digitalRead(winDown)) {
							break;
						}
					}
					//go back to input and stop commanding motor
					Serial.println("StopUp");
					digitalWrite(winUp, HIGH);
					pinMode(winUp, INPUT_PULLUP);
					pinMode(winDown, OUTPUT);
					stopTime = millis();
				}
			}
		}
		else if (!digitalRead(winDownAuto)) { //if autodown button pressed
			unsigned long currentTime = millis();
			//start moving down
			digitalWrite(winDown, LOW);
			//watchdogs
			while ((millis() < currentTime + TIMEOUT_OFFSET) && (getCurrent() < MAX_CURRENT)) {
				Serial.println("Down");
				if (!digitalRead(winUp)) {
					break;
				}
			}
			//stop moving
			Serial.println("StopDown");
			digitalWrite(winDown, HIGH);
			pinMode(winUp, INPUT_PULLUP);
			stopTime = millis();
		}
		else {
			//sleep till next interrupt after timer
			unsigned long currentTime = millis();
			if (stopTime + SLEEP_TIME < currentTime) {
				awake = false;
				stopTime = currentTime;
				Serial.println("Sleeping");
				Serial.flush();
				LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
			}
		}
	}
}

void wakeUp() {
	awake = true;
}

double getCurrent() {
	int adc = analogRead(currentInPin);
	float volt = (adc / 1024.0f) * 5000.0f; //mV reading
	float current = ((volt - offsetV) / sensitivity); //return current
	return (current < 0 ? -current : current); //make current positive
}