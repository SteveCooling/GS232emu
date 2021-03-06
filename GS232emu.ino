#include <TimerOne.h>

int debug = 0;
int skip_init = 1;

int led = 13;
int phase1 = 9;
int phase2 = 10;
int phase3 = 11;

int p1s = 0;
int p2s = 0;
int p3s = 0;

int hz_max = 65;
int hz_min = 15;
int hz = 60;
int autospeed = 0;
float ramp_speed = 0.6;

int steps = 6;
int cur_step = 0;

int rotation = 0;

int init_status = 1;

int cnt_periods = 0;

// Gearing definitions
float fullcircle_periods = 3355;
float onedegree_periods = fullcircle_periods / 360;
long cur_pos_periods = 0;

int el_cur_pos = 0;

signed int target_periods = -1;
int halfway_periods = -1;

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

  if(skip_init == 0) {
    if(debug > 0) Serial.println("init");
    rotate_left();
    while(rotation > 0) {
      delay(20);
    }
    cur_pos_periods = 0;
    }
  init_status = 0;

  //prompt();
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString); 
    
    if(inputString.startsWith("R")) {
      rotate_right();
    } else if(inputString.startsWith("L")) {
      rotate_left();
    } else if(inputString.startsWith("A")) {
      if(autospeed == 1) {
        if(rotation == 1) {
          halfway_periods = cur_pos_periods;
          target_periods = cur_pos_periods + (hz_max - hz_min);
        } else if(rotation == 2) {
          halfway_periods = cur_pos_periods;
          target_periods = cur_pos_periods - (hz_max - hz_min);
        }
      } else {
        rotate_stop();
      }
    } else if(inputString.startsWith("S")) {
      if(autospeed == 1) {
        if(rotation == 1) {
          halfway_periods = cur_pos_periods;
          target_periods = cur_pos_periods + (hz_max - hz_min);
        } else if(rotation == 2) {
          halfway_periods = cur_pos_periods;
          target_periods = cur_pos_periods - (hz_max - hz_min);
        }
      } else {
        rotate_stop();
      }
    } else if(inputString.startsWith("C2")) {
      azel_get_position();
    } else if(inputString.startsWith("C")) {
      az_get_position();
    } else if(inputString.startsWith("I")) {
      flip_debug();
    } else if(inputString.startsWith("H")) {
      help();
    } else if(inputString.startsWith("M")) {
      float in_degrees = string_to_int(inputString.substring(1));
      rotate_to(onedegree_periods * in_degrees);
    } else if(inputString.startsWith("X")) {
      int in_speed = string_to_int(inputString.substring(1));
      az_set_speed(in_speed);
    } else {
      Serial.print("? >");
    }

    Serial.print("\r");
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    //prompt();
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
  } else if (rotation == 2) {
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
      if(debug > 0) Serial.println(cur_pos_periods);
      if(debug > 0) Serial.println(target_periods);
      
      if(autospeed == 1) {
        // Take care of ramping up speed
        if(cnt_periods < ((hz_max - hz_min) * ramp_speed)) {
          // Within ramp-up "area". Calculate speed to set.
          int now_hz = (cnt_periods / ramp_speed) + hz_min;
          timer_init(now_hz);
        }
        // Ramp down speed
        if(rotation == 1) {
          // When rotating right
          // Are we nearing target?
          if(target_periods > -1 && cur_pos_periods > halfway_periods && cur_pos_periods > (target_periods - ((hz_max - hz_min) * ramp_speed))) {
            int now_hz = ((target_periods - cur_pos_periods) / ramp_speed) + hz_min;
            timer_init(now_hz);
          } else if(cur_pos_periods > (fullcircle_periods - ((hz_max - hz_min) * ramp_speed))) {
            // Nearing max rotation
            int now_hz = ((fullcircle_periods - cur_pos_periods) / ramp_speed) + hz_min;
            timer_init(now_hz);
          }
        } else if(rotation == 2) {
          // Rotating left
          if(target_periods > -1 && cur_pos_periods < halfway_periods && cur_pos_periods < (target_periods + ((hz_max - hz_min) * ramp_speed))) {
            // Nearing target
            int now_hz = ((cur_pos_periods - target_periods) / ramp_speed) + hz_min;
            timer_init(now_hz);
          } else if(cur_pos_periods < ((hz_max - hz_min) * ramp_speed)) {
            // Nearing min rotation
            int now_hz = (cur_pos_periods / ramp_speed) + hz_min;
            timer_init(now_hz);
          }
        }
        
      }
      
      if(init_status == 0) {
        // Stop rotation if target is hit.
        if(target_periods > -1 && cur_pos_periods == target_periods) {
          if(debug > 0) Serial.println("HT");
          rotate_stop();
          target_periods = -1;
        }
        
        // Stop at edges
        if(cur_pos_periods <= 0) {
          if(debug > 0) Serial.println("H0");
          rotate_stop();
        }
        if(cur_pos_periods >= fullcircle_periods) {
          if(debug > 0) Serial.println("H360");
          rotate_stop();
        }
      }
    
      // Safeguard
      if(cnt_periods > fullcircle_periods) {
        if(debug > 0) Serial.println("SS");
        rotate_stop();
      }
    }
  }
}

int string_to_int(String input) {
  char charBuf[10];
  input.toCharArray(charBuf, 10);
  int output = atoi(charBuf);
  return output;
}

/*********************************
 *   Execute command functions   *
 *********************************/

void help() {
  Serial.println("--- COMMAND LIST 1 ---");
  Serial.println("R  CW Rotation");
  Serial.println("L  CCW Rotation");
  Serial.println("A  CW / CCW Rotation Stop");
  Serial.println("C  Current azimuth");
  Serial.println("M  Antenna direction setting. MXXX");
  Serial.println("X  Rotation speed. X x, x=1-4");
  //Serial.println("C  Antenna direction value");
  //Serial.println("M  Antenna direction setting. M###");
}

void rotate_right() {
  if(debug > 0) Serial.println("RR");
  cnt_periods = 0;
  if(init_status == 0 && cur_pos_periods >= fullcircle_periods) return;
  rotation = 1;
}

void rotate_left() {
  if(debug > 0) Serial.println("RL");
  cnt_periods = 0;
  if(init_status == 0 && cur_pos_periods <= 0) return;
  rotation = 2;
}

void rotate_stop() {
  if(debug > 0) Serial.println("RS");
  rotation = 0;
  target_periods = -1;
  if(debug > 0) Serial.println(cnt_periods);
  if(debug > 0) Serial.println(cur_pos_periods);
}

void rotate_to(long target) {
  if(debug > 0) Serial.println("RT");
  target_periods = target;

  if(cur_pos_periods > target_periods) {
    halfway_periods = cur_pos_periods - ((cur_pos_periods - target_periods) / 2);
    rotate_left();
  } else if(cur_pos_periods < target_periods) {
    halfway_periods = cur_pos_periods + ((target_periods - cur_pos_periods) / 2);
    rotate_right();
  } else {
    rotate_stop();
  }
}

void flip_debug() {
  if(debug == 0) {
    debug = 1;
  } else {
    debug = 0;
  }
}

void az_set_speed(int setting) {
  switch(setting) {
    case 1:
      autospeed = 0;
      timer_init(20);
      break;
    case 2:
      autospeed = 0;
      timer_init(33);
      break;
    case 3:
      autospeed = 0;
      timer_init(46);
      break;
    case 4:
      autospeed = 0;
      timer_init(60);
      break;
    case 5:
      autospeed = 1;
      timer_init(20);
      break;
    default:
      timer_init(setting);
  }
}

void az_get_position() {
  // Not getting sprintf to work. Resorting to this rather ugly formatting..
  Serial.print("+0");
  float cur_degrees = cur_pos_periods / onedegree_periods;
  if(cur_degrees < 100) Serial.print("0");
  if(cur_degrees < 10) Serial.print("0");
  Serial.println(cur_degrees, 0);
}

void azel_get_position() {
  // Not getting sprintf to work. Resorting to this rather ugly formatting..
  Serial.print("+0");
  float cur_degrees = cur_pos_periods / onedegree_periods;
  if(cur_degrees < 100) Serial.print("0");
  if(cur_degrees < 10) Serial.print("0");
  Serial.print(cur_degrees, 0);
  Serial.print("+0");
  if(el_cur_pos < 100) Serial.print("0");
  if(el_cur_pos  < 10) Serial.print("0");
  Serial.println(el_cur_pos);
}


