#include <stdio.h>

#include <stdint.h>

#include "SerialPorts.h"
#include "DwinToolsInterface.h"
#include "ParseModule.h"


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


//variables nivel texto




int GetLengData(char event[][30]) {
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
}



void GoHomePage(char event[][30]) {

    char answer[64];

    if (event[2][0] == '\0') {
        DebugSerial.println("[DEBUG] event[2] vacío");
        return;
    }

    snprintf(answer, sizeof(answer),
             "<RES|SCREEN|%s|STGS|OK>", event[2]);

    DebugSend("Pagina Principal", answer);   // 👈 DEBUG USB
    HostSerial.println(SendCommandCPU(answer)); // UART real

    dwinChangePage_VP(0);
};




