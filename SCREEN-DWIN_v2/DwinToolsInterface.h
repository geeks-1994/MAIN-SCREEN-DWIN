
#ifndef DWINTOOLS_INTERFACE_H
#define DWINTOOLS_INTERFACE_H



// ===== Keypad DWIN - códigos estándar =====
#define KP_0      0
#define KP_1      1
#define KP_2      2
#define KP_3      3
#define KP_4      4
#define KP_5      5
#define KP_6      6
#define KP_7      7
#define KP_8      8
#define KP_9      9

#define KP_DOT    10
#define KP_DEL    11
#define KP_ENTER  12
#define KP_CANCEL 13
#define KP_CLEAR  14
#define AUTO_CURSOR 15


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
void DWIN_ReadFrames(Stream &port);
void dwinKeypadTouch(uint8_t key); 
void dwinBeep();

//funciones de buzzer
void dwinBuzzerInit_ByRegister();
void dwinBuzzerBeep();
void dwinSetBuzzFreqDiv1(uint8_t div1);
void dwinSetBuzzTime(uint8_t time10ms);
void dwinSetBuzzDuty(uint16_t duty);



#endif
