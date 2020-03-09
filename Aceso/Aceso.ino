#include <Arduino.h>
#include <VokaWav.h>
#include <Vokaturi.h>
#include "BLE.h"

#include <SPI.h> //for the SD card module
#include <SD.h> // for the SD card
#include <RTClib.h> // for the RTC


// timers
SoftwareTimer gsrTimer;
SoftwareTimer blinkTimer;
SoftwareTimer micTimer;

int current_gsr, current_mic; 


//global variables
int mem[4] = {0,0,0,0};
int led;
double vok[512];

const int chipSelect = 10; 

// Create a file to store the data
File myFile;

// RTC
RTC_DS1307 rtc;


void setup() {
  // put your setup code here, to run once:
  //Set serial port for debug
  Serial.begin(9600);

  // setup for the RTC
    if(! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }
    else {
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    if(! rtc.isrunning()) {
      Serial.println("RTC is NOT running!");
    }
    
  // setup for the SD card
  Serial.println("Initializing SD card...");

  if(!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");
    
  //open file
  myFile=SD.open("DATA.txt", FILE_WRITE);

  // if the file opened ok, write to it:
  if (myFile) {
    Serial.println("File opened ok");
    // print the headings for our data
    myFile.println("Date,Time,Voka,GSR");
  }
  myFile.close();

  delay(10);


//  LED_init();
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  BLE_Mode_Set(ON);
  
  //Set HW interrupt on switch
  attachInterrupt(digitalPinToInterrupt(7), buttonPress, FALLING);

  //Begin sampling timers

  gsrTimer.begin(250, gsr_timer_callback);
  gsrTimer.start();
  micTimer.begin(5, mic_timer_callback);
  micTimer.start();
  blinkTimer.begin(5000, blink_timer_callback);
  blinkTimer.start();
}

int current = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if(BLE_Mode_Get() == INIT){
    BLE_init();
  }
  else if(BLE_Mode_Get() == ON) BLE_uart();
  else BLE_Mode_Set(OFF);

  Serial.print(current_gsr);
  Serial.print(",");
  Serial.println(current_mic);
    vok[current] = current_mic;
  current += 1;
  delay(10);
  
  if(current >= 512){
    reportAverageEmotions(512,vok,5000);
    current = 0;
  }
  
}

//ISR for blinking LED to ensure device functions properly
void blink_timer_callback(TimerHandle_t xTimerID){
  (void) xTimerID;
  digitalWrite(3,led);
  if(led == HIGH) led = LOW;
  else led = HIGH;
}

void mic_timer_callback(TimerHandle_t xTimerID) {
  (void) xTimerID;
  current_mic = analogRead(A1);

//  current_mic = runningAverage(mic_val);
}


//ISR for GSR data collection
void gsr_timer_callback(TimerHandle_t xTimerID){
  (void) xTimerID;
  current_gsr = analogRead(A0);
//  current_gsr = runningAverage(gsr_val);
}

//ISR for enable BLE
void buttonPress(){
  //Serial.println("Button Press");
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


void loggingTime() {
  DateTime now = rtc.now();
  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.print(now.second(), DEC);
    myFile.print(",");
  }
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.println(now.day(), DEC);
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  
  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("open with success");
    myFile.print(current_gsr);
    myFile.println(",");
  }
  myFile.close();
 
  
  myFile.close();
  delay(1000);  
}

void reportAverageEmotions (
    int numberOfSamples,   // the number of samples in the recording
    double samples [],   // the buffer that contains the recording,
                         // i.e., the indexes run from 0 through numberOfSamples-1
    double sampleRate)   // the sampling frequency in hertz
{
    VokaturiVoice voice = VokaturiVoice_create(sampleRate, numberOfSamples);
    VokaturiVoice_fill(voice, numberOfSamples, samples);
    VokaturiQuality quality;
    VokaturiEmotionProbabilities emotionProbabilities;
    VokaturiVoice_extract(voice, & quality, & emotionProbabilities);
    VokaturiVoice_destroy(voice);
    if (quality.valid) {
        Serial.println("Neutrality:");
        Serial.println(emotionProbabilities.neutrality);
        Serial.println("Happiness:");
        Serial.println(emotionProbabilities.happiness);
        Serial.println("Sadness:");
        Serial.println(emotionProbabilities.sadness);
        Serial.println("Anger:");
        Serial.println(emotionProbabilities.anger);
        Serial.println("Fear:");
        Serial.println(emotionProbabilities.fear);
    } else {
        Serial.println("This sound contains no reliably voiced parts.");
    }
}
