#ifndef SERIAL_PORTS_H
#define SERIAL_PORTS_H

#include <Arduino.h>

// ====== Puertos seriales disponibles ======
// Se declaran como extern para poder usarlos en todo el proyecto

extern HardwareSerial DebugSerial;
extern HardwareSerial DwinSerial;
extern HardwareSerial HostSerial;

// ====== Inicialización central ======
namespace SerialPorts {
  void begin();
}

#endif
