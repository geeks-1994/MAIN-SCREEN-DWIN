#include "Print.h"
#include <cstdint>
#include "WString.h"
#include "ParseModule.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <climits>
char buffer[100];

//esta funcion preenderisa la entrada serial  quitando los caracteres que se le mencione
const char* OutputMessage( const char* Input, const char *Delimitador){

strncpy(buffer,Input,sizeof(buffer)-1);
buffer[sizeof(buffer) - 1]= '\0';

char* result = strtok(buffer,Delimitador);

return result != nullptr ? result : "";
  
};


//funcion de parseo de mensajes
int ParseMessage(char *cadena, char tokens[][50]) {

    if (!cadena) return 0;

    const char *firstInput  = OutputMessage(cadena, "<");
    if (!firstInput) return 0;

    const char *secondInput = OutputMessage(firstInput, ">");
    if (!secondInput) return 0;

    // Buffer más grande para evitar overflow
    char messageInput[256];

    // Copia segura
    strncpy(messageInput, secondInput, sizeof(messageInput) - 1);
    messageInput[sizeof(messageInput) - 1] = '\0';

    int count = 0;
    char *answer = strtok(messageInput, "|");

    while (answer != NULL && count < 10) {

        // Copia segura del token completo (hasta 49 chars)
        strncpy(tokens[count], answer, 50 - 1);
        tokens[count][50 - 1] = '\0';

        count++;
        answer = strtok(NULL, "|");
    }

    return count;
};




const  char*  WordKey(int Tokens,char parseMessage[][50])
{
static char  endWord[100];
static char secretWord[100];
static char typeMessage[100];

if(Tokens > 6){

  snprintf(endWord,sizeof(endWord),"%s%s%s%s",parseMessage[0] ,parseMessage[1],parseMessage[2],parseMessage[3]);

// esta condicion solo se construye si token es mayor a 6
if(strcmp(endWord,"REQMAINRASPFIN_DESPACHO") == 0){

  strncpy(secretWord,endWord,sizeof(secretWord));

} else if (strcmp(endWord, "RESSCREENRASPSGET") == 0) {
            snprintf(secretWord, sizeof(secretWord), "%s%s%s%s",
                     parseMessage[0], parseMessage[1], parseMessage[3], parseMessage[5]);
        } else if (strcmp(endWord, "REQRFIDRASPTAG") == 0) {
            snprintf(secretWord, sizeof(secretWord), "%s%s%s%s%s",
                     parseMessage[0], parseMessage[1], parseMessage[2], parseMessage[3], parseMessage[4]);
        } else if (strcmp(endWord, "RESMAINRASPFLS") == 0) {
            snprintf(secretWord, sizeof(secretWord), "%s%s%s%s",
                     parseMessage[0], parseMessage[1], parseMessage[3], parseMessage[4]);
        } else {
            snprintf(secretWord, sizeof(secretWord), "%s%s%s",
                     parseMessage[0], parseMessage[1], parseMessage[3]);
        }
} else {

    snprintf(endWord,sizeof(endWord),"%s%s%s%s",parseMessage[0] ,parseMessage[1],parseMessage[2],parseMessage[3]);

    if(strcmp(endWord,"RESSCREENRASPSGET") == 0){
                 snprintf(secretWord, sizeof(secretWord), "%s%s%s%s%s",
                     parseMessage[0], parseMessage[1], parseMessage[2], parseMessage[3],parseMessage[5]);
    }else {
        strncpy(secretWord,endWord,sizeof(secretWord));
    }



}


return secretWord;

}


String generateChecksum(String cadenaTest) {
    int checksum = 0;

    // Calcular el XOR de los caracteres en la cadena
    for (size_t i = 0; i < cadenaTest.length(); i++) {
        checksum ^= cadenaTest[i];
    }

    // Convertir el checksum a hexadecimal
    String hexString = String(checksum, HEX);

    // Asegurarse de que tenga al menos 2 caracteres, agregando ceros al inicio si es necesario
    if (hexString.length() < 2) {
        hexString = "0" + hexString;
    }

    // Crear la cadena con el formato <cadenaTest|crc>
    String dataChecksum = "<" + cadenaTest + "|" + hexString + ">";


    return dataChecksum;
}



String SendCommandCPU(char *input){


const char* FirstWord = OutputMessage(input,">");
const char* secondWord = OutputMessage(FirstWord, "<");


String comando = String(secondWord);

String CRC = generateChecksum(comando);
 Serial.println(CRC);
return CRC;
};


String ToHex(uint32_t value){

String hexValue = String(value,HEX);
hexValue.toUpperCase();

return hexValue;

}

uint64_t charToUint64(const char *cadena, int base) {
    unsigned long long val = strtoull(cadena, nullptr, base);

    if (val == ULLONG_MAX) {
        Serial.println("⚠️ Overflow detectado: valor fuera de rango uint64_t");
    }

    return static_cast<uint64_t>(val);
}



// Convierte una cadena hexadecimal tipo "023BA897700D0A03" a bytes binarios
int hexStringToBytes(const String& hexStr, byte* buffer, size_t bufferSize) {
  int len = hexStr.length() / 2;
  if (len > (int)bufferSize) len = bufferSize;

  for (int i = 0; i < len; i++) {
    String byteStr = hexStr.substring(i * 2, i * 2 + 2);
    buffer[i] = (byte) strtol(byteStr.c_str(), NULL, 16);
  }
  return len;
};





String toHexString(const char* input) {
  String hexString = "";
  while (*input) {
    char buf[3];
    sprintf(buf, "%02X", *input); // convierte cada carácter a dos dígitos hex
    hexString += buf;
    input++;
  }
  return hexString;
};


