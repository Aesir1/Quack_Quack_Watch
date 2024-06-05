
#include <Servo.h>
#include "DCF77.h"
#include "TimeLib.h"

// DCF77 def
#define DCF_PIN 2        // Connection pin to DCF 77 device
#define DCF_INTERRUPT 0  // Interrupt number associated with pin
DCF77 DCF = DCF77(DCF_PIN, DCF_INTERRUPT, false);

// Servo def
int servoMotorPin = 11;
Servo myServo;

// Buzzer
int buzzPin = 12;

// display def
int latch = 4;
int clock = 5;
int data = 3;

unsigned char table[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00 };  //Display  digital data

const int nbrDigits = 4;
const int digitPins[nbrDigits] = { 6, 7, 8, 9 };

int hourAlarm = 17;
int minuteAlarm = 43;

int lastUpdatedDay = -1;
bool timeToSync = true;

int actualMinute = -1;
int actualHour = -1;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(buzzPin, LOW);
  Serial.begin(9600);
  DCF.Start();
  Serial.println("Warten auf DCF77-Zeit... ");
  Serial.println("Dies dauert mindestens 2 Minuten, in der Regel eher länger.");

  myServo.attach(servoMotorPin);

  //Configuring  output pins for the 74HC595
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);

  Serial.print("INIT SETUP");
  //Configuring output pins for commun cathod of each 7 segment digit
  for (int i = 0; i < nbrDigits; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }
}

void loop() {
  if (timeToSync) {
    delay(4950);
    time_t DCFtime = DCF.getTime();

    if (DCFtime != 0) {
      Serial.println("Time is updated");
      setTime(DCFtime);
      quackOfDuck();
      timeToSync = false;
    }
  }

  if (hourAlarm == hour() && minuteAlarm == minute() && second() < 10) {
    kukuIsAlive();
  }

  digitalClockDisplay();

  if (actualMinute != minute()) {
    actualMinute = minute();
    actualHour = hour();
    int currentDay = day();

    if (((currentDay - 1) % 7 == 0) && currentDay != lastUpdatedDay) {
      Serial.println("Time to sync");
      timeToSync = true;
      lastUpdatedDay = currentDay;
    }
  }

  displayNumber(actualHour, actualMinute);
}

//Function to display a number on 4 digit 7 segments
void displayNumber(int hours, int minutes) {
  if (hours == 0) {
    displayDigit(0, 0);
    displayDigit(0, 1);
  }

  if (minutes == 0) {
    displayDigit(0, 2);
    displayDigit(0, 3);
  }

  if (hours > 0) {
    displayDigit(hours % 10, 1);
    hours = hours / 10;
    displayDigit(hours % 10, 0);
  }

  if (minutes > 0) {
    displayDigit(minutes % 10, 3);
    minutes = minutes / 10;
    displayDigit(minutes % 10, 2);
  }
}

//Function  to display on one seven segments digit
void displayDigit(unsigned char num, int digit) {
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, table[num]);
  digitalWrite(latch, HIGH);

  digitalWrite(digitPins[digit], LOW);
  delay(5);
  digitalWrite(digitPins[digit], HIGH);
}



void digitalClockDisplay() {
  // digital clock display of the time
  printDigits(hour());
  Serial.print(":");
  printDigits(minute());
  Serial.print(":");
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void kukuIsAlive() {
  for (int i = 110; i > 0; i--) {
    displayNumber(hour(), minute());
    myServo.write(i);
    delay(50);
  }

  quackOfDuck();

  for (int i = 0; i < 110; i++) {
    displayNumber(hour(), minute());
    myServo.write(i);
    delay(50);
  }
}

void quackOfDuck() {
  for (int i = 0; i < 2; i++) {
    tone(buzzPin, 460);  //
    delay(300);
    noTone(buzzPin);
    digitalWrite(buzzPin, LOW);
    if (i == 0) {
      delay(450);
    }
  }
}
