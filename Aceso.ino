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

  LED_init();
  pinMode(A0,INPUT);
  BLE_Mode_Set(OFF);
  
  //Set HW interrupt on switch
  attachInterrupt(digitalPinToInterrupt(7), buttonPress, FALLING);

  //Begin sampling timers
  gsrTimer.begin(500, gsr_timer_callback);
  gsrTimer.start();
  blinkTimer.begin(1000, blink_timer_callback);
  blinkTimer.start();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(BLE_Mode_Get() == INIT){
    BLE_init();
  }
  else if(BLE_Mode_Get() == ON) BLE_uart();
  else BLE_Mode_Set(OFF);
}

//ISR for blinking LED to ensure device functions properly
void blink_timer_callback(TimerHandle_t xTimerID){
  (void) xTimerID;
  digitalWrite(3,led);
  if(led == HIGH) led = LOW;
  else led = HIGH;
}

//ISR for GSR data collection
void gsr_timer_callback(TimerHandle_t xTimerID){
  (void) xTimerID;
  int gsr_val = analogRead(A0);
  current_gsr = runningAverage(gsr_val);
}

//ISR for enable BLE
void buttonPress(){
  Serial.println("Button Press");
  BLE_Mode_Set(INIT);
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
