#include <TimerOne.h>

int led = 13;
int phase1 = 9;
int phase2 = 10;
int phase3 = 11;

int speedDial = 0;

int p1s = 0;
int p2s = 0;
int p3s = 0;

int hz = 20;
int steps = 6;
int cur_step = 0;

int rotation = 0;

int cnt_periods = 0;

// Gearing definitions
int fullcircle_periods = 100; //3706;
float onedegree_periods = fullcircle_periods / 360;
long cur_pos_periods = 0;
signed long target_periods = -1;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

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
  inputString.reserve(200);

  timer_init(hz);


  Serial.println("Initializing...");
  rotate_left();
  while(rotation > 0) {
    delay(20);
  }
  cur_pos_periods = 0;
  Serial.println("Ready...");

  prompt();
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString); 
    
    if(inputString.startsWith("R")) {
      rotate_right();
    }
    
    if(inputString.startsWith("L")) {
      rotate_left();
    }
    
    if(inputString.startsWith("M")) {
      rotate_to(cur_pos_periods + 50);
    }

    if(inputString.startsWith("N")) {
      rotate_to(cur_pos_periods - 50);
    }

    if(inputString.startsWith("A")) {
      rotate_stop();
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    prompt();
  }
/*
  if (Serial.available() > 0) {
    in_byte = Serial.read();
    switch (in_byte) {
    case 'H':
      help();
      break;
    case 'R':
      rotate_right();
      break;
    case 'L':
      rotate_left();
      break;
    case 'A':
      rotate_stop();
      break;
    default:
      Serial.println("ERR: Unsupported command...");
    } 
  }*/
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}

// Custom functions
void prompt() {
  Serial.print("> ");
}

long microsFromHz(int newhz) {
  long micros = 1000000 / (newhz * steps);
  //Serial.println(micros);
  return micros;
}

void timer_init(int sethz) {
  Timer1.initialize(microsFromHz(sethz));
  Timer1.attachInterrupt(tick); // attach the service routine here
  
}
int stepping(int step) {
  if(step < (steps/2)) {
    return 0;
  }
  return 1;
}

void tick() {
  int cur_step1;
  int cur_step2;
  int cur_step3;
  
  if(rotation == 1) {
    cur_step1 = cur_step % steps;
    cur_step2 = (cur_step + ((steps / 3) * 1)) % steps;
    cur_step3 = (cur_step + ((steps / 3) * 2)) % steps;
  } else {
    cur_step1 = cur_step % steps;
    cur_step2 = (cur_step + ((steps / 3) * 2)) % steps;
    cur_step3 = (cur_step + ((steps / 3) * 1)) % steps;
  }

  int duty1 = stepping(cur_step1);
  int duty2 = stepping(cur_step2);
  int duty3 = stepping(cur_step3);

  if(rotation == 0) {
    digitalWrite(phase1, 0);
    digitalWrite(phase2, 0);
    digitalWrite(phase3, 0);
  } else {
    digitalWrite(phase1, duty1);
    digitalWrite(phase2, duty2);
    digitalWrite(phase3, duty3);
  }

  // Toggle LED
  digitalWrite(led, digitalRead(led) ^1);

  //Serial.println(analogRead(speedDial));
  //hz = analogRead(speedDial) / 21;
  //setWait(hz);

  // increment "time" and roll over
  cur_step++;
  if(cur_step == steps) {
    cur_step = 0;
    if(rotation > 0) {
      cnt_periods ++;
      if(rotation == 1) {
        cur_pos_periods ++;
      }
      if(rotation == 2) {
        cur_pos_periods --;
      }
      // Stop rotation if target is hit.
      if(target_periods > -1 && cur_pos_periods == target_periods) {
        Serial.println("Target hit...");
        rotate_stop();
        target_periods = -1;
      }
      // Safeguard
      if(cnt_periods > fullcircle_periods) {
        Serial.println("Safeguard stop...");
        rotate_stop();
      }
    }
  }
}

/*********************************
 *   Execute command functions   *
 *********************************/

void help() {
  Serial.println("--- COMMAND LIST 1 ---");
  Serial.println("R  CW Rotation");
  Serial.println("L  CCW Rotation");
  Serial.println("A  CW / CCW Rotation Stop");
  //Serial.println("C  Antenna direction value");
  //Serial.println("M  Antenna direction setting. M###");
}

void rotate_right() {
  Serial.println("Rotating right...");
  cnt_periods = 0;
  rotation = 1;
}

void rotate_left() {
  Serial.println("Rotating left...");
  cnt_periods = 0;
  rotation = 2;
}

void rotate_stop() {
  Serial.println("Rotate stop...");
  rotation = 0;
  Serial.println(cnt_periods);
  Serial.println(cur_pos_periods);
}

void rotate_to(long target) {
  Serial.println("Rotate to...");
  target_periods = target;
  if(cur_pos_periods == target_periods) {
    rotate_stop();
  }

  if(cur_pos_periods > target_periods) {
    rotate_right();
  }

  if(cur_pos_periods < target_periods) {
    rotate_left();
  }
}

