
#ifndef SCREEN_FUNCTIONS_H
#define SCREEN_FUNCTIONS_H

#include <stdint.h>

typedef struct {
    char inputNameKeypad[50];
    char decimalCursors[50];
    char device[50];
} ScreenFlow;


extern  ScreenFlow screenflow;

void GoHomePage(char event[][50]);
void getConfigData(char event[][50]);
void Showbanner(char event[][50]);
void ResponsePing(char event[][50]);
void DinaRefuel(char event[][50]);
void printRefuel(char event[][50]);
void Getfindespacho(char event[][50]);
void ShowInputScreen(char event[][50]);
void Printedvehicle(char event[][50]);
void QRscreen(char event[][50]);
void DebugSend(const char *label, const char *data);
void splashScreen(int value);
void dwinStartupXP();
void dwinErrorTone_Loud();

#endif