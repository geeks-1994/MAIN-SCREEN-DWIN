#include <stdio.h>
#include "SerialPorts.h"
#include "DwinToolsInterface.h"
#include "toolsFunctionsScreen.h"
#include "ParseModule.h"
#include <math.h>

// =====================================================
// CONFIG TECLADO (VPs LIBRES)
// =====================================================
static const uint16_t VP_KEY_FLOAT      = 0x1014; // float (2 words) -> 0x1014..0x1017
static const uint16_t VP_KEY_ENTER_FLAG = 0x1018; // u16  (1 word)  -> 0x1018..0x1019
static const uint16_t VP_KEY_DOT_USED   = 0x101A; // u16  (opcional)


// =====================================================
// FUNCIONES NUMÉRICAS DGUS (WRITE)
// =====================================================
void writeU16(uint16_t vp, uint16_t value) {
  uint8_t frame[8] = {
    0x5A, 0xA5,
    0x05,
    0x82, 
    (uint8_t)(vp >> 8),
    (uint8_t)(vp & 0xFF),
    (uint8_t)(value >> 8),
    (uint8_t)(value & 0xFF)
  };
  DwinSerial.write(frame, 8);
};


void writeU32(uint16_t vp, uint32_t value) {
  uint8_t frame[10] = {
    0x5A, 0xA5,
    0x07,
    0x82,
    (uint8_t)(vp >> 8),
    (uint8_t)(vp & 0xFF),
    (uint8_t)(value >> 24),
    (uint8_t)(value >> 16),
    (uint8_t)(value >> 8),
    (uint8_t)value
  };
  DwinSerial.write(frame, 10);
};


// =====================================================
// TEXTO DGUS (ASCII) – LIMPIO
// =====================================================
void clearVP(uint16_t vp, uint16_t length) {
  uint8_t frame[6 + length];

  frame[0] = 0x5A;
  frame[1] = 0xA5;
  frame[2] = 3 + length;
  frame[3] = 0x82;
  frame[4] = vp >> 8;
  frame[5] = vp & 0xFF;

  memset(&frame[6], 0x00, length);
  DwinSerial.write(frame, 6 + length);
  delay(6);
};

void writeTextClean(uint16_t vp, const char* txt, uint16_t maxLen) {
  clearVP(vp, maxLen);
  delay(6);

  uint16_t len = strlen(txt);
  if (len > maxLen) len = maxLen;

  uint8_t frame[6 + len];
  frame[0] = 0x5A;
  frame[1] = 0xA5;
  frame[2] = 3 + len;
  frame[3] = 0x82;
  frame[4] = vp >> 8;
  frame[5] = vp & 0xFF;

  memcpy(&frame[6], txt, len);
  DwinSerial.write(frame, 6 + len);
};



// =====================================================
// CAMBIO DE PÁGINA (VP SISTEMA 0x0084)
// =====================================================
void dwinChangePage_VP(uint16_t page) {
  uint32_t value = (0x5A01UL << 16) | (uint32_t)page;
  writeU32(0x0084, value);
};

// =====================================================
// LECTURA DGUS – N WORDS (GENÉRICO)
// * IMPORTANTE: valida VP y words en la respuesta
// =====================================================
bool dwinReadWords(uint16_t vp, uint8_t words, uint8_t *outData, uint16_t timeoutMs = 150) {
  // Request: 5A A5 04 83 vpH vpL words
  uint8_t req[7] = {
    0x5A, 0xA5,
    0x04,
    0x83,
    (uint8_t)(vp >> 8),
    (uint8_t)(vp & 0xFF),
    words
  };

  // Limpia RX viejo (evita mezclar frames)
  while (DwinSerial.available()) DwinSerial.read();

  // Envía request
  DwinSerial.write(req, sizeof(req));

  uint32_t t0 = millis();
  uint8_t buf[64];
  uint8_t idx = 0;

  // Lee hasta tener frame completo: total = 3 + LEN
  while ((millis() - t0) < timeoutMs && idx < sizeof(buf)) {
    if (DwinSerial.available()) {
      buf[idx++] = (uint8_t)DwinSerial.read();
      if (idx >= 3) {
        uint8_t len = buf[2];
        if (idx >= (uint8_t)(3 + len)) break;
      }
    }
  }

  // Validaciones mínimas
  if (idx < (uint8_t)(7 + words * 2)) return false;
  if (buf[0] != 0x5A || buf[1] != 0xA5) return false;

  // Estructura esperada: 5A A5 LEN 83 vpH vpL words data...
  if (buf[3] != 0x83) return false;
  if (buf[4] != (uint8_t)(vp >> 8)) return false;
  if (buf[5] != (uint8_t)(vp & 0xFF)) return false;
  if (buf[6] != words) return false;

  memcpy(outData, &buf[7], words * 2);
  return true;
}

// =====================================================
// DECODIFICADORES (PLUG-IN)
// =====================================================
uint16_t dwinDecodeU16(const uint8_t *d) {
  return ((uint16_t)d[0] << 8) | d[1];
}

uint32_t dwinDecodeU32(const uint8_t *d) {
  return ((uint32_t)d[0] << 24) |
         ((uint32_t)d[1] << 16) |
         ((uint32_t)d[2] << 8)  |
         d[3];
};

// Float DGUS: probamos normal (ABCD) y swap-words (CDAB)
float dwinDecodeFloat(const uint8_t *d) {
  uint32_t u1 = ((uint32_t)d[0] << 24) | ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | d[3];
  float f1; memcpy(&f1, &u1, 4);

  uint32_t u2 = ((uint32_t)d[2] << 24) | ((uint32_t)d[3] << 16) | ((uint32_t)d[0] << 8) | d[1];
  float f2; memcpy(&f2, &u2, 4);

  if (isfinite(f1) && fabs(f1) < 1e7) return f1;
  return f2;
};

// Helpers libres (opcional)
bool readU16VP(uint16_t vp, uint16_t &outValue) {
  uint8_t d[2];
  if (!dwinReadWords(vp, 1, d)) return false;
  outValue = dwinDecodeU16(d);
  return true;
}

bool readFloatVP(uint16_t vp, float &outValue) {
  uint8_t d[4];
  if (!dwinReadWords(vp, 2, d)) return false;
  outValue = dwinDecodeFloat(d);
  return true;
};


float decodeFloatBE(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
  uint32_t u = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | (uint32_t)b3;
  float f;
  memcpy(&f, &u, 4);
  return f;
};



// ------------------ 1) Convertidor a float (Big-Endian) ------------------
float DWIN_BytesToFloatBE(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
  union {
    uint32_t u32;
    float f;
  } u;

  u.u32 = ((uint32_t)b0 << 24) |
          ((uint32_t)b1 << 16) |
          ((uint32_t)b2 << 8)  |
          ((uint32_t)b3);

  return u.f;
};


// ------------------ 2) Handler central de mensajes DWIN ------------------
/*
  type:
    1 = EVENT_KEY   (Return Key Code / botones)
    2 = VP_U16      (VP con entero 16-bit)
    3 = VP_FLOAT    (VP con float 32-bit)
*/
void DWIN_HandleMessage(uint8_t type, uint16_t vp, uint16_t u16val, float fval, uint8_t key) {
  // 🔥 EJEMPLO: Tu VP de eventos
  if (type == 1 && vp == 0x2000) {
    // key: F0 cancel, F1 ok, F2 back (según DGUS)
    if (key == 0xF0) {
      DebugSend("[RX]", "🛑 CANCEL (vp=0x2000) code=0x");
      DebugSerial.println(u16val, HEX);   // u16val puede ser tu "codigo de fallo"
      // cancelarDespacho(); // <-- tu función real
    //  DebugSend("[TX]",screenflow.inputNameKeypad);
      char answer[50];
      snprintf(answer,sizeof(answer),"<RES|SCREEN|%s|SGET|%s|CANCEL|>", screenflow.device,screenflow.inputNameKeypad);      
      
      DebugSend("[TX]",answer);

      HostSerial.println(SendCommandCPU(answer));

    } else if (key == 0xF1) {
      DebugSerial.print("✅ OK code=0x");

      DebugSerial.println(u16val, HEX);
    } else if (key == 0xF2) {
      DebugSerial.println("↩️ BACK");
    }
    return;
  }

  // 🔥 EJEMPLO: Leer un float en VP 0x1014 (teclado numérico)
  if (type == 3 && vp == 0x1014) {
    DebugSerial.print("🔢 VP 0x1014 float = ");
    DebugSerial.println(fval, 2);

  //DebugSend("[TX]",screenflow.inputNameKeypad);
      char answer[100];
      snprintf(answer,sizeof(answer),"<RES|SCREEN|%s|SGET|%s|OK|%.2f>", screenflow.device,screenflow.inputNameKeypad,fval);      
      
      DebugSend("[TX]",answer);

      HostSerial.println(SendCommandCPU(answer));
    return;
  }

  // 🔥 EJEMPLO: Cualquier entero 16-bit
  if (type == 2) {
    DebugSerial.print("📥 VP=0x");
    DebugSerial.print(vp, HEX);
    DebugSerial.print(" U16=0x");
    DebugSerial.println(u16val, HEX);

    return;
  }

  // Si no calza en nada, lo dejas visible
  DebugSerial.print("ℹ️ DWIN msg type=");
  DebugSerial.print(type);
  DebugSerial.print(" vp=0x");
  DebugSerial.print(vp, HEX);
  DebugSerial.print(" u16=0x");
  DebugSerial.print(u16val, HEX);
  DebugSerial.print(" key=0x");
  DebugSerial.println(key, HEX);
};



// ------------------ 3) Parser de frames binarios DWIN ------------------
/*
  Lee frames tipo: 5A A5 LEN CMD ...
  Soporta:
   - Upload VP + float:  5A A5 08 83 VP VP 02 D0 D1 D2 D3   (LEN puede variar por modelo)
   - Return Key Code:    5A A5 06 83 VP VP Vh Vl KEY
   - Upload VP u16:      5A A5 05 83 VP VP Vh Vl            (según configuración)

  NOTA: LEN en DGUS = cantidad de bytes DESPUÉS de LEN (incluye CMD).
        Total frame = 3 + LEN
*/
void DWIN_ReadFrames(Stream &port) {
  static uint8_t buf[64];
  static uint8_t idx = 0;
  static int expected = -1;

  while (port.available()) {
    uint8_t b = (uint8_t)port.read();

    // ---- sync header 5A A5 ----
    if (idx == 0) {
      if (b != 0x5A) continue;
      buf[idx++] = b;
      continue;
    }
    if (idx == 1) {
      if (b != 0xA5) { idx = 0; continue; }
      buf[idx++] = b;
      continue;
    }

    // ---- store bytes ----
    buf[idx++] = b;

    // LEN arrived -> expected total bytes = 3 + LEN
    if (idx == 3) {
      expected = 3 + buf[2];
      if (expected > (int)sizeof(buf)) { idx = 0; expected = -1; }
    }

    // ---- complete frame ----
    if (expected > 0 && idx >= expected) {

      // Debug HEX opcional
      Serial.print("RX: ");
      for (int i = 0; i < idx; i++) {
        if (buf[i] < 0x10) Serial.print('0');
        Serial.print(buf[i], HEX);
        Serial.print(' ');
      }
      Serial.println();

      // Solo procesamos upload (83) aquí
      if (buf[3] == 0x83 && idx >= 7) {
        uint16_t vp = ((uint16_t)buf[4] << 8) | buf[5];

        // Caso A) Return Key Code: LEN=06 y idx=9 -> VP(2)+VAL(2)+KEY(1)
        if (buf[2] == 0x06 && idx >= 9) {
          uint16_t val = ((uint16_t)buf[6] << 8) | buf[7];
          uint8_t key  = buf[8];
          DWIN_HandleMessage(1, vp, val, 0.0f, key);
        }
        // Caso B) Upload U16 simple: LEN=05 y idx=8 -> VP(2)+VAL(2)
        else if (buf[2] == 0x05 && idx >= 8) {
          uint16_t val = ((uint16_t)buf[6] << 8) | buf[7];
          DWIN_HandleMessage(2, vp, val, 0.0f, 0);
        }
        // Caso C) Upload float típico: ... VP(2)+WORDS(1=02)+4 bytes
        // Ejemplo que tú validabas: buf[6]==0x02 y luego 4 bytes
        else if (idx >= 11 && buf[6] == 0x02) {
          float f = DWIN_BytesToFloatBE(buf[7], buf[8], buf[9], buf[10]);
          DWIN_HandleMessage(3, vp, 0, f, 0);
        }
      }

      // reset
      idx = 0;
      expected = -1;
    }

    // safety
    if (idx >= sizeof(buf)) {
      idx = 0;
      expected = -1;
    }
  }
};


// función interna para enviar frames
static void sendDwinFrame(const uint8_t *frame, size_t len) {
  if (!frame || len == 0) return;
  DwinSerial.write(frame, len);
  DwinSerial.flush();
}


// ===== FUNCIÓN ESTÁNDAR =====
void dwinKeypadTouch(uint8_t key) {

  switch (key) {

    case KP_1: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x00,0xFA,0x00,0xB4};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_2: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x01,0x80,0x00,0xB4};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_3: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x02,0x06,0x00,0xB4};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_4: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x00,0xFA,0x00,0xE6};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_5: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x01,0x80,0x00,0xE6};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_6: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x02,0x06,0x00,0xE6};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_7: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x00,0xFA,0x01,0x1E};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_8: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x01,0x80,0x01,0x1E};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_9: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x02,0x06,0x01,0x1E};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_0: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x01,0x80,0x01,0x80};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_DOT: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x02,0x8C,0x01,0x44};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_DEL: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x02,0x8A,0x00,0xB4};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_ENTER: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x02,0x48,0x01,0xAE};
      sendDwinFrame(f, sizeof(f));
    } break;

    case KP_CANCEL: {
      const uint8_t f[] = {0x5A,0xA5,0x0B,0x82,0x00,0xD4,0x5A,0xA5,0x00,0x04,0x01,0x3D,0x01,0xAE};
      sendDwinFrame(f, sizeof(f));
    } break;


case KP_CLEAR: {
  const uint8_t f[] = {
    0x5A, 0xA5, 0x0B, 0x82,
    0x00, 0xD4,
    0x5A, 0xA5,
    0x00, 0x04,
    0x02, 0x8A,
    0x00, 0xE6
  };
  sendDwinFrame(f, sizeof(f));
} break;


case AUTO_CURSOR: {
  const uint8_t f[] = {
    0x5A, 0xA5, 0x0B, 0x82,
    0x00, 0xD4,
    0x5A, 0xA5,
    0x00, 0x04,
    0x01, 0x94,
    0x00, 0x4F
  };
  sendDwinFrame(f, sizeof(f));
} break;


    default:
      DebugSerial.println("⚠ dwinKeypadTouch: tecla desconocida");
      break;
  }
};



void dwinBeep() {
  uint8_t frame[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x01};
  DwinSerial.write(frame,sizeof(frame));
};


// Escribir registros de control (comando 0x80, como en el video)
void dwinWriteRegister80(uint8_t reg, const uint8_t *data, uint8_t dataLen) {
  uint8_t frame[20];

  uint8_t len = 3 + dataLen;  // 80 + ID + RG + DATA...

  frame[0] = 0x5A;
  frame[1] = 0xA5;
  frame[2] = len;
  frame[3] = 0x80;        // ⬅️ comando WRITE REGISTER
  frame[4] = 0x00;        // page ID = 0 (donde están 0x27–0x2D)
  frame[5] = reg;         // dirección del registro (0x27, 0x28, etc.)

  for (uint8_t i = 0; i < dataLen; i++) {
    frame[6 + i] = data[i];
  }

  DwinSerial.write(frame, 6 + dataLen);
};


void dwinBuzzerInit_ByRegister() {
  // 0x27: BUZZ_Set_En = 0x5A (habilita configuración)
  {
    uint8_t v = 0x5A;
    dwinWriteRegister80(0x27, &v, 1);
  }

  // 0x28: BUZZ_Freq_DIV1 (ejemplo: 0x6E = valor de fábrica ~2.5kHz)
  {
    uint8_t v = 0x6E;
    dwinWriteRegister80(0x28, &v, 1);
  }

  // 0x29: BUZZ_Freq_DIV2 = 0x0BB8 (2 bytes)
  {
    uint8_t buf[2] = {0x0B, 0xB8};
    dwinWriteRegister80(0x29, buf, 2);
  }

  // 0x2B: BUZZ_Freq_Duty = 0x00F0 (8% duty)
  {
    uint8_t buf[2] = {0x00, 0xF0};
    dwinWriteRegister80(0x2B, buf, 2);
  }

  // 0x2D: BUZZ_Time (unidad 10 ms). Ej: 0x0A = 100ms
  {
    uint8_t v = 0x0A;   // prueba luego con 0x05, 0x14, etc.
    dwinWriteRegister80(0x2D, &v, 1);
  }
};

void dwinSetBuzzTime(uint8_t time10ms) {
  // time10ms: 8 = 80 ms, 12 = 120 ms, etc.
  dwinWriteRegister80(0x2D, &time10ms, 1);
};

void dwinSetBuzzFreqDiv1(uint8_t div1) {
  // Valores más bajos = tono más agudo
  dwinWriteRegister80(0x28, &div1, 1);
};


// Duty del buzzer (0x2B) – valores más grandes = más fuerte (hasta donde aguante el buzzer)
void dwinSetBuzzDuty(uint16_t duty) {
  uint8_t buf[2] = {
    (uint8_t)(duty >> 8),
    (uint8_t)(duty & 0xFF)
  };
  dwinWriteRegister80(0x2B, buf, 2);
};



void dwinBuzzerBeep() {
  // VP 0x00A0, comando 0x82 (Write VP)
  uint8_t frame[] = {
    0x5A, 0xA5,
    0x05,       // len = 0x82 + addrH + addrL + dataH + dataL
    0x82,
    0x00, 0xA0, // VP 0x00A0
    0x00, 0x01  // valor 0x0001
  };
  DwinSerial.write(frame, sizeof(frame));
}


