#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include <Arduino.h>

#include "menu.h"
#include "settings.h"
#include "hardware.h"
#include "config.h"
#include "globals.h"
#include "midi.h"
#include "numenu.h"
#include "led.h"

enum CursorIdx {
  EMain,
  EBreath,
  EControl,
  ERotator,
  EVibrato,
  EExtras,
  ERotSubA,
  ERotSubB,
  ERotSubC,
  // NEVER ADD ANYTHING AFTER THIS, ONLY ABOVE
  NUM_CURSORS
};

// Allocate some space for cursors
static byte cursors[CursorIdx::NUM_CURSORS];
static byte offsets[CursorIdx::NUM_CURSORS];
static byte activeSub[CursorIdx::NUM_CURSORS];

byte cursorNow;
static byte FPD = 0;


// constants
const unsigned long debounceDelay = 30;           // the debounce time; increase if the output flickers
const unsigned long buttonRepeatInterval = 50;
const unsigned long buttonRepeatDelay = 400;
const unsigned long cursorBlinkInterval = 300;    // the cursor blink toggle interval time
const unsigned long patchViewTimeUp = 2000;       // ms until patch view shuts off
const unsigned long menuTimeUp = 60000;           // menu shuts off after one minute of button inactivity


static unsigned long menuTime = 0;
static unsigned long patchViewTime = 0;
unsigned long cursorBlinkTime = 0;          // the last time the cursor was toggled

//Display state
static byte menuState= DISPLAYOFF_IDL;
static byte stateFirstRun = 1;

// The external function of subSquelch has been broken,
// need to come up with a smart way to make it work again.
// The status led was update when the Squelch menu was open.
byte subVibSquelch = 0; //extern

 // 'NuEVI' or 'NuRAD' logo
#define LOGO16_GLCD_WIDTH  128
#define LOGO16_GLCD_HEIGHT 64
#if defined(NURAD)
static const unsigned char PROGMEM nurad_logo_bmp[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x00, 0x03, 0xf9, 0xff, 0xff, 0xfc, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x07, 0xf9, 0xff, 0xff, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x01, 0xc0, 0x00, 0x0e, 0x19, 0x80, 0x00, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xff, 0xfc, 0xc0, 0x00, 0x1d, 0xd9, 0xbf, 0xff, 0xf3, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xff, 0xfe, 0xc0, 0x00, 0x3b, 0xd9, 0xbf, 0xff, 0xfb, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x06, 0xc0, 0x00, 0x77, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x06, 0xc0, 0x00, 0xee, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x06, 0xc0, 0x01, 0xdc, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0x00, 0x08, 0x00, 0x00, 0x6c, 0x00, 0x06, 0xc0, 0x03, 0xb8, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0x00, 0x18, 0x00, 0x00, 0x6f, 0xff, 0xfe, 0xc0, 0x07, 0x7f, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0x00, 0x18, 0x00, 0x00, 0x6f, 0xff, 0xfc, 0xc0, 0x0e, 0xff, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0xc0, 0x18, 0x00, 0x00, 0x60, 0x00, 0x01, 0xc0, 0x1c, 0x00, 0x19, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0xc0, 0x38, 0x00, 0x00, 0x6f, 0xfb, 0xff, 0x80, 0x3b, 0xff, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x00, 0xc0, 0x30, 0x00, 0x00, 0x6f, 0xfd, 0xff, 0x00, 0x77, 0xff, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x01, 0xe0, 0x30, 0x00, 0x00, 0x6c, 0x0e, 0xe0, 0x00, 0xee, 0x00, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x01, 0xe0, 0x30, 0x00, 0x00, 0x6c, 0x07, 0x70, 0x01, 0xdc, 0x00, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x01, 0xb0, 0x30, 0x00, 0x00, 0x6c, 0x03, 0xb8, 0x03, 0xb8, 0x00, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x01, 0xb0, 0x30, 0x00, 0x00, 0x6c, 0x01, 0xdc, 0x07, 0x70, 0x00, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x01, 0x98, 0x20, 0xc0, 0x40, 0x6c, 0x00, 0xee, 0x0e, 0xe0, 0x00, 0xd9, 0xb0, 0x00, 0x1b, 0x00,
  0x03, 0x0c, 0x20, 0xc0, 0xc0, 0x6c, 0x00, 0x77, 0x1d, 0xc0, 0x00, 0xd9, 0xbf, 0xff, 0xfb, 0x00,
  0x03, 0x0c, 0x20, 0xc0, 0xc0, 0x6c, 0x00, 0x3b, 0xbb, 0x80, 0x00, 0xd9, 0xbf, 0xff, 0xf3, 0x00,
  0x03, 0x06, 0x20, 0xc1, 0xc0, 0x6c, 0x00, 0x1d, 0xf7, 0x00, 0x00, 0xd9, 0x80, 0x00, 0x07, 0x00,
  0x03, 0x03, 0x20, 0x83, 0x80, 0x6c, 0x00, 0x0e, 0xee, 0x00, 0x00, 0xd9, 0xff, 0xff, 0xfe, 0x00,
  0x03, 0x03, 0xb1, 0x83, 0x80, 0x6c, 0x00, 0x07, 0x5c, 0x00, 0x00, 0xd9, 0xff, 0xff, 0xfc, 0x00,
  0x03, 0x01, 0xf1, 0x87, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0xf1, 0x8d, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x71, 0xb9, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x31, 0xf1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0xc1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#else
static const unsigned char PROGMEM nuevi_logo_bmp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xe3, 0x60, 0x00, 0x07, 0x73, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xe3, 0x60, 0x00, 0x0e, 0xe3, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0x60, 0x00, 0x1d, 0xc3, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x60, 0x00, 0x3b, 0x83, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x60, 0x00, 0x77, 0x03, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x60, 0x00, 0xee, 0x03, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x60, 0x01, 0xdc, 0x03, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x60, 0x03, 0xb8, 0x03, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x60, 0x07, 0x70, 0x03, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x60, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x60, 0x0e, 0xe0, 0x03, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x60, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x60, 0x1d, 0xc0, 0x03, 0x60, 0x00,
  0x00, 0x03, 0x00, 0x60, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0x60, 0x3b, 0x80, 0x03, 0x60, 0x00,
  0x00, 0x03, 0x00, 0xe0, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x60, 0x77, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x03, 0x00, 0xc0, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x60, 0xee, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x03, 0x80, 0xc0, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x61, 0xdc, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x07, 0x80, 0xc0, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x63, 0xb8, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x07, 0xc0, 0xc0, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x67, 0x70, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x06, 0xc0, 0xc0, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x6e, 0xe0, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x06, 0x60, 0xc1, 0x01, 0x01, 0xb0, 0x00, 0x00, 0x03, 0x7d, 0xc0, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x06, 0x30, 0xc3, 0x03, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x7b, 0x80, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x0c, 0x30, 0xc3, 0x07, 0x01, 0xbf, 0xff, 0xff, 0xe3, 0x77, 0x00, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x0c, 0x1c, 0xc3, 0x06, 0x01, 0x80, 0x00, 0x00, 0x03, 0x0e, 0x00, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x0c, 0x0c, 0xc2, 0x0e, 0x01, 0xff, 0xff, 0xff, 0xe3, 0xfc, 0x00, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x0c, 0x0e, 0xc6, 0x1e, 0x01, 0xff, 0xff, 0xff, 0xe3, 0xf8, 0x00, 0x00, 0x03, 0x60, 0x00,
  0x00, 0x0c, 0x07, 0xc6, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x03, 0xc6, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x01, 0xc7, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x00, 0xc7, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x00, 0x03, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

extern void readSwitches(void);
extern Adafruit_MPR121 touchSensor;

extern const MenuPage mainMenuPage; // Forward declaration.


#define OLED_RESET 4
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET,1000000,1000000);

int drawBatt(int x,int y){
  display.drawRect(x+1,y,3,3,WHITE);
  display.drawRect(x,y+2,5,12,WHITE);
  int bar=0;
  int vMeterReading = battAvg;
  switch (batteryType){
    case 0:
      bar = constrain((10 *(vMeterReading - ALK_BAT_LOW)) / (ALK_BAT_FULL - ALK_BAT_LOW),0,10);
      break;
    case 1:
      bar = constrain((10 * (vMeterReading - NMH_BAT_LOW)) / (NMH_BAT_FULL - NMH_BAT_LOW),0,10);
      break;
    case 2:
      bar = constrain((10 *(vMeterReading - LIP_BAT_LOW)) / (LIP_BAT_FULL - LIP_BAT_LOW),0,10);
  }
  display.fillRect(x+1,y+13-bar,3,constrain(bar,0,10),WHITE);
  return bar;
}

void drawFlash(int x, int y){
  display.drawLine(x+5,y,x,y+6,WHITE);
  display.drawLine(x,y+6,x+5,y+6,WHITE);
  display.drawLine(x,y+12,x+5,y+6,WHITE);
}

void initDisplay() {

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  display.clearDisplay();
  #if defined(NURAD)
  display.drawBitmap(0,0,nurad_logo_bmp,LOGO16_GLCD_WIDTH,LOGO16_GLCD_HEIGHT,1);
  #else
  display.drawBitmap(0,0,nuevi_logo_bmp,LOGO16_GLCD_WIDTH,LOGO16_GLCD_HEIGHT,1);
  #endif
  display.display();

  memset(cursors, 0, sizeof(cursors));
  memset(offsets, 0, sizeof(offsets));
  memset(activeSub, 0, sizeof(activeSub));
}

void showVersion() {
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(85,52);
  display.print("v.");
  display.println(FIRMWARE_VERSION);
  display.display();
}

void i2cScanDisplay(){
  uint8_t target; // slave addr
  byte error;
  while(1){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("MPR121 board scan");
    display.display();
    for(target = 0x5A; target <= 0x5D; target++) // sweep addr
    {
        Wire.beginTransmission(target);       // slave addr
        error = Wire.endTransmission();
        delay(500);
        display.print("Addr 0x");
        display.print(target,HEX);
        if (error)
          display.print(" N\n");
        else
          display.print(" Y\n");
        display.display();
    }
    delay(1000);
    display.println();
    display.println("MENU to rescan");
    display.println("Power off to exit");
    display.display();
    while (digitalRead(mPin)){
      delay(100);
    }
  }
}

// Assumes dest points to a buffer of atleast 7 bytes.
static const char* numToString(int16_t value, char* dest, bool plusSign = false) {
  char* ptr = dest;
  uint16_t absVal = abs(value);
  int c = 1;

  if(value < 0) {
    *ptr++ = '-';
  } else if(plusSign && value) {
    *ptr++ = '+';
  }
  while((c*10) < absVal+1) c *= 10;
  while(c > 0) {
    int tmp = absVal / c;
    *ptr++ = tmp + '0';
    absVal %= c;
    c /= 10;
  }
  *ptr = 0;
  return dest;
}

static void plotSubOption(const char* label, const char* unit = nullptr) {
  int text_x, unit_x;
  int label_pixel_width = strlen(label)*12;

  if(unit == nullptr) {
    text_x = 96 - (label_pixel_width/2);
  } else {
      int unit_pixel_width = strlen(unit)*6;
      int halfSum = (label_pixel_width + unit_pixel_width)/2;
      text_x = 96 - halfSum;
      unit_x = 96 + halfSum - unit_pixel_width;
      display.setCursor(unit_x,40);
      display.setTextSize(1);
      display.println(unit);
  }

    display.setTextSize(2);
    display.setCursor(text_x,33);
    display.println(label);
}

static bool drawSubMenu(const MenuPage *page) {
    int index = cursors[page->cursor];
    // TODO: Null check subMenuFunc
    const MenuEntry* subEntry =  page->entries[index];
    switch(subEntry->type) {
      case MenuType::ESub:
        {
          char buffer[12];
          const char* labelPtr = nullptr;
          const MenuEntrySub* sub = (const MenuEntrySub*)subEntry;
          sub->getSubTextFunc(*sub, buffer, &labelPtr);

          // If EMenuEntryCustom flag is set, we assume that the getSubTextFunc
          // rendered by it self.
          if( !(sub->flags & EMenuEntryCustom)) {
            plotSubOption(buffer, labelPtr);
          }
        }
        break;

      default:
        break;
    }
    return true;
}

static void clearSub(){
  display.fillRect(63,11,64,52,BLACK);
}

static void clearSubValue() {
  display.fillRect(65, 24, 60, 37, BLACK);
}

static bool updateSubMenuCursor(const MenuPage *page, uint32_t timeNow)
{
  if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    cursorBlinkTime = timeNow;
    if (cursorNow == WHITE) {
      cursorNow = BLACK;
      clearSubValue();
      return true;
    } else {
      cursorNow = WHITE;
      return drawSubMenu(page);
    }
  }
  return false;
}

static void plotMenuEntries(const MenuPage *page, bool clear = false) {
  int row = 0;
  if(clear) {
    display.fillRect( 0, MENU_HEADER_OFFSET, 63, 64-MENU_HEADER_OFFSET, BLACK);
  }
  display.setTextSize(1);
  int offset = offsets[page->cursor];
  for(int item = offset; (item < page->numEntries) && (row < MENU_NUM_ROWS); item++, row++) {
    int rowPixel = (row)*MENU_ROW_HEIGHT + MENU_HEADER_OFFSET;
    const char* lineText = page->entries[item]->title;
    display.setCursor(0,rowPixel);
    display.println(lineText);
  }

  if(offset)
    display.drawTriangle(58, MENU_HEADER_OFFSET, 57, MENU_HEADER_OFFSET+3, 59, MENU_HEADER_OFFSET+3, WHITE);

  if((offset+MENU_NUM_ROWS) < page->numEntries)
    display.drawTriangle(58, 63, 57, 60, 59, 60, WHITE);
}

typedef void (*MenuTitleGetFunc)(char*out);

static void drawMenu(const MenuPage *page) {
  //Initialize display and draw menu header + line
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  if(page->flags & EMenuCustomTitle) {
    // This is a bit hacky, but we are reusing the title pointer as a function pointer
    MenuTitleGetFunc func = (MenuTitleGetFunc)page->title;
    char buffer[23];
    func(buffer);
    display.println(buffer);
  } else
    display.println(page->title);
  display.drawLine(0, MENU_ROW_HEIGHT, 127, MENU_ROW_HEIGHT, WHITE);

  plotMenuEntries(page);
}

static void mainTitleGetStr(char* out) {
  //Construct the title including voltage reading.
  //Involves intricate splicing of the title string with battery voltage
  char menuTitle[] = "MENU         XXX Y.YV"; //Allocate string buffer of appropriate size with some placeholders
  char* splice1 = menuTitle + 13;
  char* splice2 = menuTitle + 17;

  int vMeterReading = battAvg;
  memcpy(splice1, (vMeterReading > 3020) ? "USB" : "BAT", 3);
  int vLowLimit = NMH_BAT_LOW;
  switch (batteryType){
    case 0:
      vLowLimit = ALK_BAT_LOW;
      break;
    case 1:
      vLowLimit = NMH_BAT_LOW;
      break;
    case 2:
      vLowLimit = LIP_BAT_LOW;
  }
  if (vMeterReading <= vLowLimit) { //2300 alkaline, 2250 lipo, 2200 nimh
    memcpy(splice2, "LOW ", 4);
  } else {
    int voltage = map(vMeterReading,2200,3060,36,50);
    splice2[0] = (voltage/10)+'0';
    splice2[2] = (voltage%10)+'0';
  }
  strncpy(out, menuTitle, 22);
}

#if defined(NURAD)
static void drawTrills(){
  if (LH1) display.fillRect(0,0,5,5,WHITE); else display.drawRect(0,0,5,5,WHITE);
  if (LH2) display.fillRect(10,0,5,5,WHITE); else display.drawRect(10,0,5,5,WHITE);
  if (LH3) display.fillRect(20,0,5,5,WHITE); else display.drawRect(20,0,5,5,WHITE);
}
#else
static void drawTrills(){
  if (K5) display.fillRect(0,0,5,5,WHITE); else display.drawRect(0,0,5,5,WHITE);
  if (K6) display.fillRect(10,0,5,5,WHITE); else display.drawRect(10,0,5,5,WHITE);
  if (K7) display.fillRect(20,0,5,5,WHITE); else display.drawRect(20,0,5,5,WHITE);
}
#endif

static void drawPatchView(){
  display.clearDisplay();
  if (FPD){
    drawTrills();
  }
  display.setTextSize(6);
  if (FPD < 2){
    int align;
    if (patch < 10) { // 1-9
      align = 48;
    } else if (patch < 100) { // 10-99
      align = 31;
    } else { // 100-128 use default
      align = 10;
    }
    display.setCursor(align,10);
    display.println(patch);
  } else if (FPD == 2){
    display.setCursor(10,10);
    display.println("SET");
  } else {
    display.setCursor(10,10);
    display.println("CLR");
  }
}

static void drawSubBox(const char* label)
{
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextSize(1);
  int len = strlen(label);

  display.setCursor(95-len*3,15);
  display.println(label);
}

void drawMenuCursor(byte itemNo, byte color){
  byte ymid = 15 + 9 * itemNo;
  display.drawTriangle(57, ymid,61, ymid+2,61, ymid-2, color);
}

//***********************************************************

#if defined(NURAD)
static int readTrills() {
  readSwitches();
  return LH1+2*LH2+4*LH3;
}
#else
static int readTrills() {
  readSwitches();
  return K5+2*K6+4*K7;
}
#endif
//***********************************************************

static void setFPS(int trills, uint16_t patchNum) {
  fastPatch[trills-1] = patchNum;
  writeSetting(FP1_ADDR+2*(trills-1), patchNum);
  FPD = 2;
}

//***********************************************************

static void clearFPS(int trills) {
  fastPatch[trills-1] = 0;
  writeSetting(FP1_ADDR+2*(trills-1),0);
  FPD = 3;
}


//***********************************************************
// Poly Play menu

const MenuEntryStateCh rotSubAMenu = { MenuType::EStateChange, "ROTATOR A", ROTA_MENU };
const MenuEntryStateCh rotSubBMenu = { MenuType::EStateChange, "ROTATOR B", ROTB_MENU };
const MenuEntryStateCh rotSubCMenu = { MenuType::EStateChange, "ROTATOR C", ROTC_MENU };

static void saveRotatorSetting(const Rotator * rotator, uint16_t settingAddr)
{
  int16_t stored;
  for(int i = 0; i < 4; ++i) {
    stored = readSetting(settingAddr+2*i);
    if(stored != rotator->rotations[i])
      writeSetting(settingAddr+2*i,(rotator->rotations[i]));
  }
}

static void rotatorSave(const MenuEntrySub& __unused sub) {
  saveRotatorSetting(&rotations_a, ROTN1_ADDR);
}

static void rotatorBSave(const MenuEntrySub& __unused sub) {
  saveRotatorSetting(&rotations_b, ROTB1_ADDR);
}

static void rotatorCSave(const MenuEntrySub& __unused sub) {
  saveRotatorSetting(&rotations_c, ROTC1_ADDR);
}


static void rotatorOptionGet(SubMenuRef sub, char *out, const char** __unused unit) {
  numToString((*sub.valuePtr) - 24, out, true);
}

static void parallelOptionGet(SubMenuRef __unused, char *out, const char** __unused unit) {
  numToString(rotations_a.parallel-24, out, true);
}

static void parallelBOptionGet(SubMenuRef __unused, char *out, const char** __unused unit) {
  numToString(rotations_b.parallel-24, out, true);
}

static void parallelCOptionGet(SubMenuRef __unused, char *out, const char** __unused unit) {
  numToString(rotations_c.parallel-24, out, true);
}

static void parallelSave(SubMenuRef __unused) {
  writeSetting(PARAL_ADDR, rotations_a.parallel);
}

static void parallelBSave(SubMenuRef __unused) {
  writeSetting(PARAB_ADDR, rotations_b.parallel);
}

static void parallelCSave(SubMenuRef __unused) {
  writeSetting(PARAC_ADDR, rotations_c.parallel);
}

const MenuEntrySub rotatorParaMenu  = {
  MenuType::ESub, "RTA PARAL", "SEMITONES", &rotations_a.parallel, 0, 48, MenuEntryFlags::ENone,
  parallelOptionGet, parallelSave, nullptr
};

const MenuEntrySub rotator1Menu  = {
  MenuType::ESub, "RTA ROT 1", "SEMITONES", &rotations_a.rotations[0], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotator2Menu  = {
  MenuType::ESub, "RTA ROT 2", "SEMITONES", &rotations_a.rotations[1], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotator3Menu  = {
  MenuType::ESub, "RTA ROT 3", "SEMITONES", &rotations_a.rotations[2], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotator4Menu  = {
  MenuType::ESub, "RTA ROT 4", "SEMITONES", &rotations_a.rotations[3], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotatorParaBMenu  = {
  MenuType::ESub, "RTB PARAL", "SEMITONES", &rotations_b.parallel, 0, 48, MenuEntryFlags::ENone,
  parallelBOptionGet, parallelBSave, nullptr
};

const MenuEntrySub rotatorB1Menu  = {
  MenuType::ESub, "RTB ROT 1", "SEMITONES", &rotations_b.rotations[0], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorBSave, nullptr
};

const MenuEntrySub rotatorB2Menu  = {
  MenuType::ESub, "RTB ROT 2", "SEMITONES", &rotations_b.rotations[1], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorBSave, nullptr
};

const MenuEntrySub rotatorB3Menu  = {
  MenuType::ESub, "RTB ROT 3", "SEMITONES", &rotations_b.rotations[2], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorBSave, nullptr
};

const MenuEntrySub rotatorB4Menu  = {
  MenuType::ESub, "RTB ROT 4", "SEMITONES", &rotations_b.rotations[3], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorBSave, nullptr
};
const MenuEntrySub rotatorParaCMenu  = {
  MenuType::ESub, "RTC PARAL", "SEMITONES", &rotations_c.parallel, 0, 48, MenuEntryFlags::ENone,
  parallelCOptionGet, parallelCSave, nullptr
};

const MenuEntrySub rotatorC1Menu  = {
  MenuType::ESub, "RTC ROT 1", "SEMITONES", &rotations_c.rotations[0], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorCSave, nullptr
};

const MenuEntrySub rotatorC2Menu  = {
  MenuType::ESub, "RTC ROT 2", "SEMITONES", &rotations_c.rotations[1], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorCSave, nullptr
};

const MenuEntrySub rotatorC3Menu  = {
  MenuType::ESub, "RTC ROT 3", "SEMITONES", &rotations_c.rotations[2], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorCSave, nullptr
};

const MenuEntrySub rotatorC4Menu  = {
  MenuType::ESub, "RTC ROT 4", "SEMITONES", &rotations_c.rotations[3], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorCSave, nullptr
};


static void rotatorPrioOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
    if (priority) strncpy(out, "LO", 4);
    else strncpy(out, "HI", 4);
}

static void rotatorPrioSave(SubMenuRef __unused) {
  writeSetting(PRIO_ADDR,priority);
}

const MenuEntrySub rotatorPrioMenu = {
  MenuType::ESub, "PRIORITY", "MONO PRIO", &priority, 0,1, MenuEntryFlags::EMenuEntryWrap,
  rotatorPrioOptionGet, rotatorPrioSave, nullptr,
};


static void fwcTypeOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
    switch (fwcType){
    case 0:
      strncpy(out, "6", 4);
      break;
    case 1:
      strncpy(out, "m6", 4);
      break;
    case 2:
      strncpy(out, "7", 4);
      break;
    case 3:
      strncpy(out, "m7", 4);
    }
}

static void fwcTypeSave(SubMenuRef __unused) {
  writeSetting(FWCTYPE_ADDR,fwcType);
}

const MenuEntrySub fwcTypeMenu = {
  MenuType::ESub, "FWC TYPE", "CHORD", &fwcType, 0,3, MenuEntryFlags::EMenuEntryWrap,
  fwcTypeOptionGet, fwcTypeSave, nullptr,
};

static void fwcLockHOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
    if (fwcLockH) strncpy(out, "ON", 4);
    else strncpy(out, "OFF", 4);
}

static void fwcLockHSave(SubMenuRef __unused) {
  writeSetting(FWCLCH_ADDR,fwcLockH);
}

const MenuEntrySub fwcLockHMenu = {
  MenuType::ESub, "FWC LOCKH", "LH MELODY", &fwcLockH, 0,1, MenuEntryFlags::EMenuEntryWrap,
  fwcLockHOptionGet, fwcLockHSave, nullptr,
};

static void fwcDrop2OptionGet(SubMenuRef __unused, char* out, const char** __unused) {
    if (fwcDrop2) strncpy(out, "ON", 4);
    else strncpy(out, "OFF", 4);
}

static void fwcDrop2Save(SubMenuRef __unused) {
  writeSetting(FWCDP2_ADDR,fwcDrop2);
}

const MenuEntrySub fwcDrop2Menu = {
  MenuType::ESub, "FWC DROP2", "DROP 2", &fwcDrop2, 0,1, MenuEntryFlags::EMenuEntryWrap,
  fwcDrop2OptionGet, fwcDrop2Save, nullptr,
};

static void hmzKeyOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
  switch (hmzKey){
    case 0:
      strncpy(out, "C", 4);
      break;
    case 1:
      strncpy(out, "C#", 4);
      break;
    case 2:
      strncpy(out, "D", 4);
      break;
    case 3:
      strncpy(out, "D#", 4);
      break;
    case 4:
      strncpy(out, "E", 4);
      break;
    case 5:
      strncpy(out, "F", 4);
      break;
    case 6:
      strncpy(out, "F#", 4);
      break;
    case 7:
      strncpy(out, "G", 4);
      break;
    case 8:
      strncpy(out, "G#", 4);
      break;
    case 9:
      strncpy(out, "A", 4);
      break;
    case 10:
      strncpy(out, "Bb", 4);
      break;
    case 11:
      strncpy(out, "B", 4);
      break;
  }
}

static void hmzKeySave(SubMenuRef __unused) {
  writeSetting(HMZKEY_ADDR,hmzKey);
}

const MenuEntrySub hmzKeyMenu = {
  MenuType::ESub, "HMZ KEY", "KEY PLAYED", &hmzKey, 0,11, MenuEntryFlags::EMenuEntryWrap,
  hmzKeyOptionGet, hmzKeySave, nullptr,
};


static void polySelectOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
  switch (polySelect){
    case 0:
      strncpy(out, "OFF", 4);
      break;
    case 1:
      strncpy(out, "MGR", 4);
      break;
    case 2:
      strncpy(out, "MGD", 4);
      break;
    case 3:
      strncpy(out, "MA9", 4);
      break;
    case 4:
      strncpy(out, "MND", 4);
      break;
    case 5:
      strncpy(out, "MNA", 4);
      break;
    case 6:
      strncpy(out, "MNH", 4);
      break;
    case 7:
      strncpy(out, "FWC", 4);
      break;
    case 8:
      strncpy(out, "RTA", 4);
      break;
    case 9:
      strncpy(out, "RTB", 4);
      break;
    case 10:
      strncpy(out, "RTC", 4);
      break;
  }
}

static void polySelectSave(SubMenuRef __unused) {
  writeSetting(POLYSEL_ADDR,polySelect);
}

const MenuEntrySub polySelectMenu = {
  MenuType::ESub, "POLY MODE", "SELECT", (uint16_t*)&polySelect, 0,10, MenuEntryFlags::EMenuEntryWrap, 
  polySelectOptionGet, polySelectSave, nullptr,
};

static void hmzLimitOptionGet(SubMenuRef sub, char *out, const char** __unused unit) {
  numToString((*sub.valuePtr), out, false);
}

static void hmzLimitSave(SubMenuRef __unused) {
  writeSetting(HMZLIMIT_ADDR,hmzLimit);
}

const MenuEntrySub hmzLimitMenu = {
  MenuType::ESub, "HMZ LIMIT", "VOICES", &hmzLimit, 2,5, MenuEntryFlags::EMenuEntryWrap,
  hmzLimitOptionGet, hmzLimitSave, nullptr,
};

static void otfKeyOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
    if (otfKey) strncpy(out, "ON", 4);
    else strncpy(out, "OFF", 4);
}

static void otfKeySave(SubMenuRef __unused) {
  writeSetting(OTFKEY_ADDR,otfKey);
}

const MenuEntrySub otfKeyMenu = {
  MenuType::ESub, "OTF KEY", "OTF KEYSW", &otfKey, 0,1, MenuEntryFlags::EMenuEntryWrap,
  otfKeyOptionGet, otfKeySave, nullptr,
};


const MenuEntry* rotSubAMenuEntries[] = {
  (MenuEntry*)&rotatorParaMenu,
  (MenuEntry*)&rotator1Menu,
  (MenuEntry*)&rotator2Menu,
  (MenuEntry*)&rotator3Menu,
  (MenuEntry*)&rotator4Menu
};

const MenuPage rotSubAMenuPage = {
  "ROTATOR A", 0, CursorIdx::ERotSubA, ROTATOR_MENU, ARR_LEN(rotSubAMenuEntries), rotSubAMenuEntries
};

const MenuEntry* rotSubBMenuEntries[] = {
  (MenuEntry*)&rotatorParaBMenu,
  (MenuEntry*)&rotatorB1Menu,
  (MenuEntry*)&rotatorB2Menu,
  (MenuEntry*)&rotatorB3Menu,
  (MenuEntry*)&rotatorB4Menu
};

const MenuPage rotSubBMenuPage = {
  "ROTATOR B", 0, CursorIdx::ERotSubB, ROTATOR_MENU, ARR_LEN(rotSubBMenuEntries), rotSubBMenuEntries
};

const MenuEntry* rotSubCMenuEntries[] = {
  (MenuEntry*)&rotatorParaCMenu,
  (MenuEntry*)&rotatorC1Menu,
  (MenuEntry*)&rotatorC2Menu,
  (MenuEntry*)&rotatorC3Menu,
  (MenuEntry*)&rotatorC4Menu
};

const MenuPage rotSubCMenuPage = {
  "ROTATOR C", 0, CursorIdx::ERotSubC, ROTATOR_MENU, ARR_LEN(rotSubCMenuEntries), rotSubCMenuEntries
};



const MenuEntry* rotatorMenuEntries[] = {
  (MenuEntry*)&polySelectMenu,
  (MenuEntry*)&hmzKeyMenu,
  (MenuEntry*)&otfKeyMenu,
  (MenuEntry*)&hmzLimitMenu,
  (MenuEntry*)&fwcTypeMenu,
  (MenuEntry*)&fwcLockHMenu,
  (MenuEntry*)&fwcDrop2Menu,
  (MenuEntry*)&rotatorPrioMenu,
/*(MenuEntry*)&rotatorParaMenu,
  (MenuEntry*)&rotator1Menu,
  (MenuEntry*)&rotator2Menu,
  (MenuEntry*)&rotator3Menu,
  (MenuEntry*)&rotator4Menu,
  (MenuEntry*)&rotatorParaBMenu,
  (MenuEntry*)&rotatorB1Menu,
  (MenuEntry*)&rotatorB2Menu,
  (MenuEntry*)&rotatorB3Menu,
  (MenuEntry*)&rotatorB4Menu,
  (MenuEntry*)&rotatorParaCMenu,
  (MenuEntry*)&rotatorC1Menu,
  (MenuEntry*)&rotatorC2Menu,
  (MenuEntry*)&rotatorC3Menu,
  (MenuEntry*)&rotatorC4Menu  */
  (MenuEntry*)&rotSubAMenu,
  (MenuEntry*)&rotSubBMenu,
  (MenuEntry*)&rotSubCMenu
};
/*
const MenuPage rotatorMenuPage = {
  "ROTATOR SETUP",
  EMenuPageRoot,
  CursorIdx::ERotator,
  DISPLAYOFF_IDL,
  ARR_LEN(rotatorMenuEntries), rotatorMenuEntries
};
*/
const MenuPage rotatorMenuPage = {
  "POLY PLAY SETUP",
  0,
  CursorIdx::ERotator,
  MAIN_MENU,
  ARR_LEN(rotatorMenuEntries), rotatorMenuEntries
};

//***********************************************************

// Main menu
/*
const MenuEntrySub transposeMenu = {
  MenuType::ESub, "TRANSPOSE", "TRANSPOSE", &transpose, 0, 24, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(transpose - 12, out, true);
  },
  [](const MenuEntrySub &sub) { writeSetting(TRANSP_ADDR,*sub.valuePtr); }
  , nullptr
};
*/
static void transposeOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
  switch (transpose){
    case 0:
      strncpy(out, "C>", 4);
      break;
    case 1:
      strncpy(out, "C#>", 4);
      break;
    case 2:
      strncpy(out, "D>", 4);
      break;
    case 3:
      strncpy(out, "D#>", 4);
      break;
    case 4:
      strncpy(out, "E>", 4);
      break;
    case 5:
      strncpy(out, "F>", 4);
      break;
    case 6:
      strncpy(out, "F#>", 4);
      break;
    case 7:
      strncpy(out, "G>", 4);
      break;
    case 8:
      strncpy(out, "G#>", 4);
      break;
    case 9:
      strncpy(out, "A>", 4);
      break;
    case 10:
      strncpy(out, "Bb>", 4);
      break;
    case 11:
      strncpy(out, "B>", 4);
      break;
    case 12:
      strncpy(out, ">C<", 4);
      break;
    case 13:
      strncpy(out, "<C#", 4);
      break;
    case 14:
      strncpy(out, "<D", 4);
      break;
    case 15:
      strncpy(out, "<D#", 4);
      break;
    case 16:
      strncpy(out, "<E", 4);
      break;
    case 17:
      strncpy(out, "<F", 4);
      break;
    case 18:
      strncpy(out, "<F#", 4);
      break;
    case 19:
      strncpy(out, "<G", 4);
      break;
    case 20:
      strncpy(out, "<G#", 4);
      break;
    case 21:
      strncpy(out, "<A", 4);
      break;
    case 22:
      strncpy(out, "<Bb", 4);
      break;
    case 23:
      strncpy(out, "<B", 4);
      break;
    case 24:
      strncpy(out, "<C", 4);
      break;
  }
}

static void transposeSave(SubMenuRef __unused) {
  writeSetting(TRANSP_ADDR,transpose);
}

const MenuEntrySub transposeMenu = {
  MenuType::ESub, "TRANSPOSE", "TRANSPOSE", &transpose, 0,24, MenuEntryFlags::ENone,
  transposeOptionGet, transposeSave, nullptr,
};





const MenuEntrySub octaveMenu = {
  MenuType::ESub, "OCTAVE", "OCTAVE", &octave, 0, 6, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(octave-3, out, true);
  },
  [](SubMenuRef __unused) { writeSetting(OCTAVE_ADDR,octave); }
  , nullptr
};

static void midiSaveFunc(const MenuEntrySub & __unused sub) { writeSetting(MIDI_ADDR, MIDIchannel); }

static void midiCustomDrawFunc(SubMenuRef __unused, char* __unused, const char** __unused) {
  char buff[7];
  numToString(MIDIchannel, buff);
  plotSubOption(buff);
  if (widiJumper && widiOn) {
    //indicate that widi board is enabled
    display.setTextSize(1);
    display.setCursor(100,51);
    display.print("WIDI");
  }
}

//***********************************************************

const MenuEntrySub legacyPBMenu = {
  MenuType::ESub, "LEGACY PB", "LEGACY PB", &legacy, 0, 1, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char ** __unused unit) {
    strncpy(out, legacy?"ON":"OFF", 4);
  }, [](const MenuEntrySub & __unused sub) {
    setBit(dipSwBits, DIPSW_LEGACY, legacy);
    writeSetting(DIPSW_BITS_ADDR,dipSwBits);
  }
  , nullptr
};

const MenuEntrySub legacyBRMenu = {
  MenuType::ESub, "LEGACY BR", "LEGACY BR", &legacyBrAct, 0, 1, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char ** __unused unit) {
    strncpy(out, legacyBrAct?"ON":"OFF", 4);
  }, [](const MenuEntrySub & __unused sub) {
    setBit(dipSwBits, DIPSW_LEGACYBRACT, legacyBrAct);
    writeSetting(DIPSW_BITS_ADDR, dipSwBits);
  }
  , nullptr
};

const MenuEntrySub gateOpenMenu = {
  MenuType::ESub, "GATE HOLD", "GATE HOLD", &gateOpenEnable, 0, 1, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char ** __unused unit) {
    strncpy(out, gateOpenEnable?"ON":"OFF", 4);
  }, [](const MenuEntrySub & __unused sub) {
    setBit(dipSwBits, DIPSW_GATEOPEN, gateOpenEnable);
    writeSetting(DIPSW_BITS_ADDR, dipSwBits);
  }
  , nullptr
};

const MenuEntrySub specialKeyMenu = {
  MenuType::ESub, "SPEC KEY", "SPEC KEY", &specialKeyEnable, 0, 1, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char ** __unused unit) {
    strncpy(out, specialKeyEnable?"ON":"OFF", 4);
  }, [](const MenuEntrySub & __unused sub) {
    setBit(dipSwBits, DIPSW_SPKEYENABLE, specialKeyEnable);
    writeSetting(DIPSW_BITS_ADDR, dipSwBits);
  }
  , nullptr
};

const MenuEntrySub trill3Menu = {
  MenuType::ESub, "3RD TRILL", "3RD TRILL", &trill3_interval, 3, 4, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(trill3_interval, out, true);
  },
  [](SubMenuRef __unused) { writeSetting(TRILL3_INTERVAL_ADDR, trill3_interval); }
  , nullptr
};

const MenuEntrySub bcasModeMenu = {
  MenuType::ESub, "BCAS MODE", "BCAS MODE", &bcasMode, 0, 1, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    strncpy(out, bcasMode?"ON":"OFF", 4);
  },
  [](SubMenuRef __unused) {
    setBit(dipSwBits, DIPSW_BCASMODE, bcasMode);
    writeSetting(DIPSW_BITS_ADDR, dipSwBits);
  }
  , nullptr
};

const MenuEntrySub dacModeMenu = {
  MenuType::ESub, "DAC OUT", "DAC OUT", &dacMode, 0, 1, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* dacModeLabels[] = { "BRTH", "PTCH"};
    strncpy(out, dacModeLabels[dacMode], 5);
  },
  [](SubMenuRef __unused) { writeSetting(DAC_MODE_ADDR, dacMode); }
  , nullptr
};

const MenuEntrySub fastBootMenu = {
  MenuType::ESub, "FAST BOOT", "FAST BOOT", &fastBoot, 0, 1, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    strncpy(out, fastBoot?"ON":"OFF", 4);
  },
  [](SubMenuRef __unused) {
    setBit(dipSwBits, DIPSW_FASTBOOT, fastBoot);
    writeSetting(DIPSW_BITS_ADDR, dipSwBits);
  }
  , nullptr
};

const MenuEntrySub cvTuneMenu = {
  MenuType::ESub, "CV TUNE", "TUNING", &cvTune, 1, 199, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(cvTune-100, out, true);
  },
  [](SubMenuRef __unused) { writeSetting(CVTUNE_ADDR,cvTune); }
  , nullptr
};

const MenuEntrySub cvScaleMenu = {
  MenuType::ESub, "CV SCALE", "SCALING", &cvScale, 1, 199, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(cvScale-100, out, true);
  },
  [](SubMenuRef __unused) { writeSetting(CVSCALE_ADDR,cvScale); }
  , nullptr
};

const MenuEntrySub cvEcVibMenu = {
  MenuType::ESub, "CV EC LFO",  "RATE", &cvVibRate, 0, 8, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(cvVibRate) numToString(cvVibRate, out);
    else strncpy(out, "OFF", 4);
  },
  [](const MenuEntrySub & __unused sub){
    if (readSetting(CVRATE_ADDR) != cvVibRate) {
      writeSetting(CVRATE_ADDR,cvVibRate);
    }
  }
  , nullptr
};

static uint16_t wireless_power=0;
static uint16_t wireless_channel=4;

const MenuEntrySub wlPowerMenu = {
  MenuType::ESub, "WL POWER", "WL POWER", &wireless_power, 0, 3, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(-6*wireless_power, out, true);
  },
  [](SubMenuRef __unused) { sendWLPower(wireless_power); }
  , nullptr
};

const MenuEntrySub wlChannelMenu = {
  MenuType::ESub, "WL CHAN", "WL CHAN", &wireless_channel, 4, 80, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(wireless_channel, out, false);
  },
  [](SubMenuRef __unused) { sendWLChannel(wireless_channel); }
  , nullptr
};

// Battery type menu
const MenuEntrySub batteryTypeMenu = {
  MenuType::ESub, "BAT TYPE", "BAT TYPE", &batteryType, 0, 2, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* breathCCMenuLabels[] = { "ALK", "NMH", "LIP" };
    strncpy(out, breathCCMenuLabels[batteryType], 4);
  },
  [](const MenuEntrySub & __unused sub){
    if (readSetting(BATTYPE_ADDR) != batteryType) {
      writeSetting(BATTYPE_ADDR,batteryType);
    }
  }
  , nullptr
};

#if defined(NURAD)
const MenuEntry* extrasMenuEntries[] = {
  (MenuEntry*)&legacyPBMenu,
  (MenuEntry*)&legacyBRMenu,
  (MenuEntry*)&gateOpenMenu,
  (MenuEntry*)&dacModeMenu,
  (MenuEntry*)&batteryTypeMenu,
  (MenuEntry*)&fastBootMenu,
  (MenuEntry*)&cvTuneMenu,
  (MenuEntry*)&cvScaleMenu,
  (MenuEntry*)&cvEcVibMenu,
  (MenuEntry*)&wlPowerMenu,
  (MenuEntry*)&wlChannelMenu,
};
#else
const MenuEntry* extrasMenuEntries[] = {
  (MenuEntry*)&legacyPBMenu,
  (MenuEntry*)&legacyBRMenu,
  (MenuEntry*)&gateOpenMenu,
  (MenuEntry*)&trill3Menu,
  (MenuEntry*)&bcasModeMenu,
  (MenuEntry*)&dacModeMenu,
  (MenuEntry*)&batteryTypeMenu,
  (MenuEntry*)&fastBootMenu,
  (MenuEntry*)&cvTuneMenu,
  (MenuEntry*)&cvScaleMenu,
  (MenuEntry*)&cvEcVibMenu,
  (MenuEntry*)&wlPowerMenu,
  (MenuEntry*)&wlChannelMenu,
};
#endif

const MenuPage extrasMenuPage = {
  "EXTRAS",
  0,
  CursorIdx::EExtras,
  MAIN_MENU,
  ARR_LEN(extrasMenuEntries), extrasMenuEntries
};

static bool midiEnterHandlerFunc() {

  //this switching is removed due to new breathInterval setting
  readSwitches();
  if (pinkyKey && widiJumper){
    widiOn = !widiOn;
    dipSwBits = dipSwBits ^ (1<<3);
    writeSetting(DIPSW_BITS_ADDR,dipSwBits);
    if (widiJumper && widiOn) digitalWrite(widiPowerPin, HIGH); else digitalWrite(widiPowerPin, LOW);
    return false;
  } else {
    writeSetting(MIDI_ADDR, MIDIchannel);
    return true;
  }

  //writeSetting(MIDI_ADDR, MIDIchannel);
  //return true;
}

const MenuEntrySub midiMenu = {
  MenuType::ESub, "MIDI CH", "MIDI CHNL", &MIDIchannel, 1, 16, EMenuEntryCustom | EMenuEntryEnterHandler,
  midiCustomDrawFunc, midiSaveFunc, midiEnterHandlerFunc
};

const MenuEntryStateCh adjustMenu     = { MenuType::EStateChange, "ADJUST", ADJUST_MENU };
const MenuEntryStateCh breathMenu     = { MenuType::EStateChange, "SETUP BR", SETUP_BR_MENU };
const MenuEntryStateCh controlMenu    = { MenuType::EStateChange, "SETUP CTL", SETUP_CT_MENU };
const MenuEntryStateCh rotatorMenu    = { MenuType::EStateChange, "POLY PLAY", ROTATOR_MENU };
const MenuEntryStateCh extrasMenu     = { MenuType::EStateChange, "EXTRAS", EXTRAS_MENU };
const MenuEntryStateCh aboutMenu      = { MenuType::EStateChange, "ABOUT", ABOUT_MENU };

const MenuEntry* mainMenuEntries[] = {
  (MenuEntry*)&transposeMenu,
  (MenuEntry*)&octaveMenu,
  (MenuEntry*)&midiMenu,
  (MenuEntry*)&rotatorMenu,
  (MenuEntry*)&breathMenu,
  (MenuEntry*)&controlMenu,
  (MenuEntry*)&adjustMenu,
  (MenuEntry*)&extrasMenu,
  (MenuEntry*)&aboutMenu,
};

const MenuPage mainMenuPage = {
  (const char*)mainTitleGetStr,
  EMenuPageRoot | EMenuCustomTitle,
  CursorIdx::EMain,
  DISPLAYOFF_IDL,
  ARR_LEN(mainMenuEntries), mainMenuEntries
};


// Breath menu
const MenuEntrySub breathCCMenu = {
  MenuType::ESub, "BRTH CC A", "BRTH CC A", &breathCC, 0, 10, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* breathCCMenuLabels[] = { "OFF", "MW", "BR", "VL", "EX", "MW+",
                                            "BR+", "VL+", "EX+", "CF", "UNO" };
    strncpy(out, breathCCMenuLabels[breathCC], 4);
  },
  [](const MenuEntrySub & __unused sub){
    if (readSetting(BREATH_CC_ADDR) != breathCC) {
      writeSetting(BREATH_CC_ADDR,breathCC);
      midiReset();
    }
  }
  , nullptr
};

const MenuEntrySub breathCC2Menu = {
  MenuType::ESub, "BRTH CC B",  "BRTH CC B", &breathCC2, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(breathCC2) numToString(breathCC2, out);
    else strncpy(out, "OFF", 4);
  },
  [](const MenuEntrySub & __unused sub){
    if (readSetting(BREATH_CC2_ADDR) != breathCC2) {
      writeSetting(BREATH_CC2_ADDR,breathCC2);
      midiReset();
    }
  }
  , nullptr
};

const MenuEntrySub breathCC2RiseMenu = {
  MenuType::ESub, "CC B RISE", "CC B RISE", &breathCC2Rise, 1, 10, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char *out, const char** label) {
    numToString(breathCC2Rise, out);
    *label = "X";
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(BREATH_CC2_RISE_ADDR,breathCC2Rise); }
  , nullptr
};


const MenuEntrySub breathATMenu = {
  MenuType::ESub, "BREATH AT", "BREATH AT", &breathAT, 0, 1, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char ** __unused unit) {
    strncpy(out, breathAT?"ON":"OFF", 4);
  }, [](const MenuEntrySub & __unused sub) {
    if (readSetting(BREATH_AT_ADDR) != breathAT) {
      writeSetting(BREATH_AT_ADDR, breathAT);
      midiReset();
    }
  }
  , nullptr
};

const MenuEntrySub brHarmonicsMenu = {
  MenuType::ESub, "BRTH HARM",  "HARM RANGE", &brHarmSetting, 0, 6, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(brHarmSetting) numToString(brHarmSetting, out);
    else strncpy(out, "OFF", 4);
  },
[](const MenuEntrySub & __unused sub) { writeSetting(BRHARMSET_ADDR,brHarmSetting); }
  , nullptr
};

const MenuEntrySub brHarmSelectMenu = {
  MenuType::ESub, "BR HM SEL", "SERIES", &brHarmSelect, 0, 3, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* brHarmSelectMenuLabels[] = { "HM1", "HM2", "5TH", "OCT" };
    strncpy(out, brHarmSelectMenuLabels[brHarmSelect], 4);
  },
  [](const MenuEntrySub & __unused sub){
    if (readSetting(BRHARMSEL_ADDR) != brHarmSelect) {
      writeSetting(BRHARMSEL_ADDR,brHarmSelect);
    }
  }
  , nullptr
};

const MenuEntrySub velocityMenu = {
  MenuType::ESub, "VELOCITY",  "VELOCITY", &velocity, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(velocity) numToString(velocity, out);
    else strncpy(out, "DYN", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VELOCITY_ADDR,velocity); }
  , nullptr
};

static void curveCustomDraw(SubMenuRef __unused, char* __unused out, const char** __unused unit) {
  const char* curveMenuLabels[] = {"-4", "-3", "-2", "-1", "LIN", "+1", "+2",
                                        "+3", "+4", "S1", "S2", "Z1", "Z2" };
  int y0 = 0, x0 = 0;
  int scale = ((1<<14)-1)/60;
  for(int x = x0; x < 60; x+=1) {
    int y = multiMap(x*scale, curveIn, curves[curve], 17);
    y = (y*37) / ((1<<14)-1);
    display.drawLine(x0 + 65, 60 - y0, x + 65, 60 - y, WHITE);
    x0 = x; y0 = y;
  }
  display.setCursor(125 - 3*6, 60-8 );
  display.setTextSize(0);
  display.print(curveMenuLabels[curve]);
}

static void curveWriteSettings(const MenuEntrySub & __unused sub){ writeSetting(BREATHCURVE_ADDR,curve); }

const MenuEntrySub curveMenu = {
  MenuType::ESub, "CURVE", "CURVE", &curve, 0, 12, MenuEntryFlags::EMenuEntryWrap | MenuEntryFlags::EMenuEntryCustom,
  curveCustomDraw, curveWriteSettings, nullptr
};

const MenuEntrySub velSmpDlMenu = {
  MenuType::ESub, "VEL DELAY", "VEL DELAY", &velSmpDl, 0, 30, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char *out, const char** label) {
    if (velSmpDl) {
      numToString(velSmpDl, out);
      *label = "ms";
    } else strncpy(out, "OFF", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VEL_SMP_DL_ADDR,velSmpDl); }
  , nullptr
};

const MenuEntrySub velBiasMenu = {
  MenuType::ESub, "VEL BOOST", "VEL BOOST", &velBias, 0, 9, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if (velBias) numToString(velBias, out);
    else strncpy(out, "OFF", 4);
  },
  [](SubMenuRef __unused){ writeSetting(VEL_BIAS_ADDR,velBias); }
  , nullptr
};

const MenuEntrySub breathIntervalMenu = {
  MenuType::ESub, "BR INTERV", "CC INTERV", &breathInterval, 3, 15, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(breathInterval, out, false);
  },
  [](SubMenuRef __unused) { writeSetting(BRINTERV_ADDR, breathInterval); }
  , nullptr
};

const MenuEntry* breathMenuEntries[] = {
  (MenuEntry*)&breathCCMenu,
  (MenuEntry*)&breathCC2Menu,
  (MenuEntry*)&breathCC2RiseMenu,
  (MenuEntry*)&breathATMenu,
  (MenuEntry*)&velocityMenu,
  (MenuEntry*)&curveMenu,
  (MenuEntry*)&velSmpDlMenu,
  (MenuEntry*)&velBiasMenu,
  //(MenuEntry*)&brHarmonicsMenu,
  //(MenuEntry*)&brHarmSelectMenu,
  (MenuEntry*)&breathIntervalMenu
};

const MenuPage breathMenuPage = {
  "SETUP BREATH",
  0,
  CursorIdx::EBreath,
  MAIN_MENU,
  ARR_LEN(breathMenuEntries), breathMenuEntries
};

//***********************************************************
// Control menu

const MenuEntrySub biteCtlMenu = {
  MenuType::ESub, "BITE CTL", "BITE DEST", &biteControl, 0, 3, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char ** __unused unit) {
    const char* labs[] = { "OFF", "VIB", "GLD", "CC" };
    strncpy(out, labs[biteControl], 4);
  },
  [](SubMenuRef __unused sub) { writeSetting(BITECTL_ADDR,biteControl); }
  , nullptr
};

const MenuEntrySub biteCCMenu = {
  MenuType::ESub, "BITE CC",  "CC NUMBER", &biteCC, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(biteCC, out);
  },
[](const MenuEntrySub & __unused sub) { writeSetting(BITECC_ADDR,biteCC); }
  , nullptr
};

const MenuEntrySub leverCtlMenu = {
  MenuType::ESub, "LEVER CTL", "LEVER DEST", &leverControl, 0, 3, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char ** __unused unit) {
    const char* labs[] = { "OFF", "VIB", "GLD", "CC" };
    strncpy(out, labs[leverControl], 4);
  },
  [](SubMenuRef __unused sub) { writeSetting(LEVERCTL_ADDR,leverControl); }
  , nullptr
};

const MenuEntrySub leverCCMenu = {
  MenuType::ESub, "LEVER CC",  "CC NUMBER", &leverCC, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(leverCC, out);
  },
[](const MenuEntrySub & __unused sub) { writeSetting(LEVERCC_ADDR,leverCC); }
  , nullptr
};

const MenuEntrySub portMenu = {
  MenuType::ESub, "GLIDE MOD", "PORT/GLD", &portamento, 0, 5, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char ** __unused unit) {
    const char* labs[] = { "OFF", "ON", "SW", "SEL", "SEE", "SWO" };
    strncpy(out, labs[portamento], 4);
  },
  [](SubMenuRef __unused sub) { writeSetting(PORTAM_ADDR,portamento); }
  , nullptr
};


const MenuEntrySub portLimitMenu = {
  MenuType::ESub, "GLIDE LMT",  "MAX LEVEL", &portLimit, 1, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(portLimit, out);
  },
  [](SubMenuRef __unused sub) { writeSetting(PORTLIMIT_ADDR,portLimit); }
  , nullptr
};



const MenuEntrySub pitchBendMenu = {
  MenuType::ESub, "PITCHBEND", "PITCHBEND", &PBdepth, 0, 12, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(PBdepth) {
      memcpy(out, "1/", 2);
      numToString(PBdepth, &out[2]);
    }
    else strncpy(out, "OFF", 4);
  },
  [](SubMenuRef __unused){ writeSetting(PB_ADDR,PBdepth); }
  , nullptr
};

const MenuEntrySub extraMenu = {
  MenuType::ESub, "EXCT CC A", "EXCT CC A", &extraCT, 0,4, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char** __unused unit) {
    const char* extraMenuLabels[] = { "OFF", "MW", "FP", "CF", "SP" };
    strncpy(out, extraMenuLabels[extraCT], 12);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(EXTRA_ADDR,extraCT); }
  , nullptr
};

const MenuEntrySub extraCC2Menu = {
  MenuType::ESub, "EXCT CC B",  "EXCT CC B", &extraCT2, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(extraCT2) numToString(extraCT2, out);
    else strncpy(out, "OFF", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(EXTRA2_ADDR,extraCT2); }
  , nullptr
};

const MenuEntrySub harmonicsMenu = {
  MenuType::ESub, "EXCT HARM",  "HARM RANGE", &harmSetting, 0, 6, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(harmSetting) numToString(harmSetting, out);
    else strncpy(out, "OFF", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(HARMSET_ADDR,harmSetting); }
  , nullptr
};

const MenuEntrySub harmSelectMenu = {
  MenuType::ESub, "HARM SEL", "SERIES", &harmSelect, 0, 7, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* harmSelectMenuLabels[] = { "HM1", "HM2", "5TH", "OCT", "H1R", "H2R", "5TR", "OCR" };
    strncpy(out, harmSelectMenuLabels[harmSelect], 4);
  },
  [](const MenuEntrySub & __unused sub){
    if (readSetting(HARMSEL_ADDR) != harmSelect) {
      writeSetting(HARMSEL_ADDR,harmSelect);
    }
  }
  , nullptr
};

const MenuEntryStateCh vibratoSubMenu = { MenuType::EStateChange, "VIBRATO", VIBRATO_MENU };

const MenuEntrySub deglitchMenu = {
  MenuType::ESub, "DEGLITCH", "DEGLITCH", &deglitch, 0, 70, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** label) {
    if(deglitch) {
      numToString(deglitch, textBuffer);
      *label = "ms";
    } else
      strncpy(textBuffer, "OFF", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(DEGLITCH_ADDR,deglitch); }
  , nullptr
};

#if defined(NURAD)
const MenuEntrySub pinkyMenu = {
  MenuType::ESub, "MOD KEY", "MOD KEY", &pinkySetting, 0, 31, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
#else
const MenuEntrySub pinkyMenu = {
  MenuType::ESub, "PINKY KEY", "PINKY KEY", &pinkySetting, 0, 31, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
#endif
    if (pinkySetting == PBD)
      strncpy(textBuffer, "PBD", 4);
    else if (pinkySetting == EC2)
      strncpy(textBuffer, "ECB", 4);
    else if (pinkySetting == ECSW)
      strncpy(textBuffer, "ECS", 4);
    else if (pinkySetting == LVL)
      strncpy(textBuffer, "LVL", 4);
    else if (pinkySetting == LVLP)
      strncpy(textBuffer, "LVP", 4);
    else if (pinkySetting == GLD)
      strncpy(textBuffer, "GLD", 4);
    else if (pinkySetting == ECH)
      strncpy(textBuffer, "ECH", 4);
    else if (pinkySetting == QTN)
      strncpy(textBuffer, "QTN", 4);
    else
      numToString(pinkySetting-12, textBuffer, true);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(PINKY_KEY_ADDR,pinkySetting); }
  , nullptr
};

const MenuEntrySub lvlCtrlCCMenu = {
  MenuType::ESub, "LEVEL CC",  "LEVEL CC", &levelCC, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(levelCC) numToString(levelCC, out);
    else strncpy(out, "AT", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(LEVEL_CC_ADDR,levelCC); }
  , nullptr
};

#if defined(NURAD)
const MenuEntrySub fingeringMenu = {
  MenuType::ESub, "FINGERING", "FINGERING", &fingering, 0, 4, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char ** __unused unit) {
    const char* labs[] = { "EWI", "EWX", "SAX", "EVI", "EVR" };
    strncpy(out, labs[fingering], 4);
  },
  [](SubMenuRef __unused sub) { writeSetting(FINGER_ADDR,fingering); }
  , nullptr
};
#else
const MenuEntrySub fingeringMenu = {
  MenuType::ESub, "FINGERING", "FINGERING", &fingering, 0, 3, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char ** __unused unit) {
    const char* labs[] = { "EVI", "EVR", "TPT", "HRN" };
    strncpy(out, labs[fingering], 4);
  },
  [](SubMenuRef __unused sub) { writeSetting(FINGER_ADDR,fingering); }
  , nullptr
};
#endif

const MenuEntrySub rollerMenu = {
  MenuType::ESub, "ROLLRMODE",  "ROLLRMODE", &rollerMode, 0, 3, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(rollerMode+1, out);
  },
[](const MenuEntrySub & __unused sub) { writeSetting(ROLLER_ADDR,rollerMode); }
  , nullptr
};


const MenuEntrySub lpinky3Menu = {
  MenuType::ESub, "EXTRA KEY", "EXTRA PKEY", &lpinky3, 0, 25, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
    if (lpinky3 == 0)
      strncpy(textBuffer, "OFF", 4);
    else if (lpinky3 == MOD)
      strncpy(textBuffer, "MOD", 4);
    else
      numToString(lpinky3-13, textBuffer, true);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(LPINKY3_ADDR,lpinky3); }
  , nullptr
};

#if defined(NURAD)
const MenuEntry* controlMenuEntries[] = {
  (MenuEntry*)&biteCtlMenu,
  (MenuEntry*)&biteCCMenu,
  (MenuEntry*)&leverCtlMenu,
  (MenuEntry*)&leverCCMenu,
  (MenuEntry*)&portMenu,
  (MenuEntry*)&portLimitMenu,
  (MenuEntry*)&vibratoSubMenu,
  (MenuEntry*)&extraMenu,
  (MenuEntry*)&extraCC2Menu,
  (MenuEntry*)&harmonicsMenu,
  (MenuEntry*)&harmSelectMenu,
  (MenuEntry*)&deglitchMenu,
  (MenuEntry*)&pinkyMenu,
  (MenuEntry*)&lvlCtrlCCMenu,
  (MenuEntry*)&lpinky3Menu,
  (MenuEntry*)&fingeringMenu,
  (MenuEntry*)&rollerMenu,
  (MenuEntry*)&pitchBendMenu
};
#else
const MenuEntry* controlMenuEntries[] = {
  (MenuEntry*)&biteCtlMenu,
  (MenuEntry*)&biteCCMenu,
  (MenuEntry*)&leverCtlMenu,
  (MenuEntry*)&leverCCMenu,
  (MenuEntry*)&portMenu,
  (MenuEntry*)&portLimitMenu,
  (MenuEntry*)&vibratoSubMenu,
  (MenuEntry*)&extraMenu,
  (MenuEntry*)&extraCC2Menu,
  (MenuEntry*)&harmonicsMenu,
  (MenuEntry*)&harmSelectMenu,
  (MenuEntry*)&deglitchMenu,
  (MenuEntry*)&pinkyMenu,
  (MenuEntry*)&lvlCtrlCCMenu,
  (MenuEntry*)&fingeringMenu,
  (MenuEntry*)&rollerMenu,
  (MenuEntry*)&pitchBendMenu
};
#endif


const MenuPage controlMenuPage = {
  "SETUP CTRLS",
  0,
  CursorIdx::EControl,
  MAIN_MENU,
  ARR_LEN(controlMenuEntries), controlMenuEntries
};


//***********************************************************
// Vibrato menu

static void vibGetStr(SubMenuRef __unused, char* textBuffer, const char** __unused unit) {
  if(vibrato)
    numToString(vibrato, textBuffer);
  else
    strncpy(textBuffer, "OFF", 4);
}

static void vibStore(const MenuEntrySub & __unused sub) {
  writeSetting(VIBRATO_ADDR,vibrato);
}

const MenuEntrySub vibDepthMenu = {
  MenuType::ESub, "DEPTH", "LEVEL", &vibrato, 0, 9, MenuEntryFlags::ENone,
  vibGetStr,
  vibStore,
  nullptr
};

const MenuEntrySub vibRetnMenu = {
  MenuType::ESub, "RETURN", "LEVEL", &vibRetn, 0, 4, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* textBuffer, const char** __unused unit) {
    numToString(vibRetn, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_RETN_ADDR,vibRetn); }
  , nullptr
};

const MenuEntrySub vibSenseMenu = {
  MenuType::ESub, "SENSE LVR", "LEVEL", &vibSens, 1, 12, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
    numToString(vibSens, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_SENS_ADDR,vibSens); }
  , nullptr
};

const MenuEntrySub vibSquelchMenu = {
  MenuType::ESub, "SQUELCH L", "LEVEL", &vibSquelch, 1, 30, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* textBuffer, const char** __unused unit) {
    numToString(vibSquelch, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_SQUELCH_ADDR,vibSquelch); }
  , nullptr
};

const MenuEntrySub vibSenseBiteMenu = {
  MenuType::ESub, "SENSE BTE", "LEVEL", &vibSensBite, 1, 17, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
    numToString(vibSensBite, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_SENS_BITE_ADDR,vibSensBite); }
  , nullptr
};

const MenuEntrySub vibSquelchBiteMenu = {
  MenuType::ESub, "SQUELCH B", "LEVEL", &vibSquelchBite, 1, 30, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* textBuffer, const char** __unused unit) {
    numToString(vibSquelchBite, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_SQUELCH_BITE_ADDR,vibSquelchBite); }
  , nullptr
};

const MenuEntrySub vibControlMenu = {
  MenuType::ESub, "CONTROL", "CONTROL", &vibControl , 0, 2, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if (2 == vibControl)
      strncpy(out, "BTH", 4);
    else if (1 == vibControl)
      strncpy(out, "BIT", 4);
    else
      strncpy(out, "LVR", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_CONTROL_ADDR,vibControl); }
  , nullptr
};

const MenuEntrySub vibDirMenu = {
  MenuType::ESub, "DIRECTION", "DIRECTION", &vibDirection , 0, 1, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if (DNWD == vibDirection)
      strncpy(out, "NRM", 4);
    else
      strncpy(out, "REV", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_DIRECTION_ADDR,vibDirection); }
  , nullptr
};

const MenuEntry* vibratorMenuEntries[] = {
    //(MenuEntry*)&vibControlMenu,
    (MenuEntry*)&vibDepthMenu,
    (MenuEntry*)&vibRetnMenu,
    (MenuEntry*)&vibDirMenu,
    (MenuEntry*)&vibSenseMenu,
    (MenuEntry*)&vibSquelchMenu,
    (MenuEntry*)&vibSenseBiteMenu,
    (MenuEntry*)&vibSquelchBiteMenu,
};

const MenuPage vibratoMenuPage = {
  "VIBRATO", 0, CursorIdx::EVibrato, SETUP_CT_MENU, ARR_LEN(vibratorMenuEntries), vibratorMenuEntries
};


//***********************************************************

static bool patchPageUpdate(KeyState& __unused input, uint32_t __unused timeNow);
static bool idlePageUpdate(KeyState& __unused input, uint32_t __unused timeNow);

const MenuPageCustom adjustMenuPage = { nullptr, EMenuPageCustom, adjustPageUpdate };
const MenuPageCustom patchMenuPage =  { nullptr, EMenuPageCustom, patchPageUpdate };
const MenuPageCustom idleMenuPage =   { nullptr, EMenuPageCustom, idlePageUpdate };

const MenuPageCustom aboutMenuPage =  { nullptr, EMenuPageCustom,
  [](KeyState& __unused input, uint32_t __unused timeNow) -> bool {
    static uint32_t timer = 0;
    if(stateFirstRun) {
      display.clearDisplay();
      timer = timeNow + 3500;
      stateFirstRun = 0;
      display.setCursor(49,0);
      display.setTextColor(WHITE);
      display.setTextSize(0);
      #if defined(NURAD)
      display.println("NuRAD");
      #else
      display.println("NuEVI");
      #endif
      display.setCursor(16,12);
      display.print("firmware v.");
      display.println(FIRMWARE_VERSION);
      int vMeterReading = battAvg;
      int voltage = map(vMeterReading,2200,3060,36,50);
      display.setCursor(16,32);
      if (vMeterReading > 3020){
        drawFlash(4,34);
        display.print("USB power");
        display.setCursor(16,42);
        display.print(voltage/10);
        display.print(".");
        display.print(voltage%10);
        display.print("v");
      } else {
        int bar = drawBatt(4,34);
        if (0 == batteryType) {
          display.println("Alkaline battery");
          display.setCursor(16,42);
          display.print(bar);
          display.print("0 % at ");
          display.print(voltage/10);
          display.print(".");
          display.print(voltage%10);
          display.print("v");
        } else if (1 == batteryType) {
          display.println("NiMH battery");
          display.setCursor(16,42);
          display.print(bar);
          display.print("0 % at ");
          display.print(voltage/10);
          display.print(".");
          display.print(voltage%10);
          display.print("v");
        } else if (2 == batteryType) {
          display.println("LiPo battery");
          display.setCursor(16,42);
          display.print(bar);
          display.print("0 % at ");
          display.print(voltage/10);
          display.print(".");
          display.print(voltage%10);
          display.print("v");
        }
      }


      return true;
    } else {
      if(timeNow >= timer) {
        menuState = MAIN_MENU;
        stateFirstRun = 1;
      }
      return false;
    }
  }
};


//***********************************************************

static bool selectMenuOption(const MenuPage *page) {
  int cursorPosition = cursors[page->cursor];
  const MenuEntry* menuEntry = page->entries[cursorPosition];
  cursorBlinkTime = millis();

  switch(menuEntry->type) {
    case MenuType::ESub:
      activeSub[page->cursor] = cursorPosition+1;
      drawMenuCursor(cursorPosition-offsets[page->cursor], WHITE);
      drawSubBox( ((const MenuEntrySub*)menuEntry)->subTitle);
      drawSubMenu(page);
      return true;

    case MenuType::EStateChange:
      menuState= ((const MenuEntryStateCh*)menuEntry)->state;
      stateFirstRun = 1;
      break;
  }

  return false;
}

//***********************************************************

static bool updateSubMenu(const MenuPage *page, KeyState &input, uint32_t timeNow) {

  bool redraw = false;
  bool redrawSubValue = false;
  if (input.changed) {
    int current_sub = activeSub[page->cursor] -1;

    if( current_sub < 0)
      return false;

    auto sub = (const MenuEntrySub*)page->entries[current_sub];
    uint16_t currentVal = *sub->valuePtr;

    switch (input.current){
      case BTN_DOWN:
        if(currentVal > sub->min) {
          currentVal -= 1;
        } else if(sub->flags & MenuEntryFlags::EMenuEntryWrap) {
          currentVal = sub->max;
        }
        break;

      case BTN_UP:
        if(currentVal < sub->max) {
          currentVal += 1;
        } else if(sub->flags & MenuEntryFlags::EMenuEntryWrap) {
          currentVal = sub->min;
        }
        break;

      case BTN_ENTER:
        if(sub->flags & EMenuEntryEnterHandler) {
          bool result = sub->onEnterFunc();
          if(result) {
            activeSub[page->cursor] = 0;
          }
        } else {
          activeSub[page->cursor] = 0;
          sub->applyFunc(*sub);
        }
        break;

      case BTN_MENU:
        activeSub[page->cursor] = 0;
        sub->applyFunc(*sub);
        break;
    }
    *sub->valuePtr = currentVal;
    redrawSubValue = true;
  } else {
    redraw = updateSubMenuCursor( page, timeNow );
  }

  if(redrawSubValue) {
    clearSubValue();
    redraw |= drawSubMenu(page);
    cursorNow = BLACK;
    cursorBlinkTime = timeNow;
  }

  return redraw;
}

static bool updateMenuPage(const MenuPage *page, KeyState &input, uint32_t timeNow) {
  byte cursorPos = cursors[page->cursor];
  byte newPos = cursorPos;
  bool redraw = false;

  if (input.changed) {
    int lastEntry = page->numEntries-1;
    switch (input.current) {
      case BTN_DOWN:
        if (cursorPos < lastEntry)
          newPos = cursorPos+1;
        break;

      case BTN_ENTER:
        redraw |= selectMenuOption(page);
        break;

      case BTN_UP:
        if (cursorPos > 0)
          newPos = cursorPos-1;
        break;

      case BTN_MENU:
        menuState= page->parentPage;
        stateFirstRun = 1;
        break;
    }

    if(newPos != cursorPos) {

      int offset = offsets[page->cursor];
      drawMenuCursor(cursorPos-offset, BLACK); // Clear old cursor

      if(page->numEntries >= MENU_NUM_ROWS) {
        // Handle scrolling..
        if((newPos - offset) > (MENU_NUM_ROWS-2) ) {
          offset = newPos - (MENU_NUM_ROWS-2);
        } else if( (newPos - offset) < 1) {
          offset = newPos - 1;
        }

        offset = constrain(offset, 0, page->numEntries - MENU_NUM_ROWS);

        if( offset != offsets[page->cursor]) {
          offsets[page->cursor] = offset;
          plotMenuEntries(page, true);
        }
      }

      drawMenuCursor(newPos-offset, WHITE);
      cursorNow = BLACK;
      clearSub();
      redraw = true;
    }
    cursors[page->cursor] = newPos;
  } else if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    // Only need to update cursor blink if no buttons were pressed
    if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
    drawMenuCursor(cursorPos-offsets[page->cursor], cursorNow);
    redraw = true;
    cursorBlinkTime = timeNow;
  }

  return redraw;
}

static bool updatePage(const MenuPage *page, KeyState &input, uint32_t timeNow) {

  display.setTextColor(WHITE);

  if(page->flags & EMenuPageCustom) {
    const MenuPageCustom* custom = (const MenuPageCustom*)page;
    return custom->menuUpdateFunc(input, timeNow);
  }

  bool redraw = stateFirstRun;

  if (stateFirstRun) {
    drawMenu(page);
    stateFirstRun = 0;
  }
  if (activeSub[page->cursor]) {
    redraw = updateSubMenu(page, input, timeNow);
  } else {
    redraw = updateMenuPage(page, input, timeNow);

    if((page->flags & EMenuPageRoot) && input.changed) {
        int trills = readTrills();
        switch (input.current) {
          case BTN_MENU+BTN_ENTER:
            if (trills) {
              menuState = PATCH_VIEW;
              stateFirstRun = 1;
              setFPS(trills, patch);
            }
            break;

          case BTN_MENU+BTN_UP:
            if (trills) {
              menuState = PATCH_VIEW;
              stateFirstRun = 1;
              clearFPS(trills);
            }
            break;
          default: break;
        }
    }
  }

  return redraw;
}


//***********************************************************


static bool updateSensorPixelsFlag = false;
void drawSensorPixels() {
  updateSensorPixelsFlag = true;
}

//***********************************************************


bool adjustPageUpdate(KeyState &input, uint32_t timeNow) {
  // This is a hack to update touch_Thr is it was changed..
  int old_thr = ctouchThrVal;
  int result = updateAdjustMenu(timeNow, input, stateFirstRun, updateSensorPixelsFlag);
  bool redraw = false;

  updateSensorPixelsFlag = false;
  stateFirstRun = 0;

  if(result < 0) {
    // Go back to main menu
    menuState= MAIN_MENU;
    stateFirstRun = true;
  } else {
    redraw = result;
  }

  if( old_thr != ctouchThrVal) {
    touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
  }
  return redraw;
}


static bool patchPageUpdate(KeyState& input, uint32_t timeNow) {
  bool redraw = stateFirstRun;

  if (stateFirstRun) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    drawPatchView();
    patchViewTime = timeNow;
    stateFirstRun = 0;
  }
  if ((timeNow - patchViewTime) > patchViewTimeUp) {
    menuState= DISPLAYOFF_IDL;
    stateFirstRun = 1;
    doPatchUpdate = 1;
    FPD = 0;
    writeSetting(PATCH_ADDR,patch);
  }
  if (input.changed) {
    patchViewTime = timeNow;
    int trills = readTrills();
    switch (input.current){
      case BTN_DOWN:
      // fallthrough
      case BTN_UP:
        if (trills && (fastPatch[trills-1] > 0)){
          patch = fastPatch[trills-1];
          activePatch = 0;
          doPatchUpdate = 1;
          FPD = 1;
          writeSetting(PATCH_ADDR,patch);
        } else if (!trills){
          patch = (((patch-1u) + ((input.current == BTN_UP)?1u:-1u))&127u) + 1u;
          activePatch = 0;
          doPatchUpdate = 1;
          FPD = 0;
        }
        drawPatchView();
        redraw = true;
        break;

      case BTN_ENTER:
        if (trills && (fastPatch[trills-1] > 0)){
          patch = fastPatch[trills-1];
          activePatch = 0;
          doPatchUpdate = 1;
          FPD = 1;
          drawPatchView();
          redraw = true;
        }
        break;


      case BTN_MENU:
        if (FPD < 2){
          menuState= DISPLAYOFF_IDL;
          stateFirstRun = 1;
          doPatchUpdate = 1;
        }
        writeSetting(PATCH_ADDR,patch);
        FPD = 0;
        break;

      case BTN_MENU+BTN_ENTER:
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(35,15);
          display.println("DON'T");
          display.setCursor(35,30);
          display.println("PANIC");
          display.display(); // call display explicitly _before_ we call midiPanic
          midiPanic();
          break;

        case BTN_MENU+BTN_ENTER+BTN_UP+BTN_DOWN:
        //all keys depressed, reboot to programming mode
        _reboot_Teensyduino_();
    }
  }

  return redraw;
}


static bool idlePageUpdate(KeyState& __unused input, uint32_t __unused timeNow) {
  bool redraw = false;
  if (stateFirstRun) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    display.clearDisplay();
    redraw = true;
    stateFirstRun = 0;
  }
  if (input.changed) {
    int trills = readTrills();
    switch (input.current){
      case BTN_UP:
        // fallthrough
      case BTN_DOWN:
        if (!trills) {
          patch = (((patch-1u) + ((input.current == BTN_UP)?1u:-1u))&127u) + 1u;
        }
        // fallthrough
      case BTN_ENTER:
        if (trills && (fastPatch[trills-1] > 0)){
          patch = fastPatch[trills-1];
          activePatch = 0;
          doPatchUpdate = 1;
          FPD = 1;
        }
        menuState= PATCH_VIEW;
        stateFirstRun = 1;
        break;

      case BTN_MENU:
        /* REMOVE ALL MODIFIER ENTRIES
        if (pinkyKey && (exSensor >= ((extracThrVal+extracMaxVal)/2)) && !specialKey) { // switch breath activated legacy settings on/off
          legacyBrAct = !legacyBrAct;
          dipSwBits = dipSwBits ^ (1<<2);
          writeSetting(DIPSW_BITS_ADDR,dipSwBits);
          statusLedBlink();
        } else if ((exSensor >= ((extracThrVal+extracMaxVal)/2)) && !specialKey) { // switch pb pad activated legacy settings control on/off
          legacy = !legacy;
          dipSwBits = dipSwBits ^ (1<<1);
          writeSetting(DIPSW_BITS_ADDR,dipSwBits);
          statusLedBlink();
        } else if (pinkyKey && !specialKey){ //hold pinky key for rotator menu, and if too high touch sensing blocks regular menu, touching special key helps
          display.ssd1306_command(SSD1306_DISPLAYON);
          menuState= ROTATOR_MENU;
          stateFirstRun = 1;
        } else {
          display.ssd1306_command(SSD1306_DISPLAYON);
          menuState = MAIN_MENU;
          stateFirstRun = 1;
        }*/
        display.ssd1306_command(SSD1306_DISPLAYON);
        menuState = MAIN_MENU;
        stateFirstRun = 1;
        break;

      case BTN_UP | BTN_DOWN | BTN_ENTER | BTN_MENU:
        //all keys depressed, reboot to programming mode
        _reboot_Teensyduino_();
    }
  }
  return redraw;
}


//***********************************************************

static KeyState readInput(uint32_t timeNow) {

  static uint32_t lastDebounceTime = 0;         // the last time the output pin was toggled
  static uint32_t buttonRepeatTime = 0;
  static uint32_t buttonPressedTime = 0;
  static uint8_t lastDeumButtons = 0;
  static uint8_t deumButtonState = 0;

  KeyState keys = { deumButtonState, 0 };

  // read the state of the switches (note that they are active low, so we invert the values)
  uint8_t deumButtons = 0x0f ^(digitalRead(dPin) | (digitalRead(ePin) << 1) | (digitalRead(uPin) << 2) | (digitalRead(mPin)<<3));

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (deumButtons != lastDeumButtons) {
    // reset the debouncing timer
    lastDebounceTime = timeNow;
  }


  if ((timeNow - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (deumButtons != deumButtonState) {
      keys.current = deumButtons;
      keys.changed = deumButtonState ^ deumButtons;

      deumButtonState = deumButtons;
      menuTime = timeNow;
      buttonPressedTime = timeNow;
    }

    if (((deumButtons == BTN_DOWN) || (deumButtons == BTN_UP)) && (timeNow - buttonPressedTime > buttonRepeatDelay) && (timeNow - buttonRepeatTime > buttonRepeatInterval)){
      buttonRepeatTime = timeNow;
      keys.changed = deumButtons; // Key repeat
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastDeumButtons = deumButtons;

  return keys;
}



void menu() {
  unsigned long timeNow = millis();

  bool redraw = stateFirstRun;

  KeyState input = readInput(timeNow);

  // shut off menu system if not used for a while (changes not stored by exiting a setting manually will not be stored in EEPROM)
  if (menuState&& ((timeNow - menuTime) > menuTimeUp)) {
    menuState= DISPLAYOFF_IDL;
    stateFirstRun = 1;
    subVibSquelch = 0;
    memset(activeSub, 0, sizeof(activeSub));
  }

  if (menuState== DISPLAYOFF_IDL) {
    redraw |= updatePage((const MenuPage*)&idleMenuPage, input, timeNow);
  } else if (menuState== PATCH_VIEW) {
    redraw |= updatePage((const MenuPage*)&patchMenuPage, input, timeNow);
  } else if (menuState== MAIN_MENU) {
    redraw |= updatePage(&mainMenuPage, input, timeNow);
  } else if (menuState== ROTATOR_MENU) {
    redraw |= updatePage(&rotatorMenuPage, input, timeNow);
  } else if (menuState== ADJUST_MENU) {
    redraw |= updatePage((const MenuPage*)&adjustMenuPage, input, timeNow);
  } else if (menuState== SETUP_BR_MENU) {
    redraw |= updatePage(&breathMenuPage, input, timeNow);
  } else if (menuState== SETUP_CT_MENU) {
    redraw |= updatePage(&controlMenuPage, input, timeNow);
  } else if (menuState== VIBRATO_MENU) {
    redraw |= updatePage(&vibratoMenuPage, input, timeNow);
  } else if (menuState == ABOUT_MENU) {
    redraw |= updatePage((const MenuPage*)&aboutMenuPage, input, timeNow);
  } else if (menuState == EXTRAS_MENU) {
    redraw |= updatePage((const MenuPage*)&extrasMenuPage, input, timeNow);
  } else if (menuState == ROTA_MENU) {
    redraw |= updatePage((const MenuPage*)&rotSubAMenuPage, input, timeNow);
  } else if (menuState == ROTB_MENU) {
    redraw |= updatePage((const MenuPage*)&rotSubBMenuPage, input, timeNow);
  } else if (menuState == ROTC_MENU) {
    redraw |= updatePage((const MenuPage*)&rotSubCMenuPage, input, timeNow);
  }

  if(redraw) {
    display.display();
  }
}
