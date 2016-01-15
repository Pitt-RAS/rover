
#define INPUT_BUFFER_SIZE = 200
#define PARAMETER_SEPERATOR = ','

#define MOTOR_FL_PWM_PIN = 0
#define MOTOR_FR_PWM_PIN = 0
#define MOTOR_BL_PWM_PIN = 0
#define MOTOR_BR_PWM_PIN = 0
#define MOTOR_FL_GPIO_PIN = 0
#define MOTOR_FR_GPIO_PIN = 0
#define MOTOR_BL_GPIO_PIN = 0
#define MOTOR_BR_GPIO_PIN = 0

char inputBuffer[INPUT_BUFFER_SIZE];
byte inputBufferSize = 0;

// Cut down on some conditionals by picking from an array
const byte[] motorPins = {
  MOTOR_FL_PWM_PIN, MOTOR_FR_PWM_PIN, MOTOR_BL_PWM_PIN, MOTOR_BR_PWM_PIN,
  MOTOR_FL_GPIO_PIN, MOTOR_FR_GPIO_PIN, MOTOR_BL_GPIO_PIN, MOTOR_BR_GPIO_PIN
};

//----------------------------------------------------------------------------
// setup
// Runs once when the program starts
//----------------------------------------------------------------------------
void setup(){
  Serial.begin(115200);
  Serial.setTimeout(10);
}

//----------------------------------------------------------------------------
// loop
// Main program loop
//----------------------------------------------------------------------------
void loop(){
  byte newChars = Serial.readBytesUntil('\n', inputBuffer + inputBufferSize, sizeof(inputBuffer) - inputBufferSize);
  inputBufferSize += newChars;
  if (inputBufferSize > 0 && inputBffer[inputBufferSize - 1] == '\n'){
    // Split the command, clear the buffer
    char[][] command = splitString(inputBuffer,',');
    inputBuffer = new char[INPUT_BUFFER_SIZE];
    // Process the command
    processCommand(command);
  }
}

//----------------------------------------------------------------------------
// processCommand
// Switch off to the different specified functions
//----------------------------------------------------------------------------
void processCommand(string[][] command){
  switch (command[0][0]){
    case 'm' : // Motor Command
      int motorPinSet = 2 * (command[2][0] == 'f' : 0 ? 1) + (command[2][1] == 'l' : 0 ? 1);
      writeMotorSpeed(atof(command[1]), motorPinSet, motorPinSet + 4);
      break;
  }
}

//----------------------------------------------------------------------------
// splitString
// Splits a string into seperate parameters in a 2D jagged array
//----------------------------------------------------------------------------
char[][] splitString(char[] string, char splitChar){
  int splitCount = 0;       // How many times the string has been split so far
  int splitPosOld = 0;      // The last character that the string was split at
  int stringSize = sizeof(string);
  // Memory is sparse; get an initial count
  toReturn = new char[getCharCount() + 1][];
  // Check every single character
  for (int k = 0; k < stringSize){
    if (string[k] == splitChar){
      // Split the string, trim the array, move on.
      toReturn[splitCount] = new char[k-splitPosOld];
      for (int j = splitPosOld; j < k; j++){
        toReturn[splitCount][j - splitPosOld] = string[j];
      }
    }
  }
  // Return our modified array
  return toReturn;
}
//----------------------------------------------------------------------------
// getCharCount
// Counts how many times a character appears
//----------------------------------------------------------------------------
int getCharCount(char[] string, char toFind){
  int stringSize = sizeof(string);
  int count = 0;
  for (int k = 0; k < stringSize; k++){
    if (string[k] == toFind){
      count++;
    }
  }
  return count;
}
//----------------------------------------------------------------------------
// writeMotorSpeed
// Enables the motor.
//----------------------------------------------------------------------------
void writeMotorSpeed(float speed, int pwm_pin, int gpio_pin) {
  if (speed >= 0) {
    digitalWrite(gpio_pin, 0);
    analogWrite(pwm_pin, speed * 255);
  } else {
    digitalWrite(gpio_pin, 1);
    analogWrite(pwm_pin, (1 + speed) * 255);
  }
}
