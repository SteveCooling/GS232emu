#include <TimerOne.h>
 
int led = 13;

int send_debug = 1; // Set to 0 to avoid debug output.

void setup() {
  pinMode(led, OUTPUT);    
  
  Timer1.initialize(250000);
  Timer1.attachInterrupt(callback); // attach the service routine here

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

}
 
void loop() {
    // Dette skjer uavhengig av at LED blinker.
}

void debug(String info) {
  if(send_debug) {
    Serial.println(info);
  }
}

void callback() {
    // Toggle LED
    digitalWrite(led, digitalRead(led) ^1);
}

