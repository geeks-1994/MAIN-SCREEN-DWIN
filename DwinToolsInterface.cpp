#include "SerialPorts.h"
#include "DwinToolsInterface.h"
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
