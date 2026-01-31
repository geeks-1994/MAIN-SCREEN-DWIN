#include "keypadhandler.h"
#include "SerialPorts.h"      // para DebugSerial
#include "DwinToolsInterface.h" // donde está dwinKeypadTouch + KP_*

// ===== Parseo del byte recibido =====
KeypadAction parseKeypad(uint8_t code) {

  switch (code) {

    // ===== NUMEROS =====
    case 0x30: return KEY_0; // '0'
    case 0x31: return KEY_1; // '1'
    case 0x32: return KEY_2; // '2'
    case 0x33: return KEY_3; // '3'
    case 0x34: return KEY_4; // '4'
    case 0x35: return KEY_5; // '5'
    case 0x36: return KEY_6; // '6'
    case 0x37: return KEY_7; // '7'
    case 0x38: return KEY_8; // '8'
    case 0x39: return KEY_9; // '9'

    // ===== ESPECIALES =====
    case 0x2E: return KEY_DOT;    // '.'
    case 0x0D: return KEY_ENTER;  // Enter
    case 0x08: return KEY_CLEAR;  // Clear / Backspace
    case 0x1B: return KEY_ESC;    // ESC
    case 0x21: return KEY_PGUP;   // PgUp
    case 0x22: return KEY_PGDN;   // PgDn

    default:
      return KEY_NONE;
  }
}

// ===== Manejo de la acción =====
void handleKeypadAction(KeypadAction action, uint8_t rawCode) {

  switch (action) {

    // ===== NUMÉRICAS 0–9 =====
    case KEY_0:
      DebugSerial.println("KEYPAD NUM: 0");
      dwinKeypadTouch(KP_0);
      break;

    case KEY_1:
      DebugSerial.println("KEYPAD NUM: 1");
      dwinKeypadTouch(KP_1);
      break;

    case KEY_2:
      DebugSerial.println("KEYPAD NUM: 2");
      dwinKeypadTouch(KP_2);
      break;

    case KEY_3:
      DebugSerial.println("KEYPAD NUM: 3");
      dwinKeypadTouch(KP_3);
      break;

    case KEY_4:
      DebugSerial.println("KEYPAD NUM: 4");
      dwinKeypadTouch(KP_4);
      break;

    case KEY_5:
      DebugSerial.println("KEYPAD NUM: 5");
      dwinKeypadTouch(KP_5);
      break;

    case KEY_6:
      DebugSerial.println("KEYPAD NUM: 6");
      dwinKeypadTouch(KP_6);
      break;

    case KEY_7:
      DebugSerial.println("KEYPAD NUM: 7");
      dwinKeypadTouch(KP_7);
      break;

    case KEY_8:
      DebugSerial.println("KEYPAD NUM: 8");
      dwinKeypadTouch(KP_8);
      break;

    case KEY_9:
      DebugSerial.println("KEYPAD NUM: 9");
      dwinKeypadTouch(KP_9);
      break;

    // ===== PUNTO DECIMAL =====
    case KEY_DOT:
      DebugSerial.println("KEYPAD DOT");
      dwinKeypadTouch(KP_DOT);
      break;

    // ===== ENTER =====
    case KEY_ENTER:
      DebugSerial.println("KEYPAD ENTER");
      dwinKeypadTouch(KP_ENTER);
      break;

    // ===== CLEAR / DEL =====
    case KEY_CLEAR:
      DebugSerial.println("KEYPAD CLEAR");
      dwinKeypadTouch(KP_CLEAR);
      delay(40);
      dwinKeypadTouch(AUTO_CURSOR);
      break;

    // ===== ESC → CANCEL =====
    case KEY_ESC:
      DebugSerial.println("KEYPAD ESC");
      dwinKeypadTouch(KP_CANCEL);
      break;

    // ===== PGUP / PGDN (si luego los usas para otra cosa) =====
    case KEY_PGUP:
      DebugSerial.println("KEYPAD PAGE UP");
      // aquí puedes mapear a otro botón DWIN si quieres más adelante
      dwinKeypadTouch(KP_DEL);
      break;

    case KEY_PGDN:
      DebugSerial.println("KEYPAD PAGE DOWN");
      // igual que arriba
      dwinKeypadTouch(KP_DEL);
      break;




    default:
      DebugSerial.print("KEYPAD UNKNOWN: 0x");
      DebugSerial.println(rawCode, HEX);
      break;
  }
}
