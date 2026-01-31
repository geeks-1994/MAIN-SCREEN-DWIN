#ifndef SERIAL_PORTS_H
#define SERIAL_PORTS_H

#include <Arduino.h>
#include <SoftwareSerial.h>

// ====== Puertos seriales disponibles ======
// Se declaran como extern para poder usarlos en todo el proyecto

extern HardwareSerial DebugSerial;
extern HardwareSerial DwinSerial;
extern HardwareSerial HostSerial;
extern SoftwareSerial HardwareKeypad;

// ====== Inicialización central ======
namespace SerialPorts {
  void begin();
}

#endif
