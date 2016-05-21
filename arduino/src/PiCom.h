//============================================================================
// PiCom Library for Pitt RAS Rover
// 2016
//============================================================================
#ifndef PI_COM_h
#define PI_COM_h

#include <Arduino.h>

// How many characters should be back-logged for the input buffer
#define kPiComBufferSize 30 //Should be at least twice the command size plus some extra bytes for the seperators

// Serial Baudrate
static const long kPiComBaudRate = 115200;

// The amount of arguments in a command
static const int kPiComArguments = 4;

// The character that begins individual commands
static const char kPiComCommandBegin = ':';

// The character that separates individual commands
static const char kPiComCommandEnd = ';';

// If we are being flooded with serial data we don't want to halt everything
// Timeout on searching for a command to prevent never servicing our other tasks
static const int kPiComCommandTimeout = 500;

// Command Size in bytes
static const int kPiCommandSize = 255; //1 command, 1 four byte float, arguments

// Buffer for serial input
extern char input_buffer[kPiComBufferSize];

// Initialize the library
void PiComInit(void);

// Wait for a byte
char PiComReadByte(void);

// Get a new command
boolean PiComGetData(char* data);

// Send some data in response
void PiComSendData(char data);
void PiComSendData(uint16_t data);
void PiComSendData(float data);

// Not implemented
void PiComConnect(char* id);

#endif