#pragma once
#include <Arduino.h>

// ===== Acciones posibles del teclado =====
enum KeypadAction {
  KEY_NONE = 0,

  KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
  KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

  KEY_DOT,
  KEY_ENTER,
  KEY_CLEAR,
  KEY_ESC,
  KEY_PGUP,
  KEY_PGDN
};

// ===== API =====
KeypadAction parseKeypad(uint8_t code);
void handleKeypadAction(KeypadAction action, uint8_t rawCode);