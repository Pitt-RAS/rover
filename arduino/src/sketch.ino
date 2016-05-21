//============================================================================
// Arduino Code for Pitt RAS Rover
// 2015
//----------------------------------------------------------------------------
// Each command is a 10-character word sent over from the pi.
// 0 = char : Function command (m for Motor)
// 1 = char : Parameter 1 (f or b for front or back)
// 2 = char : Parameter 2 (l or r for left or right)
// 3-9 = float as string : Paramter 3 (motor speed)
// A few commands may use Parameters 1 and 2 together in different ways,
// most often specifying a pin. (Ex: 08 or A2)
//----------------------------------------------------------------------------
// The following commands are currently available:
//----------------------------------------------------------------------------
// m (Set motor speed)
// - f(ront), b(ack), or a(ll) motor sets
// - l(eft), r(ight), or a(ll) wheels on that motor set
// - motor speed
// ex; mfr100.000:
//----------------------------------------------------------------------------
// r (Read pin Digital to Serial)
// - characters are used together for pin id
// - float value is always 0000000 (ignored)
// ex; r090000000:
//----------------------------------------------------------------------------
// w (Write pin Digital)
// - characters are used together for pin id
// - float value is either 0000000 or 0000001 for LOW and HIGH respectively
// ex; w120000001:
//----------------------------------------------------------------------------
// R (Read pin Analog to Serial)
// - characters are used together for pin id
// - float value is always 0000000 (ignored)
// ex; RA10000000:
//----------------------------------------------------------------------------
// W (Write pin Analog)
// - characters are used together for pin id
// - float used as integer value from 0 to 255 for duty cycle
// ex; WA30000100:
//----------------------------------------------------------------------------
// T (Tone start)
// - characters are used together for pin id
// - float for tone frequency
// Note that if another tone is already playing, it will be stopped.
// If 00 is sent as pinID, all tones will stop
// ex; TA3415.305:
//----------------------------------------------------------------------------
// t (Set tone duration)
// - characters are unused (put 00)
// - integer for the duration of the following tones in milliseconds
// Note that if the duration is set to 0, the tone will not stop automatically,
// which is the default behavior.
// ex; t000000100:
//----------------------------------------------------------------------------
// s (Set servo angle)
// - first character = servoId. h = the horizontal axis servo v= the vertical access servo.
// - unused, can be anything
// - float for angle valid range is 0 to 180
//----------------------------------------------------------------------------
// p (read ping sensor)
// - characters are used together for sensor id
// - float value is ignored
// ex; pbr0000:
//----------------------------------------------------------------------------

#include <Arduino.h>
#include "PiCom.h"
#include "NewPing.h"
#include "Adafruit_TiCoServo.h"
#include "Adafruit_NeoPixel.h"
#include "NeoPixelController.h"
// If a pin was previously used for OUTPUT, it cannot be swapped to INPUT and reverse
#define STRICT_PINS true

// Motor pins
#define MOTOR_BL_PWM_PIN 10
#define MOTOR_BL_GPIO_PIN 11
#define MOTOR_BR_PWM_PIN 8
#define MOTOR_BR_GPIO_PIN 9

#define MOTOR_FR_PWM_PIN 4
#define MOTOR_FR_GPIO_PIN 5
#define MOTOR_FL_PWM_PIN 6
#define MOTOR_FL_GPIO_PIN 7

#define SERVO1_PIN 3
#define SERVO2_PIN 2

// Servo Center
#define SERVO_V_CENTER 90
#define SERVO_H_CENTER 90

// LED's
#define RGB_LEDS 98
#define RGB_PIN 41

#define SERVO_SPEED 150.0/1000.0 //In degrees a millisecond

// Keep track if pins are set to INPUT or OUTPUT
byte pinModes[30];
// The last error raised
char errorReport[30];

// How long a tone should play for
int toneDuration = 0;
// Which pin is playing a tone
byte tonePin = 0;

// Cut down on some conditionals by picking from an array
byte motorPins[] = {
  MOTOR_FL_PWM_PIN, MOTOR_FR_PWM_PIN, MOTOR_BL_PWM_PIN, MOTOR_BR_PWM_PIN,
  MOTOR_FL_GPIO_PIN, MOTOR_FR_GPIO_PIN, MOTOR_BL_GPIO_PIN, MOTOR_BR_GPIO_PIN
};

NewPing ping_sensors[] = {
  NewPing(53, 51),
  NewPing(49, 47),
  NewPing(45, 43),
  NewPing(35, 37),
  NewPing(31, 33),
  NewPing(27, 29),
  NewPing(23, 25)
};

// Analog pins, for easy access in getPin
byte aPins[] = {A0, A1 ,A2, A3, A4, A5, A6, A7};

// Servo objects one for horizontal and vertical axis
Adafruit_TiCoServo  h_servo, v_servo;  // create servo object to control a servo
float h_servo_pos, h_servo_pos_current;
float v_servo_pos, v_servo_pos_current;

NeoPixelController led_strip(RGB_LEDS, RGB_PIN);

void (*command_handlers[])(uint8_t*) = {
  &runMotorCommand,     // Command 0
  &readPinDigital,      // Command 1
  &writePinDigital,     // Command 2
  &readPinAnalog,       // Command 3
  &writePinDigital,     // Command 4
  &writeServoPosition,  // Command 5
  &readPing,            // Command 6
  &ledSet               // Command 7
};

//----------------------------------------------------------------------------
// setup
// Runs once when the program starts
//----------------------------------------------------------------------------
void setup() {
  
  led_strip.begin();
  led_strip.setPattern(NeoPixelController::PATTERN_CHASER, 10, 0, 0, 64);
  
  PiComInit();
  
  Serial1.begin(115200);

  // Set the pin mode for each motor pin, and mark it for future reference
  for (int k = 0; k < 8; k++) {
    pinModes[motorPins[k]] = OUTPUT + 1;
    pinMode(motorPins[k], OUTPUT);
  }
  
  h_servo.attach(SERVO1_PIN);  // attaches the servo on pin A0 to the servo object
  v_servo.attach(SERVO2_PIN);  // attaches the servo on pin A1 to the servo object
  //Write initial position, should be centered
  h_servo.write(SERVO_H_CENTER);
  v_servo.write(SERVO_V_CENTER);
  h_servo_pos = SERVO_H_CENTER;
  h_servo_pos_current = SERVO_H_CENTER;
  v_servo_pos = SERVO_V_CENTER;
  v_servo_pos_current = SERVO_V_CENTER;
}

//----------------------------------------------------------------------------
// loop
// Main program loop
//----------------------------------------------------------------------------
void loop() {
  //Check for new command
  char command=3;
  uint8_t command_data[kPiComArguments];
    
  if(PiComGetData(command_data))
  {
    command_handlers[command_data[0]](&command_data[1]);
  }
  
  // Handle moving the servos
  static long time_since_update = millis();
  long temp_time = millis();
  long delta_t = temp_time - time_since_update;
  time_since_update = temp_time;
  
  v_servo_pos_current += constrain(v_servo_pos-v_servo_pos_current, -1, 1) * ((float)delta_t * (float)SERVO_SPEED);
  h_servo_pos_current += constrain(h_servo_pos-h_servo_pos_current, -1, 1) * ((float)delta_t * (float)SERVO_SPEED);

  v_servo.write(v_servo_pos_current);
  h_servo.write(h_servo_pos_current);  
  
  led_strip.update();
  
}

//----------------------------------------------------------------------------
// processCommand
// Switch off to the different specified functions
// cmd is the first character of the command, it signifies while case statement will execute
// arg3_f floating point payload
// args is a 2 character array of the two argument characters
//----------------------------------------------------------------------------
/*void processCommand(char cmd, float arg3_f, char* args) {
  switch (cmd) {
    case 'm' : // Motor Speed Command
      runMotorCommand(arg3_f, args[0], args[1]);
      break;
    case 'r' : // Read Pin Digital
      readPinDigital(getPin(args));
      break;
    case 'w' : // Write Pin Digital
      writePinDigital(getPin(args), (arg3_f == 0 ? LOW : HIGH));
      break;
    case 'R' : // Read Pin Analog
      readPinAnalog(getPin(args));
      break;
    case 'W' : // Write Pin Analog
      writePinDigital(getPin(args), ((int) arg3_f) % 256);
      break;
    case 'T' : // Play tone
      playTone(getPin(args), (int) arg3_f);
      break;
    case 't' : // Set tone duration
      toneDuration = (int) arg3_f;
      break;
    case 's' : // Set servo position
      //Serial.println("s called");
      writeServoPosition(args, arg3_f);
      break;
    case 'p' : // Read Ping sensor
      readPing(args);
      break;
    case 'l' : // Led command
      switch (args[0])
      {
        case 'o' :
          led_strip.setPattern(NeoPixelController::PATTERN_OFF, 1000, 0, 0, 0);
          break;
        case 's' :
          led_strip.setPattern(NeoPixelController::PATTERN_SOLID, 1000, args[1], args[2], args[3]);
          break;
        case 'r' :
          led_strip.setPattern(NeoPixelController::PATTERN_RAINBOW, arg3_f, args[1], args[2], args[3]);
          break;
        case 'c' :
          led_strip.setPattern(NeoPixelController::PATTERN_CHASER, arg3_f, args[1], args[2], args[3]);
      }
      break;
  }
}*/
//----------------------------------------------------------------------------
// runMotorCommand
// Figure out which wheels to turn on
//----------------------------------------------------------------------------
void runMotorCommand(uint8_t* command) {
 
  int forwardPercent = (int)command[0] - 100; //100 - 100 = 0
  int rotationPercent = (int)command[1] - 100; //200 - 100 = 100
    
  int pwml = (forwardPercent + rotationPercent) * 255 / 100; // Forward plus rotation on that side
  int pwmr = (forwardPercent - rotationPercent) * 255 / 100; // Forward minus rotation, no need to subtract because the motor is on the other side of the robot
  
  Serial1.println(pwmr);
  
  // Front Left
  if(pwml >= 0){ // Forward
    digitalWrite(MOTOR_FL_GPIO_PIN, LOW);
    analogWrite(MOTOR_FL_PWM_PIN, pwml);
  }
  else{ // Reverse
    digitalWrite(MOTOR_FL_GPIO_PIN, HIGH);
    analogWrite(MOTOR_FL_PWM_PIN, 255 + pwml);
  }
  
  // Front Right
  if(pwmr >= 0){ // Forward
    digitalWrite(MOTOR_FR_GPIO_PIN, HIGH);
    analogWrite(MOTOR_FR_PWM_PIN, 255 - pwmr);
  }
  else{ // Reverse
    digitalWrite(MOTOR_FR_GPIO_PIN, LOW);
    analogWrite(MOTOR_FR_PWM_PIN, abs(pwmr));
  }
  
  // Back Left
  if(pwml >= 0){ // Forward
    digitalWrite(MOTOR_BL_GPIO_PIN, HIGH);
    analogWrite(MOTOR_BL_PWM_PIN, 255 - pwml);
  }
  else{ // Reverse
    digitalWrite(MOTOR_BL_GPIO_PIN, LOW);
    analogWrite(MOTOR_BL_PWM_PIN, abs(pwml));
  }
  
  // Back Right
  if(pwmr >= 0){ // Forward
    digitalWrite(MOTOR_BR_GPIO_PIN, LOW);
    analogWrite(MOTOR_BR_PWM_PIN, pwmr);
  }
  else{ // Reverse
    digitalWrite(MOTOR_BR_GPIO_PIN, HIGH);
    analogWrite(MOTOR_BR_PWM_PIN, 255 + pwmr);
  }
  
  
}
//----------------------------------------------------------------------------
// applyPinType
// Changes the pin type as needed.
// Provides an error if pin has already been decided on
//----------------------------------------------------------------------------
boolean applyPinType(byte pinID, byte type) {
  
  return true;
  if (pinModes[pinID] == type + 1) {
    return true;
  } else if (pinModes[pinID] == 0 || !STRICT_PINS) {
    pinModes[pinID] = type + 1;
    pinMode(pinID, type);
    return true;
  }
  // Pin has already been set to a different type, and strict pins is enabled.
  // Raise an error, and don't allow the command to complete
  //sprintf(errorReport, "Pin %i is %s", pinID, (pinModes[pinID] == OUTPUT + 1 ? "OUTPUT" : "INPUT"));
  //Serial.println("-1");
  //Serial.println(errorReport);
  return false;
}

//----------------------------------------------------------------------------
// getPin
// Gets the pin number from a set of characters.
// Works for pins labeled A and such as well.
//----------------------------------------------------------------------------
byte getPin(char pin[]) {
  //Acknowledge that the command was successfully received
  if (pin[0] == 'A') {
    return aPins[atoi(pin + 1)]; 
  }
  return atoi(pin);
}

//----------------------------------------------------------------------------
// writeMotorSpeed
// Sets one of the pre-defined motors to a particular speed
//----------------------------------------------------------------------------
void writeMotorSpeed(float mspeed, byte pwm_pin, byte gpio_pin) {

  if (mspeed >= 0) {
    digitalWrite(gpio_pin, 0);
    analogWrite(pwm_pin, mspeed * 255);
  } else {
    digitalWrite(gpio_pin, 1);
    analogWrite(pwm_pin, (1 + mspeed) * 255);
  }
}

//----------------------------------------------------------------------------
// readPinDigital
// Reads the value of a digital pin, and writes either 0 or 1
// This is a stub
//----------------------------------------------------------------------------
void readPinDigital(uint8_t* command) {
  // Check if pin type set to read
  if (applyPinType(command[0], command[1])) {
    // Read the pin
    //Serial.println(digitalRead(pinID));
  }
}

//----------------------------------------------------------------------------
// writePinDigital
// Writes a value to a particular pin
//----------------------------------------------------------------------------
void writePinDigital(uint8_t* command) {
  uint8_t pinID = command[0];
  uint8_t output = command[1];
  // Check if pin type set to write
  if (applyPinType(pinID, OUTPUT)) {
    // Write to the pin
    digitalWrite(pinID, output);
  }
}

//----------------------------------------------------------------------------
// readPinAnalog
// Reads the value of an analog pin, and prints a number from 0 to 1024.
//----------------------------------------------------------------------------
void readPinAnalog(uint8_t* command) {
  // Check if pin type set to read
  if (applyPinType(command[0], INPUT)) {
    // Read the pin
    PiComSendData((uint16_t)analogRead(command[0]));
  }
}

//----------------------------------------------------------------------------
// writePinAnalog
// Writes a value to a particular pin
// Note that this generates a square wave on that pin, with the output
// value being the duty cycle. If you want to specify frequency, use playTone
//----------------------------------------------------------------------------
void writePinAnalog(uint8_t* command) {
  uint8_t pinID = command[0];
  uint8_t output = command[1];
  // When writing, you do not need to set the pin type to output,
  // However, strict mode should prevent us from doing anything
  if (applyPinType(pinID, OUTPUT)) {
    // Write to the pin
    analogWrite(pinID, output);
  }
}

//----------------------------------------------------------------------------
// writeServoPosition
// write a position to a servo
//----------------------------------------------------------------------------
void writeServoPosition(uint8_t* command) {
  float angle;
  memcpy(&angle, &command[1], 4);
  
  switch(command[0]){
    case 'v':
      v_servo_pos = constrain(SERVO_V_CENTER - angle, 20, 180);
      break;
    case 'h':
      h_servo_pos = constrain(SERVO_H_CENTER + angle, 0, 180);
      break;
  }
}
//----------------------------------------------------------------------------
// readPing
//----------------------------------------------------------------------------
void readPing(uint8_t* command) {
  int index = -1;
  switch (command[0]) {
    case 'f':
      switch (command[1]) {
        case 'r':
          index = 6;
          break;
        case 'l':
          index = 5;
          break;
      }
      break;
    case 'r':
      index = 4;
      break;
    case 'l':
      index = 3;
      break;
    case 'b':
      switch (command[1]) {
        case 'r':
          index = 2;
          break;
        case 'l':
          index = 1;
          break;
        case ' ':
          index = 0;
          break;
      }
      break;
  }
  if (index >= 0) {
    PiComSendData(0.0f);
    //PiComSendData((float)ping_sensors[index].ping_cm());
  }
}

void ledSet(uint8_t* command)
{
  float period;
  memcpy(&period, &command[4], 4);
  
  switch (command[0])
      {
        case 'o' :
          led_strip.setPattern(NeoPixelController::PATTERN_OFF, 1000, 0, 0, 0);
          break;
        case 's' :
          led_strip.setPattern(NeoPixelController::PATTERN_SOLID, 1000, command[1], command[2], command[3]);
          break;
        case 'r' :
          led_strip.setPattern(NeoPixelController::PATTERN_RAINBOW, period, command[1], command[2], command[3]);
          break;
        case 'c' :
          led_strip.setPattern(NeoPixelController::PATTERN_CHASER, period, command[1], command[2], command[3]);
      }
}
