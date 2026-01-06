
#ifndef DWINTOOLS_INTERFACE_H
#define DWINTOOLS_INTERFACE_H


void writeU16(uint16_t vp, uint16_t value);
void writeU32(uint16_t vp, uint32_t value);
void clearVP(uint16_t vp, uint16_t length);
void writeTextClean(uint16_t vp, const char* txt, uint16_t maxLen);
void dwinChangePage_VP(uint16_t page);

//funciones numericas
uint16_t dwinDecodeU16(const uint8_t *d);
uint32_t dwinDecodeU32(const uint8_t *d);
float dwinDecodeFloat(const uint8_t *d);
bool readU16VP(uint16_t vp, uint16_t &outValue);
bool readFloatVP(uint16_t vp, float &outValue);
float decodeFloatBE(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);


#endif
