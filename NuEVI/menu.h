#ifndef __MENU_H
#define __MENU_H

#include "Wiring.h"

#define MENU_ROW_HEIGHT 9
#define MENU_HEADER_OFFSET 12
#define MENU_NUM_ROWS 6

//display states
#define DISPLAYOFF_IDL 0
#define MAIN_MENU 1
#define PATCH_VIEW 2
#define ADJUST_MENU   70
#define SETUP_BR_MENU 80
#define SETUP_CT_MENU 90
#define ROTATOR_MENU 100
#define VIBRATO_MENU 110

#define ARR_LEN(a)  (sizeof (a) / sizeof (a[0]))

#define BTN_DOWN 1
#define BTN_ENTER 2
#define BTN_UP 4
#define BTN_MENU 8



extern const unsigned long debounceDelay;           // the debounce time; increase if the output flickers
extern const unsigned long buttonRepeatInterval;
extern const unsigned long buttonRepeatDelay;
extern const unsigned long cursorBlinkInterval;    // the cursor blink toggle interval time
extern const unsigned long patchViewTimeUp;       // ms until patch view shuts off
extern const unsigned long menuTimeUp;           // menu shuts off after one minute of button inactivity



extern byte subVibSquelch;

void initDisplay();
void showVersion();
void menu();
void drawSensorPixels();
unsigned short readSetting(byte address);
void writeSetting(byte address, unsigned short value);

int updateAdjustMenu(uint32_t timeNow, uint8_t buttons, bool firstRun, bool updateSensor);
bool adjustPageUpdate(uint16_t buttonChanges, uint32_t timeNow);

#endif
