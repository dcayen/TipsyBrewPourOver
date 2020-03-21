// TipsyBrew Pour Over Machine
// by Tipsy
// See http://tipsybrew.com for complete project details
// Version 0.5
// This is a rough start connecting the following components in unison
// - Load cell with HX711 amplifier
// - Diaphragm pump (12v) with L298N driver
// - SSD1306 OLED display (128x64)
// - Keypad 4x4
// Initially this is going to have a limited feature set. You may find it pretty
// rigid and mostly hard coded, however I've had a lot of odd fluke like setbacks
// so I just want to get this thing making a cup of coffee.

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include "HX711.h"
#include <Servo.h>

// ### General app definitions ###

enum AppModeValues
{
  APP_INTRO_SCREEN,
  APP_NORMAL_MODE,
  APP_CONFIRM_START,
  APP_BLOOM,
  APP_BREW,
  APP_BREW_COMPLETE
};

byte appMode = APP_INTRO_SCREEN;

byte coffeeRatio = 16;
float waterLoss = 1.995;

// ### Load cell definitions ###

#define LOADCELL_DOUT_PIN  11   // HX711 DOUT pin
#define LOADCELL_SCK_PIN  12    // HX711 SCK pin

// Calibration factor dependent on your load cell
#define calibration_factor 1695.0

float weightRead = 0;
float prevWeight = 0;
float groundWeight = 0;

HX711 scale;

// ### Screen definitions ###

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ### Keypad definitions ###

const byte ROWS = 2; 
const byte COLS = 3; 

char keys[ROWS][COLS] = {
  {'C','L','U'},
  {'D','S','R'}
};
byte rowPins[ROWS] = {7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {3, 4, 5}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// ### Pump definitions ###

// Define pin connections
#define enA 8
#define in1 9
#define in2 10

// ### SPOOOOOOOOOOON... definitions ###

#define spoonPin 13
Servo servo;
int spoonIn = 108;
int spoonOut = 75;

// ### Functions ###

void runPump();
void stopPump();
void pumpSpeed(byte dcSpeed);

// ### Pic ###
const unsigned char TipsyBrew [] PROGMEM = {
  // 'tipsybrew', 128x64px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x0f, 0xff, 0xc1, 0xff, 0x73, 0xfe, 0x3f, 0xce, 0x1d, 0xfe, 0x3f, 0xe7, 0xfb, 0x8e, 0x38, 
0x00, 0x3f, 0xff, 0xc1, 0xff, 0x7b, 0xff, 0x7f, 0xce, 0x3d, 0xff, 0x3f, 0xf7, 0xfb, 0xce, 0x38, 
0x00, 0x7f, 0xff, 0x01, 0xff, 0x7b, 0xff, 0x7f, 0xee, 0x3d, 0xff, 0x3f, 0xf7, 0xfb, 0xce, 0x78, 
0x00, 0xff, 0xf8, 0x78, 0x38, 0x7b, 0xcf, 0x71, 0xef, 0x39, 0xc7, 0x38, 0xf7, 0x81, 0xce, 0x78, 
0x01, 0xff, 0xe1, 0xf8, 0x38, 0x7b, 0xcf, 0x71, 0xe7, 0x39, 0xc7, 0x3c, 0xf7, 0x81, 0xcf, 0x70, 
0x03, 0xff, 0xc7, 0xf8, 0x38, 0x7b, 0xcf, 0x71, 0xe7, 0x79, 0xc7, 0x3c, 0xf7, 0x81, 0xdf, 0x70, 
0x07, 0xff, 0x9f, 0xfc, 0x38, 0x7b, 0xcf, 0x78, 0xc7, 0x71, 0xc7, 0x3c, 0xf7, 0x81, 0xdf, 0x70, 
0x0f, 0xff, 0x3f, 0xfc, 0x38, 0x7b, 0xcf, 0x3c, 0x03, 0xf1, 0xc7, 0x3c, 0xf7, 0x81, 0xdf, 0x70, 
0x0f, 0xff, 0x3f, 0xf8, 0x38, 0x7b, 0xcf, 0x1e, 0x03, 0xf1, 0xff, 0x3b, 0xf7, 0xf9, 0xdf, 0x70, 
0x1f, 0xfe, 0x7f, 0xf8, 0x38, 0x7b, 0xcf, 0x0f, 0x03, 0xe1, 0xfe, 0x3b, 0xf7, 0xf9, 0xff, 0x70, 
0x1f, 0xfe, 0xff, 0xf8, 0x38, 0x7b, 0xff, 0x07, 0x81, 0xe1, 0xff, 0x3b, 0xe7, 0xf8, 0xfb, 0xf0, 
0x1f, 0xfc, 0xff, 0xf0, 0x38, 0x7b, 0xff, 0x03, 0xc1, 0xe1, 0xc7, 0xbb, 0x87, 0x80, 0xfb, 0xe0, 
0x1f, 0xfc, 0xff, 0xf0, 0x38, 0x7b, 0xc4, 0x01, 0xe1, 0xc1, 0xc7, 0xbb, 0xc7, 0x80, 0xfb, 0xe0, 
0x1f, 0xf9, 0xff, 0xe0, 0x38, 0x7b, 0xc0, 0x79, 0xe1, 0xc1, 0xc7, 0xb9, 0xc7, 0x80, 0xfb, 0xe0, 
0x1f, 0xf1, 0xff, 0xc0, 0x38, 0x7b, 0xc0, 0x71, 0xe1, 0xc1, 0xc7, 0xbd, 0xe7, 0x80, 0xfb, 0xe0, 
0x0f, 0xe3, 0xff, 0x80, 0x38, 0x7b, 0xc0, 0x71, 0xe1, 0xc1, 0xc7, 0xbc, 0xe7, 0x80, 0xf9, 0xe0, 
0x0f, 0x8f, 0xff, 0x00, 0x38, 0x7b, 0xc0, 0x7f, 0xe1, 0xc1, 0xff, 0xbc, 0xe7, 0xf8, 0x71, 0xe0, 
0x00, 0x3f, 0xfe, 0x00, 0x38, 0x7b, 0xc0, 0x7f, 0xe1, 0xc1, 0xff, 0xbc, 0xf7, 0xfc, 0x71, 0xe0, 
0x01, 0xff, 0xfc, 0x00, 0x38, 0x7b, 0xc0, 0x3f, 0xc1, 0xc1, 0xff, 0x38, 0x77, 0xfc, 0x71, 0xc0, 
0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup() {
  Serial.begin(9600);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  //Assuming there is no weight on the scale at start up, reset the scale to 0
  scale.tare();

  // initialize with the I2C addr 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Turn off motors - Initial state
  stopPump();
  
  // Title screen... completely unecessary
  display.clearDisplay();
  // Display bitmap
  display.drawBitmap(0, 0,  TipsyBrew, 128, 64, WHITE);
  display.display();
  delay(5000);

  servo.attach(spoonPin);
  servo.write(spoonIn);
  delay(3000);
  servo.write(spoonOut);
  delay(1000);
  servo.detach();
  
  display.clearDisplay();
  
  // Intro
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("Place a prepared"));
  display.println(F("brewer in the machine"));  
  display.println(F("WITHOUT any grounds"));
  display.display();
  delay(1000);
  display.setCursor(10,34);
  display.println(F("Press any key to"));
  display.println(F("continue."));
  display.display();
}

void loop() {
  
  char key = keypad.getKey(); // Read the key

  switch (appMode)
  {
    case APP_INTRO_SCREEN :
      if (key) {
        scale.tare();
        display.clearDisplay();
        appMode = APP_NORMAL_MODE;
        weightRead = -100; // this forces a refresh
      }
      break;
    case APP_NORMAL_MODE :
      // Display active weight, calculate what the weight means for finished coffee
      prevWeight = weightRead;
      weightRead = scale.get_units();
      if ( abs(weightRead - prevWeight) > 0.1 ) { // refresh display
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,10);
        display.print(F("Dead beans: "));
        display.print(weightRead, 1);
        display.println(F(" g"));
        display.print(F("Water used: "));
        display.print((weightRead*coffeeRatio), 1);
        display.println(F(" ml"));
        display.print(F("Coffee made:"));
        display.print(((weightRead*coffeeRatio) - (weightRead*waterLoss)), 1);
        display.println(F(" ml")); 
        display.println(F(" "));
        display.println(F("Press UP to tare"));
        display.println(F("COFFEE NOW to Brew"));
        display.display();  
      }
      if ( key == 'U' ) {
        // Tare the scale with the A button
        scale.tare();
      } else if ( key == 'C' ) {
        // Start the brew with the * button
        appMode = APP_CONFIRM_START;
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println(F("You are about to make"));
        display.print(((weightRead*coffeeRatio) - (weightRead*waterLoss)), 1);
        display.println(F(" ml of coffee"));
        display.println(F(" "));
        display.println(F("Are you sure?"));
        display.println(F("Select to Continue"));
        display.println(F("Any key to Cancel"));
        display.display();
      }
      break;
    case APP_CONFIRM_START :
      // Confirm start with the 1 button
      if ( key == 'S' ) {
        groundWeight = weightRead;
        appMode = APP_BLOOM;
        display.clearDisplay();
      } else if ( key ) {
        appMode = APP_NORMAL_MODE;
        display.clearDisplay();
        weightRead = -100; // forces a refresh
      }
      break;
    case APP_BLOOM :
      // Dispense total water*bloom and wait
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(F("Bloom in Progress!"));
      display.println(F("Do not touch brewer"));
      display.display();
      scale.tare(); // weight from this point on is water dispensed only
      
      servo.attach(spoonPin);
      servo.write(spoonOut);
      delay(1000);
      servo.detach();
      pumpSpeed(155);
      runPump();
      while (scale.get_units() < groundWeight) {
        // possibly display weight to the user, but do they really care?
        // just do nothing for now
      }
      stopPump();
      
      servo.attach(spoonPin);
      servo.write(spoonIn);
      delay(1000);
      servo.detach();
      runPump();
      while (scale.get_units() < (2*groundWeight)) {
        // possibly display weight to the user, but do they really care?
        // just do nothing for now
      }
      stopPump();
      
      delay(1000);
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(F("Bloom WAIT"));
      display.println(F("Do not touch brewer"));
      display.print(scale.get_units(), 1);
      display.println(" g dispensed");
      display.display();
      delay(29000);
      display.clearDisplay();
      appMode = APP_BREW;
      break;
    case APP_BREW :
      pumpSpeed(125);
      // For loop dispense 150 ml of water or amount remaining then wait (150 is an arbitrary number that I *think* will not overflow the cone)
      float waterRound = scale.get_units();
      bool complete = false;
      while (scale.get_units() < (coffeeRatio * groundWeight)) { // run until we have pumped the needed water
        float waterLeft = (coffeeRatio * groundWeight) - scale.get_units();
        if ( waterLeft > 150 ) {
          waterLeft = 150;
        } else {
          complete = true;
        }
        while ( waterLeft > (scale.get_units() - waterRound) ) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println(F("Brewing Cycle!!!"));
          display.println(F("Do not touch the brewer"));
          display.print(scale.get_units(), 1);
          display.println("g dispensed");
          display.display();
          runPump();
        }
        stopPump();
        if ( complete ) {
          appMode = APP_BREW_COMPLETE;
        } else {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println(F("Drawdown Cycle!!!"));
          display.println(F("Do not touch brewer"));
          display.print(scale.get_units(), 1);
          display.println(" g dispensed");
          display.display();
          delay(20000);
          waterRound = scale.get_units();
        }
      }
      appMode = APP_BREW_COMPLETE;
      break;
    case APP_BREW_COMPLETE :
      // Display that brew is done and wait for input
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(F("Brewing is complete"));
      display.println(F("Allow time for drawdown"));
      display.println(F("if still neccessary"));
      display.println(F(" "));
      display.println(F("Press any key to"));
      display.println(F("continue"));
      display.display();
      if ( key ) {
        appMode = APP_NORMAL_MODE;
        display.clearDisplay();
      }
      break;
  }
}

void runPump() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}

void stopPump() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void pumpSpeed(byte dcSpeed) {
  analogWrite(enA, dcSpeed);
}
