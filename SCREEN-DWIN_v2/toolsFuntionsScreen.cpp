#include "esp32-hal.h"
#include "BuzzerMelodies.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stdint.h>

#include "SerialPorts.h"
#include "DwinToolsInterface.h"
#include "ParseModule.h"
#include "toolsFunctionsScreen.h"
#include <Arduino.h>
#include "ConfigManager.h"

BuzzerMelodies buzzer;
ConfigManager config;

/// Variables tipo texto

static const uint16_t  GSNM_station = 0x2100;
static const uint16_t vehicleID = 0x2110;
static const uint16_t unitMeasure = 0x2008;
static const uint16_t banner = 0x2200;




///variablres numericas 
static const uint16_t  TotalGross = 0x100E;
static const uint16_t  heigtLevel = 0x1012;
static const uint16_t  flowRate  = 0x1000; 
static const uint16_t  preset = 0x1006;
static const uint16_t  counter = 0x100A;
static const uint16_t  volume = 0x1002;
static const uint16_t  inputName = 0x1100;
static const uint16_t  bannersplashscreen = 0x1300;
static const uint16_t  iconstatus = 0x1200;

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

// funciones de  uso general
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

int CharArrayToInt(const char value[]) {
  return atoi(value);
}

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


void GetHomePageNumber() {
  config.begin("LastRefuel"); // true = solo lectura

  // Leer de memoria la página de inicio
  int page = config.getInt("HomeScreen", 0);

  config.end();

  dwinChangePage_VP(page);
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
     delay(800);
   // dwinChangePage_VP(0);
   GetHomePageNumber();

};


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

 
    DebugSend("Unidad de medida:", event[5]);
    if(strcmp(event[5], "Galones:Gal") == 0){
   writeTextClean(unitMeasure, "Gal", 3);
    }else if(strcmp(event[5], "Litros:l") == 0){
writeTextClean(unitMeasure, "Lts", 3);
    }

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

  }else if(strcmp(answer,"CONFIGHOMENUMBER") == 0){

    config.begin("LastRefuel"); // true = solo lectura

    int HomeScreen = CharArrayToInt(event[5]);
    config.setInt("HomeScreen",HomeScreen);

    config.end();

    DebugSend("[TX] ScreenOption:", event[5]);

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


void DinaRefuel(char event[][50]) {
  const char* ns = "LastRefuel";
  const char* bannerMsg = "DESPACHO NO AUTORIZADO";

  // Validar contenido de vehicle
  const char* vehicle = event[3];

  if (vehicle == NULL || vehicle[0] == '\0') {
    DebugSend("[ERR]", "DinaRefuel: vehicle vacio");
    writeTextClean(vehicleID, "", 20);
    writeTextClean(banner, bannerMsg, 150);
    return;
  }

  config.begin(ns);

  // Mostrar en pantalla
  writeTextClean(vehicleID, vehicle, 20);
  writeTextClean(banner, bannerMsg, 150);

  // Guardar ultimo vehicle asociado
  config.setString("vehicle", String(vehicle));

  config.end();

  // Debug
  DebugSerial.println("===== DINA REFUEL =====");
  DebugSerial.print("Vehicle: ");
  DebugSerial.println(vehicle);
  DebugSend("[TX]", "DinaRefuel procesado");
}

void printRefuel(char event[][50]){

float counterprint = charToFloatCustom(event[4],2);
float flowRatePrint = charToFloatCustom(event[7],2);

writeU32(counter,counterprint * 100);
writeU16(flowRate,flowRatePrint * 100);


};

void Getfindespacho(char event[][50]) {

    // Validación primero
    if (event[1][0] == '\0' || event[4][0] == '\0' ||
        event[5][0] == '\0' || event[6][0] == '\0') {
        DebugSerial.println("[DEBUG] Getfindespacho: faltan campos en event[]");
        return;
    }

    char answer[200];

    snprintf(answer, sizeof(answer),
             "<RES|SCREEN|%s|FINDESPACHO|%s|%s|%s|OK>",
             event[1], event[4], event[5], event[6]);

    DebugSend("[TX]", answer);
    HostSerial.println(SendCommandCPU(answer));

    delay(400);

    float counterTotal = charToFloatCustom(event[4], 2);

    // Convertir de forma controlada
    uint32_t quantityValue = (uint32_t)(counterTotal * 100.0f + 0.5f);

    config.begin("LastRefuel");

    config.setInt("Quantity", quantityValue);

    writeU32(counter, quantityValue);
   

    config.end();

    DebugSerial.println("===== GET FINDESPACHO =====");
    DebugSerial.print("event[4]: ");
    DebugSerial.println(event[4]);
    DebugSerial.print("counterTotal: ");
    DebugSerial.println(counterTotal, 2);
    DebugSerial.print("quantityValue: ");
    DebugSerial.println(quantityValue);
    DebugSerial.print("guardado preferences: ");

    DebugSend("[TX]", totalCounter);
    HostSerial.println(SendCommandCPU(totalCounter));

     writeU16(iconstatus,0);
    delay(1000);
    writeU16(iconstatus,0);

};



void GetInputScreen() {
  config.begin("LastRefuel");

  bool enableKeypadTouch = config.getBool("enable_keypad_touch", true);

  config.end();

  if (enableKeypadTouch) {
    dwinChangePage_VP(1);  // Pantalla con keypad touch
  } else {
    dwinChangePage_VP(4);  // Pantalla alternativa
  }
};



void ShowInputScreen(char event[][50]) {

  char answer[80];
  const char* commandType = "SGET";

  // Detectar tipo de comando
  if (strcmp(event[3], "SGETA") == 0) {
    commandType = "SGETA";
  }

  // Guardar datos en screenflow
  strncpy(screenflow.inputNameKeypad, event[4], sizeof(screenflow.inputNameKeypad) - 1);
  screenflow.inputNameKeypad[sizeof(screenflow.inputNameKeypad) - 1] = '\0';

  strncpy(screenflow.decimalCursors, event[5], sizeof(screenflow.decimalCursors) - 1);
  screenflow.decimalCursors[sizeof(screenflow.decimalCursors) - 1] = '\0';

  strncpy(screenflow.device, event[1], sizeof(screenflow.device) - 1);
  screenflow.device[sizeof(screenflow.device) - 1] = '\0';

  // Construir respuesta
  snprintf(answer, sizeof(answer),
           "<RES|SCREEN|%s|%s|%s|%s|OK>",
           event[1],
           commandType,
           event[4],
           event[5]);

  // Flujo común
  dwinKeypadTouch(AUTO_CURSOR);
  DebugSend("[TX]", answer);
  HostSerial.println(SendCommandCPU(answer));

 // dwinChangePage_VP(1);
  GetInputScreen();
  delay(600);

  dwinKeypadTouch(AUTO_CURSOR);
  writeTextClean(inputName, event[4], 20);
};


void factory(char event[][50]){

  HostSerial.println(SendCommandCPU("<REQ|SCREEN|MAIN|FACTORY|34>"));
  DebugSend("[TX]","FACTORY");
};






void Printedvehicle(char event[][50]){
char answer[100];
config.begin("LastRefuel");

if(strcmp(event[3], "TAGID") == 0){
snprintf(answer,sizeof(answer) ,"<RES|SCREEN|%s|TAGID|%s>", event[1],event[4]);
config.setString("vehicle", String(event[4]));
DebugSend("[TX]",answer);
HostSerial.println(SendCommandCPU(answer));
writeTextClean(vehicleID,event[4],20);
writeU16(iconstatus,1);
buzzer.playOk();
return;
}

float presecounter = charToFloatCustom( event[7],2);
float initcouter = charToFloatCustom( event[5],2);
writeU32(preset,presecounter * 100);
writeU32(counter,initcouter * 100);
config.end();

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


void splashScreen(int value) {

    writeTextClean(bannersplashscreen,"......Cargando gstation 5.9 ....", 150);
    

};



void factoryScreenData(char event[][50]){
char answer[100];
config.begin("LastRefuel");

config.setString("vehicle", "NA");
config.setInt("Quantity", 0);

//settings screen Flow.
config.setBool("enable_keypad_touch",true);
config.setInt("HomeScreen",0);

snprintf(answer,sizeof(answer),"<RES|SCREEN|%S|FACTORY|OK>",event[1]);
DebugSend("[TX]", "Variables inicializadas");
HostSerial.println(SendCommandCPU(answer));

config.end();

};







// “Melodía” tipo XP usando solo ritmos
void dwinStartupXP() {
  // 💥 Subimos fuerza del buzzer para este sonido
  dwinSetBuzzDuty(0x0300);   // prueba también 0x0300 si quieres más fuerte

  // NOTA 1: tono medio, dur. ~120 ms
  dwinSetBuzzFreqDiv1(0x80); // más grave
  dwinSetBuzzTime(12);       // 12 * 10 ms = 120 ms
  dwinBuzzerBeep();
  delay(160);

  // NOTA 2: un poco más aguda, dur. ~160 ms
  dwinSetBuzzFreqDiv1(0x70);
  dwinSetBuzzTime(16);       // 160 ms
  dwinBuzzerBeep();
  delay(200);

  // NOTA 3: más aguda y larga, cierra como XP
  dwinSetBuzzFreqDiv1(0x60);
  dwinSetBuzzTime(24);       // 240 ms aprox
  dwinBuzzerBeep();

  // 🔉 opcional: regresar duty a un valor más suave para el resto de beeps
  dwinSetBuzzDuty(0x00F0);

};


void dwinErrorTone_Loud() {
  // 💥 Subimos la fuerza del buzzer
  dwinSetBuzzDuty(0x0280);     // prueba 0x0300 si quieres aún más fuerte

  // Si quieres respetar EXACTAMENTE la frecuencia actual,
  // puedes comentar esta línea. Aquí uso un valor medio "tipo sistema":
  dwinSetBuzzFreqDiv1(0x70);   // misma familia de frecuencia que el startup

  // BEEP 1: largo (~220 ms)
  dwinSetBuzzTime(22);         // 22 * 10ms = 220 ms
  dwinBuzzerBeep();
  delay(260);

  // BEEP 2: largo (~220 ms)
  dwinSetBuzzTime(22);
  dwinBuzzerBeep();
  delay(260);

  // BEEP 3: corto (~80 ms) como remate
  dwinSetBuzzTime(8);          // 80 ms
  dwinBuzzerBeep();

  // 🔉 Regresamos el duty a un valor más suave para el resto de sonidos
  dwinSetBuzzDuty(0x00F0);
};

void LoadLastRefuel() {

  const char* ns = "LastRefuel";
  const char* keyVehicle  = "vehicle";
  const char* keyQuantity = "Quantity";

  config.begin(ns);

  String vehicle = config.getString(keyVehicle, "");
  uint32_t quantityLast = config.getInt(keyQuantity, 0);

  // Limitar longitud por seguridad
  if (vehicle.length() >= 20) {
    vehicle = vehicle.substring(0, 19);
  }

  // Restaurar datos en pantalla
  writeTextClean(vehicleID, vehicle.c_str(), 20);
  writeU32(counter, quantityLast);

  // Debug
  DebugSerial.println("===== LOAD LAST REFUEL =====");
  DebugSerial.print("Vehicle: ");
  DebugSerial.println(vehicle.length() ? vehicle : "(vacio)");
  DebugSerial.print("Quantity: ");
  DebugSerial.println(quantityLast);

  if (vehicle.length() == 0 && quantityLast == 0) {
    DebugSend("[TX]", "No habia datos guardados de ultimo abastecimiento");
  } else {
    DebugSend("[TX]", "Restauracion completa");
  }
  config.end();
};

// Función auxiliar para enviar la respuesta
static void sendBuzzerResponse(const char* device, const char* command, const char* status) {
    char answer[80];  // un poco más grande por seguridad
    snprintf(answer, sizeof(answer),
             "<RES|SCREEN|%s|BUZZER|%s|%s>", device, command, status);

    HostSerial.println(SendCommandCPU(answer));
    DebugSend("[TX]", answer);
};

void ExecuteBuzzer(char event[][50]) {
    // Por legibilidad
    const char* device  = event[1];
    const char* command = event[4];

    // Opcional: validar que vengan datos
    if (device[0] == '\0' || command[0] == '\0') {
        DebugSend("[ERR]", "ExecuteBuzzer: device o command vacíos");
        return;
    }

    // Ejecutar acción según el comando
    if (strcmp(command, "STARTUP") == 0) {
        buzzer.playStartup();
        sendBuzzerResponse(device, command, "OK");

    } else if (strcmp(command, "ERROR") == 0) {
        buzzer.playError();
        sendBuzzerResponse(device, command, "OK");

    } else if (strcmp(command, "SUCCESS") == 0) {
        buzzer.playOk();
        sendBuzzerResponse(device, command, "OK");

    } else {
        // Manejo de comando desconocido
        DebugSend("[ERR]", "ExecuteBuzzer: comando de buzzer desconocido");
        sendBuzzerResponse(device, command, "UNKNOWN");
    }
};

const char* convertDateTimeToHex(const char* input) {

    static char output[30];

    int yy, mm, dd, hh, mi, ss;

    if (sscanf(input, "%d-%d-%d %d:%d:%d",
               &yy, &mm, &dd,
               &hh, &mi, &ss) != 6) {
        return "00-00-00 00:00:00";
    }

    snprintf(output, sizeof(output),
             "%02X%02X%02X%02X%02X%02X",
             yy, mm, dd, hh, mi, ss);

    return output;
};



// Convierte una cadena hexadecimal tipo "023BA897700D0A03" a bytes binarios
int hexStringToBytesGPS(const String& hexStr, byte* buffer, size_t bufferSize) {
  int len = hexStr.length() / 2;
  if (len > (int)bufferSize) len = bufferSize;

  for (int i = 0; i < len; i++) {
    String byteStr = hexStr.substring(i * 2, i * 2 + 2);
    buffer[i] = (byte) strtol(byteStr.c_str(), NULL, 16);
  }
  return len;
};



void setDateScreen(char event[][50]){

  char answer[50];
  int n = snprintf(answer, sizeof(answer), "%s%s", event[3], event[4]); 

  String Datasend = convertDateTimeToHex(event[5]);
  String codeAVL = "5AA50B82009C5AA5" + Datasend;
  DebugSerial.print("Trama HEX:");
  DebugSerial.println(codeAVL);
  byte buffer[32];

  int len = hexStringToBytesGPS(codeAVL, buffer, sizeof(buffer));

   DwinSerial.write(buffer,len);


DebugSend("[TX] SETDATE", "HORAS SET OK");
};

void setEnableSCreen(char event[][50]){

//char answer[100];
config.begin("LastRefuel");

if(strcmp(event[4],"SHOW") == 0){
config.setBool("enable_keypad_touch",true);
DebugSend("[TX] enable Touch", "TRUE");
}
if(strcmp(event[4], "HIDE") == 0 ){

config.setBool("enable_keypad_touch",false);
DebugSend("[TX] enable Touch", "FALSE");
}

config.end();


};

