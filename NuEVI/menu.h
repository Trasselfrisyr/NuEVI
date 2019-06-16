#ifndef __MENU_H
#define __MENU_H

#include "Wiring.h"

#define MENU_ROW_HEIGHT 9
#define MENU_HEADER_OFFSET 3
#define MENU_NUM_ROWS 5

//display states
#define DISPLAYOFF_IDL 0
#define MAIN_MENU 1
#define PATCH_VIEW 2
#define ADJUST_MENU   70
#define SETUP_BR_MENU 80
#define SETUP_CT_MENU 90
#define ROTATOR_MENU 100
#define VIBRATO_MENU 110


extern byte subVibSquelch;

void initDisplay();
void showVersion();
void menu();
void drawSensorPixels();
unsigned short readSetting(byte address);
void writeSetting(byte address, unsigned short value);

#endif
