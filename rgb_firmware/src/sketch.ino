//============================================================================
// RGB LED firmware Code for Pitt RAS Rover
// 2016
//============================================================================
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN    2    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 144

// How many characters should be back-logged for the input buffer
#define INPUT_BUFFER_SIZE 20
// The character that separates individual commands
#define COMMAND_BEGINNER ':'
// The character that separates individual commands
#define COMMAND_ENDER ';'
// The number of bytes in a command not including start character
#define COMMAND_SIZE 8
// The number of byte size arguments
#define ARGUMENTS 3

#define MIN_PATTERN_DELAY 5

enum Patterns 
        { 
          OFF = 'o',
          CHASE = 'c', 
          RAINBOW = 'r', 
          GLOW = 'g',
          SOLID = 's'
        };    

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

Patterns pattern = OFF;
int pattern_delay = 50;
char pattern_argument[3] = {0, 0, 0};

// Buffer for serial input
char input_buffer[INPUT_BUFFER_SIZE];

void setup()
{
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  //Check for new incoming command
  if((char)Serial.read() == COMMAND_BEGINNER) //The beginning character
  {
    //Read up to the end of the command
    Serial.readBytesUntil(COMMAND_ENDER, input_buffer, INPUT_BUFFER_SIZE);
    
    //Parse the command
    float arg;
    memcpy(&arg, &input_buffer[ARGUMENTS + 1], 4);
    pattern_delay = (int) arg;

    if(pattern_delay < MIN_PATTERN_DELAY)
    {
      pattern_delay = MIN_PATTERN_DELAY;
    }

    pattern = (Patterns)input_buffer[0];

    for(int i = 0; i < ARGUMENTS; i++){
      pattern_argument[i] = input_buffer[i + 1];
    }

    //Immediately update in case the last command had a really long delay
    runPattern();
  }


  //Update lights periodically
  static long last_t = millis();
  if(millis() - last_t > pattern_delay){
    last_t = millis();
    runPattern();
  }

}


void runPattern(){
  switch(pattern) 
  {
    case OFF :
      pattern_off();
      break;
    case SOLID :
      pattern_solid();
      break;
    case RAINBOW :
      pattern_rainbow();
      break;
  }
}

void pattern_rainbow()
{
  uint16_t i;
  static uint16_t j = 0;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+j) & 255));
  }
  strip.show();

  j++;
  if(j > 255)
  {
    j = 0;
  }
}
  

void pattern_off()
{
  uint32_t c = strip.Color(0, 0, 0);
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void pattern_solid()
{
  uint32_t c = strip.Color(pattern_argument[0], pattern_argument[1], pattern_argument[2]);
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}


// Take from Adafruits NeoPixel Library
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
