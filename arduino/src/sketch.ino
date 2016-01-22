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

// How many characters should be back-logged for the input buffer
#define INPUT_BUFFER_SIZE 200
// How many commands can be stored for execution at once
#define COMMAND_BUFFER_SIZE 25
// The character that seperates individual commands
#define COMMAND_SEPERATOR ':'
// If a pin was previously used for OUTPUT, it cannot be swapped to INPUT and reverse
#define STRICT_PINS true

// Motor pins
#define MOTOR_FL_PWM_PIN 5
#define MOTOR_FR_PWM_PIN 11
#define MOTOR_BL_PWM_PIN 9
#define MOTOR_BR_PWM_PIN 10
#define MOTOR_FL_GPIO_PIN 2
#define MOTOR_FR_GPIO_PIN 3
#define MOTOR_BL_GPIO_PIN 4
#define MOTOR_BR_GPIO_PIN 8

// Buffer for serial input
char inputBuffer[INPUT_BUFFER_SIZE];
byte inputBufferSize = 0;
// Keep track if pins are set to INPUT or OUTPUT
byte pinModes[30];
// The last error raised
char errorReport[30];

// How long a tone should play for
int toneDuration = 0;
// Which pin is playing a tone
byte tonePin = 0;

// The commands to be executed
char commands[COMMAND_BUFFER_SIZE][10];
byte commandsSize = 0;

// Cut down on some conditionals by picking from an array
byte motorPins[] = {
  MOTOR_FL_PWM_PIN, MOTOR_FR_PWM_PIN, MOTOR_BL_PWM_PIN, MOTOR_BR_PWM_PIN,
  MOTOR_FL_GPIO_PIN, MOTOR_FR_GPIO_PIN, MOTOR_BL_GPIO_PIN, MOTOR_BR_GPIO_PIN
};
// Analog pins, for easy access in getPin
byte aPins[] = {A0,A1,A2,A3,A4,A5};

// Battery voltage flag
volatile bool battery_interrupt_fired = false;
volatile bool battery_low = false;

//----------------------------------------------------------------------------
// setup
// Runs once when the program starts
//----------------------------------------------------------------------------
void setup() {
  // Start Serial
  Serial.begin(115200);
  Serial.setTimeout(10);
  // Set the pin mode for each motor pin, and mark it for future reference
  for (int k = 0; k < 8; k++) {
    pinModes[motorPins[k]] = OUTPUT + 1;
    Serial.println("Can we even get inside the setup?");  
  pinMode(motorPins[k], OUTPUT);
  }

  // Enable analog comparator for battery voltage
  ACSR |= _BV(ACIE) | _BV(ACIS1);
}

//----------------------------------------------------------------------------
// loop
// Main program loop
//----------------------------------------------------------------------------
void loop() {
  // Send pending responses
  if (battery_interrupt_fired) {
    battery_interrupt_fired = false;

    // Disable battery compare interrupt
    ACSR &= ~_BV(ACIE);

    Serial.print("b000000000:");
  }

  // Accept new commands
  byte newChars = Serial.readBytesUntil('\n', inputBuffer + inputBufferSize, INPUT_BUFFER_SIZE - inputBufferSize);
  inputBufferSize += newChars;
  while (inputBufferSize > 10 && inputBuffer[inputBufferSize - 1] == COMMAND_SEPERATOR && commandsSize < COMMAND_BUFFER_SIZE) {
    // Add to be executed
    Serial.println("Reached the point where commands can actually be parsed (memcpy(commands[commandsSize] etc.))");
    memcpy(commands[commandsSize], &inputBuffer[inputBufferSize - 11], 10);
    inputBufferSize -= 11; // Removes 10 characters + separator
    commandsSize++;
  }
  // Execute the commands FIFO.
  if (commandsSize > 0) {
    Serial.println("We have now started executing the commands FIFO.");
    for (int k = commandsSize - 1; k >= 0; k--) {
      char args[] = {commands[k][1], commands[k][2]};
      processCommand(commands[k][0], atof(commands[k] + 3), args);
    }
    commandsSize = 0;
  }
  // Do some clean-up here if too much stuff
  //if (inputBufferSize >= INPUT_BUFFER_SIZE * 0.5) {
  //  Serial.println("Now we're cleaning things up because there's too much stuff. ");
  //  for (int k = inputBufferSize; k > 0; k--) {
  //    if (inputBuffer[k] != COMMAND_SEPERATOR) {
  //      inputBuffer[k] = 0;
  //      inputBufferSize--;
  //    } else {
  //      break;
  //    }
  //  }
  //}
}

//----------------------------------------------------------------------------
// processCommand
// Switch off to the different specified functions
//----------------------------------------------------------------------------
void processCommand(char cmd, float arg3_f, char* args) {
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
      readPinDigital(getPin(args));
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
  }
}
//----------------------------------------------------------------------------
// runMotorCommand
// Figure out which wheels to turn on
//----------------------------------------------------------------------------
void runMotorCommand(float mspeed, char arg1, char arg2) {
  // We have 4 unique instruction 'positions'
  // This accounts for the 'a' parameter in each of the arguments
  boolean shouldSetMotors[] = {
    (arg1 == 'a' || arg1 == 'f'),
    (arg1 == 'a' || arg1 == 'b'),
    (arg2 == 'a' || arg2 == 'l'),
    (arg2 == 'a' || arg2 == 'r')
  };
  // This goes through each individual wheel, and makes sure that
  // the conditions for each wheel are met above to enable it.
  // Ex: If shouldSetMotors[0] (front) and shouldSetMotors[2] (left),
  //     then apply the speed to the pin MOTOR_FL_PWM_PIN
  for (int k = 0; k < 4; k++) {
    if (shouldSetMotors[k/2] && shouldSetMotors[2+k%2]) {
      writeMotorSpeed(mspeed, motorPins[k], motorPins[k+4]);
    }
  }
}
//----------------------------------------------------------------------------
// applyPinType
// Changes the pin type as needed.
// Provides an error if pin has already been decided on
//----------------------------------------------------------------------------
boolean applyPinType(byte pinID, byte type) {
  if (pinModes[pinID] == type + 1) {
    return true;
  }else if (pinModes[pinID] == 0 || !STRICT_PINS) {
    pinModes[pinID] = type + 1;
    pinMode(pinID, type);
    return true;
  }
  // Pin has already been set to a different type, and strict pins is enabled.
  // Raise an error, and don't allow the command to complete
  sprintf(errorReport,"Pin %i is %s", pinID, (pinModes[pinID] == OUTPUT + 1 ? "OUTPUT" : "INPUT"));
  Serial.println("-1");
  Serial.println(errorReport);
  return false;
}
//----------------------------------------------------------------------------
// getPin
// Gets the pin number from a set of characters.
// Works for pins labeled A and such as well.
//----------------------------------------------------------------------------
byte getPin(char pin[]) {
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
//----------------------------------------------------------------------------
void readPinDigital(byte pinID) {
  // Check if pin type set to read
  if (applyPinType(pinID,INPUT)) {
    // Read the pin
    Serial.println(digitalRead(pinID));
  }
}
//----------------------------------------------------------------------------
// writePinDigital
// Writes a value to a particular pin
//----------------------------------------------------------------------------
void writePinDigital(byte pinID, int output) {
  // Check if pin type set to write
  if (applyPinType(pinID,OUTPUT)) {
    // Write to the pin
    digitalWrite(pinID,output);
  }
}
//----------------------------------------------------------------------------
// readPinAnalog
// Reads the value of an analog pin, and prints a number from 0 to 1024.
//----------------------------------------------------------------------------
void readPinAnalog(byte pinID) {
  // Check if pin type set to read
  if (applyPinType(pinID,INPUT)) {
    // Read the pin
    Serial.println(analogRead(pinID));
  }
}
//----------------------------------------------------------------------------
// writePinAnalog
// Writes a value to a particular pin
// Note that this generates a square wave on that pin, with the output
// value being the duty cycle. If you want to specify frequency, use playTone
//----------------------------------------------------------------------------
void writePinAnalog(byte pinID, int output) {
  // When writing, you do not need to set the pin type to output,
  // However, strict mode should prevent us from doing anything
  if (applyPinType(pinID,OUTPUT)) {
    // Write to the pin
    analogWrite(pinID,output);
  }
}
//----------------------------------------------------------------------------
// playTone
// Plays a tone at a specified frequency on a pin
//----------------------------------------------------------------------------
void playTone(byte pinID, int freq) {
  // Stop the tone as needed
  if (tonePin != 0) {
    noTone(tonePin);
    tonePin = 0;
  }
  if (pinID == 0) {
    return;
  }
  // Start the new tone, with appropriate duration
  tonePin = pinID;
  if (toneDuration == 0) {
    tone(pinID,freq); 
  }else{
    tone(pinID,freq,toneDuration);
  }
}

//----------------------------------------------------------------------------
// battery voltage interrupt
// Triggered when the battery voltage dips below the voltage threshold
//----------------------------------------------------------------------------
ISR(ANALOG_COMP_vect) {
  battery_interrupt_fired = true;
  battery_low = true;
}
