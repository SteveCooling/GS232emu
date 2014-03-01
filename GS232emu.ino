#include <TimerOne.h>

unsigned long previousMillis = 0;

int led = 13;
int phase1 = 9;
int phase2 = 10;
int phase3 = 11;

int speedDial = 0;

int p1s = 0;
int p2s = 0;
int p3s = 0;

int hz = 1;
int steps = 12;
int cur_step = 0;

float rad_step = 6.283 / steps;

unsigned long wait = 1000;
int in_byte;

/***********************
 *   Basic functions   *
 ***********************/

void setup() {
  pinMode(led, OUTPUT);    
  pinMode(phase1, OUTPUT);
  pinMode(phase2, OUTPUT);
  pinMode(phase3, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  setWait(hz);

  //Timer1.initialize(wait);
  //Timer1.attachInterrupt(tick); // attach the service routine here

  Serial.println("===");
  //Serial.println(hz);
  //Serial.println(steps);
  Serial.println(wait);
}

void setWait(int newhz) {
  wait = 1000 / (newhz * steps);
  if(wait > 1000) wait = 1000;
}

void loop() {
  //unsigned long currentMillis = millis();
  //if(currentMillis - previousMillis > wait) {
    tick();
  //}

  if (Serial.available() > 0) {
    in_byte = Serial.read();
    switch (in_byte) {
    case 'H':
      help();
      break;
    case 'R':
      rotate_right();
      break;
    default:
      Serial.println("ERR: Unsupported command...");
    } 
  }
  delay(wait);
}

void tick() {
  int cur_step1 = cur_step;
  int cur_step2 = cur_step + (steps / 3);
  int cur_step3 = cur_step + ((steps / 3) *2);

  float duty1 = ((sin(cur_step1 * rad_step)+1)/2);
  float duty2 = ((sin(cur_step2 * rad_step)+1)/2);
  float duty3 = ((sin(cur_step3 * rad_step)+1)/2);
  
  if(wait >= 1000) {
    analogWrite(phase1, 0);
    analogWrite(phase2, 0);
    analogWrite(phase3, 0);
  } else {
    analogWrite(phase1, (int)255*duty1);
    analogWrite(phase2, (int)255*duty2);
    analogWrite(phase3, (int)255*duty3);
  }

  //Serial.println((int)255*duty1);

  // Toggle LED
  digitalWrite(led, digitalRead(led) ^1);

  //Serial.println(analogRead(speedDial));
  hz = analogRead(speedDial) / 21;
  setWait(hz);

  // increment "time"
  cur_step++;
  if(cur_step == steps) cur_step = 0;
  
}

/*********************************
 *   Execute command functions   *
 *********************************/

void help() {
  Serial.println("--- COMMAND LIST 1 ---");
  Serial.println("R  CW Rotation");
  Serial.println("L  CCW Rotation");
  Serial.println("A  CW / CCW Rotation Stop");
  Serial.println("C  Antenna direction value");
  Serial.println("M  Antenna direction setting. M###");
}

void rotate_right() {
  Serial.println("Rotating right...");
}

