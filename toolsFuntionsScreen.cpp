#include "esp32-hal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stdint.h>

#include "SerialPorts.h"
#include "DwinToolsInterface.h"
#include "ParseModule.h"
#include "toolsFunctionsScreen.h"
#include <Arduino.h>
/// Variables tipo texto

static const uint16_t  GSNM_station = 0x2100;
static const uint16_t vehicleID = 0x2110;
static const uint16_t unitMeasure = 0x2120;
static const uint16_t banner = 0x2200;




///variablres numericas 
static const uint16_t  TotalGross = 0x100E;
static const uint16_t  heigtLevel = 0x1012;
static const uint16_t  flowRate  = 0x1000; 
static const uint16_t  preset = 0x1006;
static const uint16_t counter = 0x100A;
static const uint16_t  volume = 0x1002;
static const uint16_t   inputName = 0x1100;

//variables nivel texto
static  char *totalCounter = "<REQ|SCREEN|MAIN|CONFIG|TGAL>";
static  char *IDPCB        = "<REQ|SCREEN|MAIN|CONFIG|GSID>";
static  char *name_station = "<REQ|SCREEN|MAIN|CONFIG|GSNM>";
static  char *unit_measure = "<REQ|SCREEN|MAIN|CONFIG|GSUM>";

// estados de pantalla
ScreenFlow screenflow = {
  .inputNameKeypad = "",
  .decimalCursors = "",
  .device = ""
}; 

int GetLengData(char event[][50]) {
    int i = 0;
    int len = 0;

    // Buscamos la primera cadena no vacía
    while (event[i][0] != '\0') {

        len = 0;

        // Contamos caracteres de esa cadena
        while (event[i][len] != '\0') {
            len++;
        }

        return len;   // regresamos la longitud
    }

    return 0; // no hay cadenas válidas
};


void DebugSend(const char *label, const char *data) {
    DebugSerial.print("[DEBUG] ");
    DebugSerial.print(label);
    DebugSerial.print(" -> ");
    DebugSerial.println(data);
};


float charToFloatCustom(const char *str, uint8_t decimals) {
    float result = 0.0f;
    float sign = 1.0f;
    uint32_t integerPart = 0;
    uint32_t decimalPart = 0;
    uint8_t decimalCount = 0;

    // Validación básica
    if (str == NULL || *str == '\0') {
        return 0.0f;
    }

    // Signo negativo
    if (*str == '-') {
        sign = -1.0f;
        str++;
    }

    // Parte entera
    while (*str >= '0' && *str <= '9') {
        integerPart = integerPart * 10 + (*str - '0');
        str++;
    }

    // Parte decimal
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9' && decimalCount < decimals) {
            decimalPart = decimalPart * 10 + (*str - '0');
            decimalCount++;
            str++;
        }
    }

    // Convertimos a float
    result = (float)integerPart;

    float divisor = 1.0f;
    for (uint8_t i = 0; i < decimalCount; i++) {
        divisor *= 10.0f;
    }

    result += (float)decimalPart / divisor;

    return result * sign;
};



void GoHomePage(char event[][50]) {

    char answer[64];

    // Si vas a usar event[1], valida event[1]
    if (event[1][0] == '\0') {
        DebugSerial.println("[DEBUG] event[1] vacío");
        return;
    }

    snprintf(answer, sizeof(answer),
             "<RES|SCREEN|%s|STGS|OK>", event[1]);

    DebugSend("Pagina Principal", answer);           // Debug USB
    HostSerial.println(SendCommandCPU(answer));      // UART real

    delay(200);
    HostSerial.println(SendCommandCPU(name_station));

    delay(400);
    HostSerial.println(SendCommandCPU(unit_measure));

    delay(600);
    HostSerial.println(SendCommandCPU(totalCounter));

    dwinChangePage_VP(0);
}


void getConfigData(char event[][50]) {

  char answer[50];
  int n = snprintf(answer, sizeof(answer), "%s%s", event[3], event[4]);

  // Si se truncó, mejor no comparar (evita matches raros)

  DebugSend("configuracion:", answer);
  if (n < 0 || n >= (int)sizeof(answer)) {
    DebugSend("Error: comando truncado", answer);
    return;
  }

  if (strcmp(answer, "CONFIGGSUM") == 0) {

    writeTextClean(unitMeasure, event[5], 10);
    DebugSend("Unidad de medida:", event[5]);

  } else if (strcmp(answer, "CONFIGGSNM") == 0) {

    writeTextClean(GSNM_station, event[5], 30);
    DebugSend("Estacion:", event[5]);

  } 
  else if (strcmp(answer, "CONFIGTGAL") == 0) {

    char *endPtr = nullptr;
    float totalizer = strtof(event[5], &endPtr);

    // Validación: no convirtió nada
    if (endPtr == event[5]) {
      DebugSend("Error totalizador (no numerico):", event[5]);
      return;
    }

    // Permitir basura típica final: espacios / \r / \n
    while (*endPtr == ' ' || *endPtr == '\r' || *endPtr == '\n' || *endPtr == '\t') endPtr++;
    if (*endPtr != '\0') {
      DebugSend("Error totalizador (caracteres extra):", event[5]);
      return;
    }

    if (totalizer < 0.0f) {
      DebugSend("Error totalizador (negativo):", event[5]);
      return;
    }

    // Escala x10 con redondeo
    float scaledF = totalizer * 100.0f;
    if (scaledF > 4294967295.0f) { // max uint32
      DebugSend("Error totalizador (overflow):", event[5]);
      return;
    }

    uint32_t scaled = (uint32_t)(scaledF + 0.5f);
    writeU32(TotalGross, scaled);

    DebugSend("totalizador:", event[5]);

  }
};


void Showbanner(char event[][50]) {

    // Si es CLEAR, limpiamos y salimos (sin enviar trama si no la necesitas)
    if (strcmp(event[4], "CLEAR") == 0) {
        DebugSend("Banner: CLEAR", "...");
        writeTextClean(banner, "...", 150);
        return;
    }

    // Buffer más grande para evitar truncamiento
    char answer[300];

    // event[5] puede venir vacío; si lo es, mandamos algo seguro
    const char *extra = (event[5][0] != '\0') ? event[5] : "OK";

    snprintf(answer, sizeof(answer),
             "<RES|SCREEN|%s|BANNER|%s|%s|OK>", event[1], event[4], event[5]);

    // Debug: ver exactamente qué se enviará y qué se mostrará
   // DebugSend("BannerText", event[4]);
    DebugSerial.println(event[4]);
    DebugSend("TX Frame", answer);

    // Envío real (si ya tienes sendToHost, úsalo)
    // sendToHost("BANNER", answer);
    HostSerial.println(SendCommandCPU(answer));

    // Mostrar texto en pantalla/local
    writeTextClean(banner, event[4], 150);
};


void ResponsePing(char event[][50]){

  char answer[50];

  snprintf(answer,sizeof(answer), "<RES|SCREEN|%s|PING|OK>", event[1]);

  DebugSend("[TX]",answer);
  HostSerial.println(SendCommandCPU(answer));

};


void DinaRefuel(char event[][50]){
// char answer[30];

writeTextClean(vehicleID,event[3],20);
writeTextClean(banner,"DESPACHO NO AUTORIZADO",150);
};

void printRefuel(char event[][50]){

float counterprint = charToFloatCustom(event[4],2);
float flowRatePrint = charToFloatCustom(event[7],2);

writeU32(counter,counterprint * 100);
writeU16(flowRate,flowRatePrint * 100);


};


void Getfindespacho(char event[][50]) {

    // Validación mínima
    if (event[1][0] == '\0' || event[4][0] == '\0' || event[5][0] == '\0' || event[6][0] == '\0') {
        DebugSerial.println("[DEBUG] Getfindespacho: faltan campos en event[]");
        return;
    }

    char answer[200];  // buffer suficiente

    snprintf(answer, sizeof(answer),
             "<RES|SCREEN|%s|FINDESPACHO|%s|%s|%s|OK>",
             event[1], event[4], event[5], event[6]);

    // Debug de lo que se envía
    DebugSend("[TX]", answer);

    // Envío real
    HostSerial.println(SendCommandCPU(answer));

    delay(400);
    float counterTotal = charToFloatCustom(event[4], 2);
    writeU32(counter,counterTotal *100);
    // También loguea esta trama si quieres verla en debug
    DebugSend("[TX]", totalCounter);
    HostSerial.println(SendCommandCPU(totalCounter));




};

void ShowInputScreen(char event[][50]){

char answer[50];

snprintf(answer,sizeof(answer),"<RES|SCREEN|%s|SGET|%s|%s|OK>", event[1],event[4],event[5]);

strncpy(screenflow.inputNameKeypad,event[4],sizeof(screenflow.inputNameKeypad) - 1 );
strncpy(screenflow.decimalCursors,event[5],sizeof(screenflow.decimalCursors) - 1 );
strncpy(screenflow.device,event[1],sizeof(screenflow.device) - 1 );

DebugSend("[TX]",answer);
HostSerial.println(SendCommandCPU(answer));
dwinChangePage_VP(1);

writeTextClean(inputName,event[4],20);


};


void Printedvehicle(char event[][50]){
char answer[100];


if(strcmp(event[3], "TAGID") == 0){
snprintf(answer,sizeof(answer) ,"<RES|SCREEN|%s|TAGID|%s>", event[1],event[4]);
DebugSend("[TX]",answer);
HostSerial.println(SendCommandCPU(answer));
writeTextClean(vehicleID,event[4],20);
return;
}

float presecounter = charToFloatCustom( event[7],2);
float initcouter = charToFloatCustom( event[5],2);
writeU32(preset,presecounter * 100);
writeU32(counter,initcouter * 100);




};



void QRscreen(char event[][50]){

  char answer[100];

  snprintf(answer, sizeof(answer),"<RES|SCREEN|%s|QR|1|1|%s|OK>", event[1],event[6]);

      HostSerial.println(SendCommandCPU(answer));

      char inputdata[100];
      snprintf(inputdata,sizeof(inputdata),"<RES|SCREEN|%s|SGET|%s|OK|%s>", screenflow.device,screenflow.inputNameKeypad,event[6]);            
      DebugSend("[TX]",inputdata);

      delay(300);
      HostSerial.println(SendCommandCPU(inputdata));

};




