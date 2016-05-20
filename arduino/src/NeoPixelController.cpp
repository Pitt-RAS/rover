#include "NeoPixelController.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

NeoPixelController::NeoPixelController(int leds, int pin)
:m_PixelStrip(leds, pin, NEO_GRB + NEO_KHZ800)
{  
  m_CurrentPattern = PATTERN_OFF;
  m_UpdateImmediately = false;
  m_UpdatePeriod = 1000;
  
  m_TimeSinceUpdate = millis();
  m_CurrentColor = m_PixelStrip.Color(0, 0, 0);
  
  m_PatternDirection = true;
  
  kNumLeds = leds;
}

void NeoPixelController::begin(void){
  m_PixelStrip.begin();
  m_PixelStrip.show();
}

void NeoPixelController::update(){
  if((millis() - m_TimeSinceUpdate > m_UpdatePeriod) || m_UpdateImmediately)
  {
    m_UpdateImmediately = false;
    m_TimeSinceUpdate = millis();
    switch(m_CurrentPattern) 
    {
      case PATTERN_OFF :
        pattern_off();
        break;
      case PATTERN_SOLID :
        pattern_solid();
        break;
      case PATTERN_RAINBOW :
        pattern_rainbow();
        break;
      case PATTERN_CHASER :
        pattern_chaser();
        break;
    }
  }
}

void NeoPixelController::setPattern(const Pattern pattern, const int& period, const int& r, const int& b, const int& g)
{
  m_CurrentColor = m_PixelStrip.Color(r, b, g);
  m_CurrentPattern = pattern;
  
  m_UpdatePeriod = abs(period);
  
  if(m_UpdatePeriod < kMinPatternDelay)
  {
    m_UpdatePeriod = kMinPatternDelay;
  }
  
  m_PatternDirection = period >= 0;
  
  m_UpdateImmediately = true;

}

void NeoPixelController::pattern_rainbow()
{
  uint16_t i;
  static uint16_t j = 0;

  for(i=0; i<m_PixelStrip.numPixels(); i++) {
    m_PixelStrip.setPixelColor(i, Wheel((i+j) & 255));
  }
  
  j = (j+1)%255;
  m_PixelStrip.show();
}
  

void NeoPixelController::pattern_off()
{
  uint32_t c = m_PixelStrip.Color(0, 0, 0);
  for(uint16_t i=0; i<m_PixelStrip.numPixels(); i++) {
    m_PixelStrip.setPixelColor(i, c);
  }
  m_PixelStrip.show();
}

void NeoPixelController::pattern_solid()
{
  for(uint16_t i=0; i<m_PixelStrip.numPixels(); i++) {
    m_PixelStrip.setPixelColor(i, m_CurrentColor);
  }
  m_PixelStrip.show();
}

void NeoPixelController::pattern_chaser()
{
  
  static char x = 0;
  m_PixelStrip.clear();
  
  //Yay magic numbers, these are the locations for the start and ends of the chasers on the robot
  const char startr = 97;
  const char startl = 18;
  const char end = 30;

  
  if(m_PatternDirection){
    // forward
    m_PixelStrip.setPixelColor(startr - end + x, m_CurrentColor);
    m_PixelStrip.setPixelColor(startl + end - x, m_CurrentColor);
  }
  else{
    // backward
    m_PixelStrip.setPixelColor(startr - x, m_CurrentColor);
    m_PixelStrip.setPixelColor(startl + x, m_CurrentColor);
  }
  m_PixelStrip.show();
  
  x++;
  if(x > end)
  {
    x = 0;
  }
  
}


// Take from Adafruits NeoPixel Library
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t NeoPixelController::Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return m_PixelStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return m_PixelStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return m_PixelStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
