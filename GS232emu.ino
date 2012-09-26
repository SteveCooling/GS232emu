#include <TimerOne.h>
//#include <RotatorSync3Phase.h>

int led = 13;

int send_debug = 1; // Set to 0 to avoid debug output.
int in_byte = 0;

/***********************
 *   Basic functions   *
 ***********************/

void setup() {
  pinMode(led, OUTPUT);    

  Timer1.initialize(20000);
  Timer1.attachInterrupt(tick); // attach the service routine here

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

}

void loop() {
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
}

void debug(String info) {
  if(send_debug) {
    Serial.println(info);
  }
}

void tick() {
  // Toggle LED
  digitalWrite(led, digitalRead(led) ^1);
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

