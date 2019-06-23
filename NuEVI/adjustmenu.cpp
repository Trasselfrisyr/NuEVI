#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>

#include "menu.h"
#include "numenu.h"
#include "globals.h"
#include "config.h"
#include "hardware.h"
#include "settings.h"

//***********************************************************

extern Adafruit_SSD1306 display;
extern Adafruit_MPR121 touchSensor;
extern byte cursorNow;

//***********************************************************
// Variables used for the adjust menu
static byte forcePix;

static uint16_t pos1;
static uint16_t pos2;

static int16_t adjustOption;
static int16_t adjustCurrent;

static int16_t sensorPixelPos1 = -1;
static int16_t sensorPixelPos2 = -1;

static bool refreshScreen;

//***********************************************************

static void breathSave(const AdjustMenuEntry& e) {
    writeSetting(BREATH_THR_ADDR, *e.entries[0].value);
    writeSetting(BREATH_MAX_ADDR, *e.entries[1].value);
}

const AdjustMenuEntry breathAdjustMenu  = {
  "BREATH",
  {
    { &breathThrVal, breathLoLimit, breathHiLimit },
    { &breathMaxVal, breathLoLimit, breathHiLimit }
  },
  breathSave
};

static void portamentoSave(const AdjustMenuEntry& e) {
  writeSetting(PORTAM_THR_ADDR, *e.entries[0].value);
  writeSetting(PORTAM_MAX_ADDR, *e.entries[1].value);
}

const AdjustMenuEntry portamentoAdjustMenu = {
  "PORTAMENTO", 
  {
    { &portamThrVal, portamLoLimit, portamHiLimit },
    { &portamMaxVal, portamLoLimit, portamHiLimit }
  }, 
  portamentoSave
};

static void pbSave(const AdjustMenuEntry& e) {
  writeSetting(PITCHB_THR_ADDR, *e.entries[0].value);
  writeSetting(PITCHB_MAX_ADDR, *e.entries[1].value);
}

const AdjustMenuEntry pitchBendAdjustMenu = {
  "PITCH BEND", 
  {
    { &pitchbThrVal, pitchbLoLimit, pitchbHiLimit },
    { &pitchbMaxVal, pitchbLoLimit, pitchbHiLimit }
  }, 
  pbSave
};

static void extracSave(const AdjustMenuEntry& e) {
  writeSetting(EXTRAC_THR_ADDR, *e.entries[0].value);
  writeSetting(EXTRAC_MAX_ADDR, *e.entries[1].value);
}

const AdjustMenuEntry extraSensorAdjustMenu = {
  "EXTRA CONTROLLER", 
  {
    { &extracThrVal, extracLoLimit, extracHiLimit },
    { &extracMaxVal, extracLoLimit, extracHiLimit }
  }, 
  extracSave
};


static void ctouchThrSave(const AdjustMenuEntry& e) {
  writeSetting(CTOUCH_THR_ADDR, *e.entries[0].value);
}

const AdjustMenuEntry ctouchAdjustMenu = {
  "TOUCH SENSE", 
  {
    { &ctouchThrVal, ctouchLoLimit, ctouchHiLimit },
    { nullptr, 0, 0 }
  }, 
  ctouchThrSave
};

const AdjustMenuEntry* adjustMenuEntries[] = {
  &breathAdjustMenu,
  &portamentoAdjustMenu,
  &pitchBendAdjustMenu,
  &extraSensorAdjustMenu,
  &ctouchAdjustMenu,
};

static const int numAdjustEntries = ARR_LEN(adjustMenuEntries);

//***********************************************************

static void drawAdjCursor(byte color) {
  display.drawTriangle(16,4,20,4,18,1,color);
  display.drawTriangle(16,6,20,6,18,9,color);
}

static void drawAdjustFrame(int line) {
  display.drawLine(25,line,120,line,WHITE); // Top line
  display.drawLine(25,line+12,120,line+12,WHITE); // Bottom line

  display.drawLine(25,line+1,25,line+2,WHITE);
  display.drawLine(120,line+1,120,line+2,WHITE);

  display.drawLine(120,line+10,120,line+11,WHITE);
  display.drawLine(25,line+10,25,line+11,WHITE);
}

static void drawAdjustBase(const char* title, bool all) {
  display.clearDisplay();

  drawAdjustFrame(17);

  // sensor marker lines.
  display.drawLine(25,36,25,40,WHITE);
  display.drawLine(120,36,120,40,WHITE);

  display.setTextSize(1);
  display.setCursor(25,2);
  display.println(title);

  display.setCursor(0,20);
  display.println("THR");
  display.setCursor(0,35);
  display.println("SNS");


  if(all) {
    drawAdjustFrame(47);
    display.setCursor(0,50);
    display.println("MAX");
  }
  cursorNow = WHITE;
  drawAdjCursor(WHITE);
}

static void drawLineCursor(uint16_t hPos, uint16_t vPos, int color) {
  display.drawLine(hPos, vPos,hPos, vPos+6, color);
}

static bool updateAdjustLineCursor(uint32_t timeNow, uint16_t hPos, uint16_t vPos ) {
  if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
    drawLineCursor(hPos, vPos, cursorNow);
    cursorBlinkTime = timeNow;
    return true;
  }
  return false;
}

static void drawAdjustMenu(const AdjustMenuEntry *menu) {

  bool haveSecondValue = menu->entries[1].value != nullptr;
  drawAdjustBase( menu->title, haveSecondValue );

  pos1 = map( *menu->entries[0].value, menu->entries[0].limitLow, menu->entries[0].limitHigh, 27, 119);
  display.drawLine( pos1, 20, pos1, 26, WHITE );

  if(haveSecondValue) {
    pos2 = map( *menu->entries[1].value, menu->entries[1].limitLow, menu->entries[1].limitHigh, 27, 119);
    display.drawLine( pos2, 50, pos2, 56, WHITE );
  }
}

//***********************************************************

static bool updateSensorPixel(int pos, int pos2) {
  bool update = pos != sensorPixelPos1 || pos2 != sensorPixelPos2;
  if(update) {
    display.drawFastHLine(28, 38, 119-28, BLACK); // Clear old line
    display.drawPixel(pos, 38, WHITE);
    if( pos2 >= 0) display.drawPixel(pos2, 38, WHITE);

    sensorPixelPos1 = pos;
    sensorPixelPos2 = pos2;
  }
  return update;
}

void plotSensorPixels(){
  int redraw = 0;

  if(forcePix)
    sensorPixelPos1 = -1;

  // This is hacky. It depends on the order of items in the adjust menu list.
  if(adjustOption == 0) {
    int pos = map(constrain(pressureSensor, breathLoLimit, breathHiLimit), breathLoLimit, breathHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, -1);
  }
  else if(adjustOption == 1) {
    int pos = map(constrain(biteSensor,portamLoLimit,portamHiLimit), portamLoLimit, portamHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, -1);
  }
  else if(adjustOption == 2) {
    int pos = map(constrain(pbUp, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    int pos2 = map(constrain(pbDn, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, pos2);
  }
  else if(adjustOption == 3) {
    int pos = map(constrain(exSensor, extracLoLimit, extracHiLimit), extracLoLimit, extracHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, -1);
  }
  else if(adjustOption == 4) {
    display.drawLine(28,38,118,38,BLACK);
    for (byte i=0; i<12; i++){
      int pos = map(constrain(touchSensor.filteredData(i), ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 38, WHITE);
    }

    int posRead = map(touchRead(halfPitchBendKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    int pos = map(constrain(posRead, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);

    posRead = map(touchRead(specialKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    int pos2 = map(constrain(posRead, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);

    updateSensorPixel(pos, pos2);

    redraw = 1;
  }
  if (redraw){
    display.display();
  }
  forcePix = 0;
}

//***********************************************************

static bool drawAdjustBar(uint16_t buttons, int row, const AdjustValue* entry, uint16_t *pos) {
  bool updated = false;
  uint16_t step = (entry->limitHigh-entry->limitLow)/92;
  int val = *entry->value;
  switch(buttons) {
    case BTN_UP:
      val += step;
      updated = true;
    break;

    case BTN_DOWN:
      val -= step;
      updated = true;
    break;
  }
  if(updated) {
    *entry->value = constrain(val, entry->limitLow, entry->limitHigh);
    auto p = *pos;
    display.drawLine(p, row, p, row+6, BLACK);
    *pos = p = map(*entry->value, entry->limitLow, entry->limitHigh, 27, 119);
    display.drawLine(p, row, p, row+6, WHITE);
    cursorNow = BLACK;
  }
  return updated;
}

static bool updateAdjustCursor(uint32_t timeNow) {
  if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
    drawAdjCursor(cursorNow);
    cursorBlinkTime = timeNow;
    return true;
  }
  return false;
}

static bool handleInput(const AdjustMenuEntry *currentMenu, uint32_t timeNow, uint8_t buttons, uint16_t *xpos, int ypos, int index) {
  if (buttons) {
    drawAdjustBar( buttons, ypos, &currentMenu->entries[index], xpos );
    int last = adjustCurrent;
    if(buttons == BTN_ENTER)      adjustCurrent += 1;
    else if( buttons == BTN_MENU) adjustCurrent = 0;

    if(last != adjustCurrent) drawLineCursor(*xpos, ypos, WHITE);

    return true;
  } else { 
    return updateAdjustLineCursor(timeNow, *xpos, ypos);
  }
}

int updateAdjustMenu(uint32_t timeNow, KeyState &input, bool firstRun, bool drawSensor) {
  bool redraw = false;
  int result = 0; 

  const AdjustMenuEntry *currentMenu = adjustMenuEntries[adjustOption];

  uint8_t buttons = input.changed ? input.current : 0;

  if(firstRun || refreshScreen) {
    adjustCurrent = 0;
    refreshScreen = false;
    drawAdjustMenu(currentMenu);
    // the sensor pixel stuff should be refactored (to work again)
    forcePix = 1;
    sensorPixelPos1 = -1; // Force draw of sensor pixels
  }

  if(adjustCurrent == 0) {
    // Currently selecting what option to modify
    redraw |= updateAdjustCursor(timeNow);

    bool save = false;
    if( buttons == BTN_DOWN ) {
      adjustOption += 1;
      refreshScreen = 1;
      save = true;
    }
    else if ( buttons == BTN_UP ) {
      adjustOption -= 1;
      refreshScreen = 1;
      save = true;
    }
    else if ( buttons == BTN_ENTER ) {
      adjustCurrent = 1;
    }
    else if (buttons == BTN_MENU ) {
      adjustCurrent = 0;
      result = -1;
      save = true;
    }

    if(save && currentMenu->saveFunc)
      currentMenu->saveFunc(*currentMenu);

  } else if( adjustCurrent == 1) {
    handleInput(currentMenu, timeNow, buttons, &pos1, 20, 0);
  } else {
    handleInput(currentMenu, timeNow, buttons, &pos2, 50, 1);
  }

  // Keep adjustCurrent in range
  if( (adjustCurrent > 2) || ((adjustCurrent == 2) && (currentMenu->entries[1].value == nullptr)))
    adjustCurrent = 0;

  // Keep adjust option in range.
  if(adjustOption < 0)
    adjustOption = numAdjustEntries-1;
  else if (adjustOption >= numAdjustEntries)
    adjustOption = 0;


  if(drawSensor) {
    plotSensorPixels();
  }

  if(result >= 0)
    result = redraw;

  return result;
}

