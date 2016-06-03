//============================================================================
// Neopixel control Library for Pitt RAS Rover
// 2016
//============================================================================

#ifndef NEOPIXELCONTROL_h
#define NEOPIXELCONTROL_h

#include <Adafruit_NeoPixel.h>

class NeoPixelController
{
  public:
  
    enum Pattern
    { 
      PATTERN_OFF,
      PATTERN_CHASER, 
      PATTERN_RAINBOW, 
      PATTERN_GLOW,
      PATTERN_SOLID,
    };
    
    NeoPixelController(int leds, int pin);
    
    void begin(void);
    
    void update(void);
    
    void setPattern(const Pattern pattern, const int& period);
    
    void setColor(const int& r, const int& g, const int& b);
    
    void setRainbow(const int& period);
    
    uint32_t rainbowColor();

    uint32_t Wheel(byte WheelPos);
    
  private:
  
    static const int kMinPatternDelay = 5; //in mS
    static const int kMaxOutputPower = 400; //r + g + b
    
    int kNumLeds;
  
    Adafruit_NeoPixel m_PixelStrip;

    Pattern m_CurrentPattern;
    
    long m_TimeSinceUpdate;
    
    int m_UpdatePeriod;
    
    long m_RainbowTimeSinceUpdate;
    
    int m_RainbowUpdatePeriod;
    
    uint32_t m_CurrentColor;
    
    bool rainbow;
    
    boolean m_UpdateImmediately;
    
    boolean m_PatternDirection;
    
    //Functions
    void pattern_rainbow();

    void pattern_off();

    void pattern_solid();
    
    void pattern_chaser();
   
};

#endif