/*
 * Simple Programm to measure a camshaft with a simple china caliper an a stepper to rotate
 *  
 *
 * Author: Marcel Herzberg
 * Datum: 09.2022
 *
 */

#include <Arduino.h>
#include <BasicStepperDriver.h>
#include <LiquidCrystal_I2C.h>

/**********************************************************************/
// definitions

#define VERSION "1.0"

// debug macro to enable specific outputs
//#define DEBUG

// communication defs
#define STR_SIZE 32

// caliper definitions
#define PIN_DATA 12
#define PIN_CLK 10
#define SAMPLE_NUM 5

// display definition
#define LCD_CHARS 16
LiquidCrystal_I2C lcd(0x3F, LCD_CHARS, 2);

// stepper definition
#define MOTOR_STEPS 200
#define MICROSTEPS 8
#define RPM 10
#define DIR_PIN 2 //D2
#define STEP_PIN 3 //D3

BasicStepperDriver stepper(MOTOR_STEPS, DIR_PIN, STEP_PIN);

// übersetzung definieren
#define DIRECT_GEAR 25
#define SECOND_GEAR 60

// amount of degrees for one measurement
#define DEG2MEAS 180

/**********************************************************************/
// CMD definitions
// all commands has to be finished with a New Line character

// starts a complete measurement of a nocken 
// split by a space there can be entered the distance in degrees between two measurements
// if no distance is given 1 degree is default 
// example: start 0.1
const String CMD_START = "start";

// reads a sample of values from the caliper an displays the average 
// Amount of samples is defined by SAMPLE_NUM
const String CMD_READ = "read";

// reads five times from the caliper with a pause of 3sec between to test the caliper reading
const String CMD_TEST  = "test";

// rotates the camshaft a given value in degrees
// example: rotate 2.5 -> (rotates 2.5 degrees)
const String CMD_ROTATE = "rotate";
/**********************************************************************/

/**********************************************************************/
// global variables

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

float transmission = 1; // übersetzung des steppers wird berechnet in setup

/**********************************************************************/
// initialisation

void setup() {
  // put your setup code here, to run once:

  // setup pis for caliper
  pinMode(PIN_DATA, INPUT);
  pinMode(PIN_CLK, INPUT);
  
  // init serial com
  Serial.begin(9600);
  Serial.println("Nocken-Messmaschine V" VERSION);

  // init lcd display
  lcd.init();
  lcd.backlight();
  lcd.print("Nocken      V" VERSION);
  lcd.setCursor(0,1);
  lcd.print("Messmaschine" );

  stepper.begin(RPM, MICROSTEPS);
  transmission = (float)DIRECT_GEAR / (float)SECOND_GEAR;

  Serial.println("Setup done");
}

/**********************************************************************/
// read new incomming cmd at the end of loop

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    //Serial.print(inChar);
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

/**********************************************************************/
// main loop handles different commands

void loop() {
  // put your main code here, to run repeatedly:
  if (stringComplete) {

    #ifdef DEBUG
      Serial.print("debug inputString\t\t>");
      Serial.print(inputString);
    #endif

    if (inputString.startsWith(CMD_START)) {
      startCmd();
    }
    else if (inputString.startsWith(CMD_ROTATE)) {
      rotateCmd();
    }
    else if (inputString.equals(CMD_TEST + "\n")) {
      for (int i = 0; i < 5; i++){
        Serial.print(i);
        Serial.println(" turn!");
        delay(3000);
        Serial.println(sample_caliper_reading());
      }
    }
    else if (inputString.equals(CMD_READ + "\n")){      
      float reading = sample_caliper_reading();
      Serial.println(reading);
    }
    else {
      Serial.println("undefined command");
    }

    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  //stepper.rotate(720*3/72);//Grundeinstellung 720*3 ist gleich 360 Grad, durch 72 ist gleich 5Grad, durch 180Grad ist 2Grad Auflösung, durch 36Grad ist 1Grad Auflösung
} // main loop

/**********************************************************************/
// cmd functions

void startCmd() {
  float degrees = inputString.substring(CMD_START.length() + 1).toFloat(); 
  degrees = degrees <= 0 ? 1 : degrees; // set default if no give or invalid given
  int amount = (float)DEG2MEAS / degrees;

  Serial.print("start measurement (");
  Serial.print(degrees);
  Serial.print("deg distance, ");
  Serial.print(amount + 1);
  Serial.println(" values)");

  Serial.println(sample_caliper_reading()); // initial measurement

  for (int i = 0; i < amount; i++){
    stepper.rotate(transmission * degrees);
    delay(200);
    Serial.println(sample_caliper_reading());
    delay(100);
  }

  Serial.println("finished measurement");
}

void rotateCmd() {
  float degrees = inputString.substring(CMD_ROTATE.length() + 1).toFloat(); 

  if (degrees != 0) {
    // valid angle value
    printLcd2Ln("rotating ...");
    Serial.println("rotating ...");
    #ifdef DEBUG
      Serial.print("debug rotateCmd \t\t>");
      Serial.println(transmission * degrees);
    #endif
    stepper.rotate(transmission * degrees);
    printLcd2Ln("rotating done");
    Serial.println("rotating done");    
  }
  else {
    Serial.println("invalid rotate command");
  }
}

/**********************************************************************/
// lcd functions

void printLcd2Ln(char *str) {
  lcd.setCursor(0, 1);
  for(int i = 0; i < LCD_CHARS; i++) {
    lcd.print(' ');
  }
  lcd.setCursor(0, 1);
  lcd.print(str);
}

/**********************************************************************/
// caliper reading

float read_caliper(){
  int clock = 0;
  int lastClock = 0;
  int inBit = 0;
  int input = 0;
  int pos = 0;
  bool start = false;

  unsigned long startTime = 0;
  unsigned long lastTime = 0;

  while(1){ // loop read every single bit
    lastClock = clock;
    clock = digitalRead(PIN_CLK);

    if (lastClock == 1 && clock == 0) { // falling edge detected
      lastTime = startTime;
      startTime = millis();
      unsigned long duration = startTime - lastTime;

      inBit = digitalRead(PIN_DATA);

      input |= inBit << pos; // set bit at position
      pos++;

      if (pos >= 24) {
        #ifdef DEBUG
          Serial.print("debug read_caliper\t\t>");
          Serial.println(input, BIN);
        #endif

        float result = (float)input / 100.0; // calculate float value
        return result;
      }

      if (duration > 80) { // reset at start of bit sequenz
        input = 0;
        pos = 0;
      }

    }
  }
}

float sample_caliper_reading(){
  float samples[SAMPLE_NUM];
  float sum = 0.0;

  for (int i = 0; i < SAMPLE_NUM; i++){
    samples[i] = read_caliper();
    sum += samples[i];

    #ifdef DEBUG
      Serial.print("debug sample_caliper_reading\t>");
      Serial.println(samples[i]);
    #endif
  }

  return sum / SAMPLE_NUM;
}
