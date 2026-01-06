//incluyo librerias  de parseo o manejo de eventos, libreria para controlar los eventos de la pantalla  y cntrol de puestros seriales
#include "ParseModule.h"
#include "SerialPorts.h"
#include "DwinToolsInterface.h"


// variables globales de uso;
String InputSerial = "";
char messageConvert[50];


char *initScreen = "<REQ|SCREEN|MAIN|MESSAGETO|SCREEN>";


//lista de funciones 
typedef void (*FunctionType)(char[][30]);



//Se define la estructura para el uso de las funciones 
typedef struct {
const  char* name;
FunctionType function;

} Command;



// lista de comandos activos

Command commands[] = {
};



// Buscar y ejecutar el comando
const char* executeCommand(const char* commandName, char event[][30]) {
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strcmp(commands[i].name, commandName) == 0) {
            commands[i].function(event); // Ejecutar la función
            return "Comando ejecutado correctamente";
        }
    }
    return "Comando no agregado";
};


//Enviar un comando



void setup() {

// inicio el puerto seriales.
SerialPorts::begin();

DebugSerial.println("DEBUG listo usb");
delay(2000);
HostSerial.println(SendCommandCPU(initScreen));



}

void loop() {
  // put your main code here, to run repeatedly:
//logica de funcionamiento

if( HostSerial.available()  > 0 || DebugSerial.available() ){

  
        // Verificar de dónde viene la entrada
        if (DebugSerial.available() > 0) {
            InputSerial = DebugSerial.readString();  // Leer del puerto USB
        } 
        else if (HostSerial.available() > 0) {
            InputSerial = HostSerial.readString();    // Leer del puerto RASP
        }




  
  DebugSerial.println(InputSerial);

  InputSerial.toCharArray(messageConvert,sizeof(messageConvert));
  const char* Method;

  char tokens[10][20];
  int N_messages = ParseMessage(messageConvert,tokens);

  Method = WordKey(N_messages,tokens);
  DebugSerial.println(Method);  
//  executeCommand(Method,tokens);

};




}
 