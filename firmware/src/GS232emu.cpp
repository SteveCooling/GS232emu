#include <MotorThreePhase.h>

int debug = 1;

const int led = LED_BUILTIN;

MotorThreePhase azrot(9, 10, 11);

int hz_max = 65;
int hz_min = 20;

int ramp_periods = 1;

// Gearing definitions
const float periods_fullcircle = 3355;
const float periods_onedegree = periods_fullcircle / 360;
long cur_pos_periods = 0;
long target_periods = 0;

// Dummy elevator position
int el_cur_pos = 0;

void rotate_right() {
  if(debug > 0) Serial.println("ROT_RIGHT");
  azrot.rotateRight();
}

void rotate_left() {
  if(debug > 0) Serial.println("ROT_LEFT");
  azrot.rotateLeft();
}

void rotate_stop() {
  if(debug > 0) Serial.println("ROT_STOP");
  azrot.stop();
}

/*int string_to_int(String input) {
  char charBuf[10];
  input.toCharArray(charBuf, 10);
  int output = atoi(charBuf);
  return output;
}*/

/*********************************
 *   Execute command functions   *
 *********************************/

void command_list1() {
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



void rotate_to(long target) {
  if(debug > 0) {
    Serial.print("ROT_TO ");
    Serial.println(target);
  }
  target_periods = target;

  if(cur_pos_periods > target_periods) {
    //halfway_periods = cur_pos_periods - ((cur_pos_periods - target_periods) / 2);
    rotate_left();
  } else if(cur_pos_periods < target_periods) {
    //halfway_periods = cur_pos_periods + ((target_periods - cur_pos_periods) / 2);
    rotate_right();
  } else {
    rotate_stop();
  }
}

void toggle_debug() {
  debug = (debug == 0) ? 1 : 0;
  Serial.println(debug);
}

void az_set_speed(int setting) {
  switch(setting) {
    case 1:
      azrot.setSpeed(20);
      break;
    case 2:
      azrot.setSpeed(30);
      break;
    case 3:
      azrot.setSpeed(45);
      break;
    case 4:
      azrot.setSpeed(60);
      break;
    default:
      azrot.setSpeed(setting);
  }
}

void az_get_position() {
  // Not getting sprintf to work. Resorting to this rather ugly formatting..
  Serial.print("+0");
  float cur_degrees = cur_pos_periods / periods_onedegree;
  if(cur_degrees < 100) Serial.print("0");
  if(cur_degrees < 10) Serial.print("0");
  Serial.println(cur_degrees, 0);
}

void azel_get_position() {
  // Not getting sprintf to work. Resorting to this rather ugly formatting..
  Serial.print("+0");
  float cur_degrees = cur_pos_periods / periods_onedegree;
  if(cur_degrees < 100) Serial.print("0");
  if(cur_degrees < 10) Serial.print("0");
  Serial.print(cur_degrees, 0);
  Serial.print("+0");
  if(el_cur_pos < 100) Serial.print("0");
  if(el_cur_pos  < 10) Serial.print("0");
  Serial.println(el_cur_pos);
}

void az_callback(unsigned int motion) {
  if(debug > 0) Serial.println(motion);
  if(motion == MotorThreePhase::ROT_RIGHT) {
    cur_pos_periods ++;
  }
  if(motion == MotorThreePhase::ROT_LEFT) {
    cur_pos_periods --;
  }
  if(cur_pos_periods == target_periods) {
    azrot.stop();
  }
  // Endstops
  if(cur_pos_periods <= 0) {
    azrot.stop();
    cur_pos_periods = 0;
  }
  if(cur_pos_periods >= periods_fullcircle) {
    azrot.stop();
    cur_pos_periods = periods_fullcircle;
  }

  // blink led
  digitalWrite(led, digitalRead(led) ^ 1);
  if(debug > 0) Serial.println(cur_pos_periods);
}


void cmd_gs232(String cmd) {
  if(debug > 0) Serial.println(cmd); 
  
  if(cmd.startsWith("R")) {
    // Clockwise rotation
    rotate_right();
  } else if(cmd.startsWith("L")) {
    // Counter clockwise rotation
    rotate_left();
  } else if(cmd.startsWith("A")) {
    // AZ Rotation stop
    rotate_stop();
  } else if(cmd.startsWith("S")) {
    // ALL Rotation stop
    rotate_stop();
  } else if(cmd.startsWith("C2")) {
    // Get AZ+EL position
    azel_get_position();
  } else if(cmd.startsWith("C")) {
    // Get AZ position
    az_get_position();
  } else if(cmd.startsWith("I")) {
    // Toggle debug
    toggle_debug();
  } else if(cmd.startsWith("H")) {
    // Command list
    command_list1();
  } else if(cmd.startsWith("M")) {
    // AZ Rotate to
    int in_degrees = cmd.substring(1,4).toInt();
    rotate_to(periods_onedegree * in_degrees);
  } else if(cmd.startsWith("W")) {
    // AZ+EL Rotate to
    int in_degrees = cmd.substring(1,4).toInt();
    rotate_to(periods_onedegree * in_degrees);
  } else if(cmd.startsWith("X")) {
    // AZ rotation speed
    int in_speed = cmd.substring(1).toInt();
    az_set_speed(in_speed);
  } else {
    Serial.print("? >");
  }
 Serial.print("\r");
}

String serial_cmd = ""; // hold incoming commands

void handleSerial() {
  while (Serial.available()) {
    char inChar = (char)Serial.read(); 
    serial_cmd += inChar;

    // react to command
    if (inChar == '\n' || inChar == '\r') {
      cmd_gs232(serial_cmd);
      serial_cmd = "";
    }
  }
}

// Setup and main loop

void setup() {
  pinMode(led, OUTPUT);    

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  serial_cmd.reserve(32);

  azrot.setCallback(az_callback);
  az_set_speed(1);
}

void loop() {
  azrot.update(micros());
  handleSerial();
}
