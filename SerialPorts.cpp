#include "SerialPorts.h"

// ====== Instancias únicas de UART ======
HardwareSerial DebugSerial(0);  // USB
HardwareSerial DwinSerial(1);   // UART1
HardwareSerial HostSerial(2);   // UART2 -- este  es el puerto serial que va hacia  la main.

// ====== Configuración de pines y velocidad ======
static const int DWIN_RX  = 16;
static const int DWIN_TX  = 17;

static const int HOST_RX  = 4;
static const int HOST_TX  = 2;

static const int KEYPAD_RX = 25;
static const int KEYPAD_TX = 26;




static const uint32_t BAUD_DEBUG = 9600;
static const uint32_t BAUD_DWIN  = 115200;
static const uint32_t BAUD_MAIN  = 9600;
static const uint32_t KEYPAD_ACC = 9600;

SoftwareSerial HardwareKeypad(KEYPAD_RX,KEYPAD_TX);

// ====== Inicialización única ======
void SerialPorts::begin() {

  DebugSerial.begin(BAUD_DEBUG);
  delay(100);

  DwinSerial.begin(
    BAUD_DWIN,
    SERIAL_8N1,
    DWIN_RX,
    DWIN_TX
  );

  HostSerial.begin(
    BAUD_MAIN,
    SERIAL_8N1,
    HOST_RX,
    HOST_TX
  );

  HardwareKeypad.begin(KEYPAD_ACC);

  DebugSerial.println("✅ SerialPorts inicializados");
}
