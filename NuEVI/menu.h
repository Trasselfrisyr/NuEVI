#ifndef __MENU_H
#define __MENU_H

#include "wiring.h"
#include "numenu.h"

#define MENU_ROW_HEIGHT 9
#define MENU_HEADER_OFFSET 12
#define MENU_NUM_ROWS 6

//display states
#define DISPLAYOFF_IDL 0
#define MAIN_MENU 1
#define PATCH_VIEW 2
#define ADJUST_MENU   3
#define SETUP_BR_MENU 4
#define SETUP_CT_MENU 5
#define ROTATOR_MENU 6
#define VIBRATO_MENU 7
#define ABOUT_MENU 8
#define EXTRAS_MENU 9
#define ROTA_MENU 10
#define ROTB_MENU 11
#define ROTC_MENU 12

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

extern byte subVibSquelch;  // TODO: This is broken <- subVibSquelch is never set, we need another way to expose what menu is open.

void initDisplay();
void showVersion();
void menu();
void drawSensorPixels();
void i2cScanDisplay();

int updateAdjustMenu(uint32_t timeNow, KeyState &input, bool firstRun, bool drawSensor);
bool adjustPageUpdate(KeyState &input, uint32_t timeNow);

#endif
