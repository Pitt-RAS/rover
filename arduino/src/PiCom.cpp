#include "PiCom.h"
#include <Arduino.h>

char input_buffer[kPiComBufferSize];

void PiComInit()
{
    Serial.begin(kPiComBaudRate);
}

boolean PiComGetData(char* data)
{

  long time = millis();
  while(Serial.available() > 0 && millis() - time < kPiComCommandTimeout)
  {

    char character = Serial.read();

    //Check for new incoming command
    if(character == ':') //The beginning character
    {
      Serial.println("go");
      
      char packetSize = PiComReadByte(); //First byte after handshake is the packet size including command
            
      int bytes = Serial.readBytes(data, packetSize);

      if(bytes == packetSize)
      {
        Serial.println("ok");
        return true;
      }
      
      Serial.println("no");
    }
  }

  //We timed out while being streamed bad data
  return false;
}

char PiComReadByte()
{
  while(Serial.available() == 0);
  
  return Serial.read();
}

void PiComSendData(char data)
{
  Serial.write(data);
}

void PiComSendData(uint16_t data)
{
  Serial.write((uint8_t*)&data, 2);
}

void PiComSendData(float data)
{
  Serial.write((uint8_t*)&data, 4);
}