#include <Arduino.h>
#include "BLE.h"

// timers
SoftwareTimer gsrTimer;
SoftwareTimer blinkTimer;

//global variables
int mem[4] = {0,0,0,0};
int current_gsr;
int led;

void setup() {
  // put your setup code here, to run once:
  //Set serial port for debug
  Serial.begin(115200);

  BLE_init();
  
  //initialize ports for LED and sensors
  LED_init();
  pinMode(A0,INPUT);

  //Begin sampling timers
  gsrTimer.begin(500, gsr_timer_callback);
  gsrTimer.start();
  blinkTimer.begin(1000, blink_timer_callback);
  blinkTimer.start();
}

void loop() {
  // put your main code here, to run repeatedly:
  BLE_uart();
}

void blink_timer_callback(TimerHandle_t xTimerID){
  (void) xTimerID;
  digitalWrite(3,led);
  if(led == HIGH) led = LOW;
  else led = HIGH;
}

void gsr_timer_callback(TimerHandle_t xTimerID){
  (void) xTimerID;
  int gsr_val = analogRead(A0);
  current_gsr = runningAverage(gsr_val);
}
  
int runningAverage(int n){
  int average = (n + mem[0] + mem[1] + mem[2] + mem[3]) / 5;
  mem[3] = mem[2];
  mem[2] = mem[1];
  mem[1] = mem[0];
  mem[0] = average;
  return average;
}

void LED_init(){
  pinMode(3,OUTPUT);
  led = HIGH;
}
