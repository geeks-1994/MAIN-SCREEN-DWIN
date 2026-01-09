#include "WString.h"
#include <string>
#include <Arduino.h>
#include <climits>
#ifndef   ParseDataSerial_H
#define  ParseDataSerial_H

#include <string.h>

int ParseMessage(char *cadena,char tokens[][50] );
String  generateChecksum(String cadenaTest);
const  char*  WordKey(int Tokens,char parseMessage[][50]);
String SendCommandCPU(char *input);
String ToHex(uint32_t value);
uint64_t charToUint64(const char *cadena, int base);
int hexStringToBytes(const String& hexStr, byte* buffer, size_t bufferSize);
String toHexString(const char* input); 


#endif