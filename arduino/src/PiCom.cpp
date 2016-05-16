#include "PiCom.h"
#include <Arduino.h>

char input_buffer[kPiComBufferSize];

void PiComInit()
{
    Serial.begin(kPiComBaudRate);
}

boolean PiComGetCommand(char& command, float& number, char* arguments)
{

  long time = millis();
  while(Serial.available() > 0 && millis() - time < kPiComCommandTimeout)
  {

    char character = Serial.read();

    //Check for new incoming command
    if(character == ':') //The beginning character
    {
      Serial.println("go");

      int read = Serial.readBytesUntil(kPiComCommandEnd, input_buffer, kPiComBufferSize);

      //Success!
      if(read == kPiCommandSize){
        command = input_buffer[0];
        memcpy(arguments, &input_buffer[1], kPiComArguments);
        memcpy(&number, &input_buffer[kPiComArguments + 1], 4);
        Serial.println("ok");
        return true;
      }
      else
      {
        Serial.println("no");
        time = millis(); //Reset the timeout so we don't timeout in the middle of this
      }
      
    }
  }

  //We timed out while being streamed bad data
  return false;
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