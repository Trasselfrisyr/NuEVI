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
#if defined(NURAD)
extern Adafruit_MPR121 touchSensorRH;
extern Adafruit_MPR121 touchSensorLH;
extern Adafruit_MPR121 touchSensorRollers;
#else
extern Adafruit_MPR121 touchSensor;
#endif
extern byte cursorNow;
#if defined(NURAD)
extern int calOffsetRollers[6];
extern int calOffsetRH[12];
extern int calOffsetLH[12];
#endif

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
  "BITE", 
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
  "BEND", 
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
  "LIP/EC", 
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
  "TOUCH", 
  {
    { &ctouchThrVal, ctouchLoLimit, ctouchHiLimit },
    { nullptr, 0, 0 }
  }, 
  ctouchThrSave
};


static void leverSave(const AdjustMenuEntry& e) {
  writeSetting(LEVER_THR_ADDR, *e.entries[0].value);
  writeSetting(LEVER_MAX_ADDR, *e.entries[1].value);
}

const AdjustMenuEntry leverAdjustMenu = {
  "LEVER", 
  {
    { &leverThrVal, leverLoLimit, leverHiLimit },
    { &leverMaxVal, leverLoLimit, leverHiLimit }
  }, 
  leverSave
};


const AdjustMenuEntry* adjustMenuEntries[] = {
  &breathAdjustMenu,
  &portamentoAdjustMenu,
  &pitchBendAdjustMenu,
  &extraSensorAdjustMenu,
  &ctouchAdjustMenu,
  &leverAdjustMenu,
};

static const int numAdjustEntries = ARR_LEN(adjustMenuEntries);

//***********************************************************



void autoCalSelected() {
  int calRead;
  int calReadNext;
// NuRAD/NuEVI sensor calibration
  // Extra Controller
  if(adjustOption == 3) {
    calRead = touchRead(extraPin);
    extracThrVal = constrain(calRead+200, extracLoLimit, extracHiLimit);
    extracMaxVal = constrain(extracThrVal+600, extracLoLimit, extracHiLimit);
    writeSetting(EXTRAC_THR_ADDR, extracThrVal);
    writeSetting(EXTRAC_MAX_ADDR, extracMaxVal);
  }
  // Breath sensor
  if(adjustOption == 0) {
    calRead = analogRead(breathSensorPin);
    breathThrVal = constrain(calRead+200, breathLoLimit, breathHiLimit);
    breathMaxVal = constrain(breathThrVal+1500, breathLoLimit, breathHiLimit);
    writeSetting(BREATH_THR_ADDR, breathThrVal);
    writeSetting(BREATH_MAX_ADDR, breathMaxVal);
  }
  // Pitch Bend
  if(adjustOption == 2) {
    calRead = touchRead(pbUpPin);
    calReadNext = touchRead(pbDnPin);
    if (calReadNext > calRead) calRead = calReadNext; //use highest value
    pitchbThrVal = constrain(calRead+200, pitchbLoLimit, pitchbHiLimit);
    pitchbMaxVal = constrain(pitchbThrVal+800, pitchbLoLimit, pitchbHiLimit);
    writeSetting(PITCHB_THR_ADDR, pitchbThrVal);
    writeSetting(PITCHB_MAX_ADDR, pitchbMaxVal);
  }
  // Lever
  if(adjustOption == 5) {
#if defined(SEAMUS)
    calRead = touchRead(vibratoPin);
#else
    calRead = 3000-touchRead(vibratoPin);
#endif
    leverThrVal = constrain(calRead+60, leverLoLimit, leverHiLimit);
    leverMaxVal = constrain(calRead+120, leverLoLimit, leverHiLimit);
    writeSetting(LEVER_THR_ADDR, leverThrVal);
    writeSetting(LEVER_MAX_ADDR, leverMaxVal);
  }
#if defined(NURAD) // NuRAD sensor calibration
  // Bite Pressure sensor
  if(adjustOption == 1) {
    calRead = analogRead(bitePressurePin);
    portamThrVal = constrain(calRead+300, portamLoLimit, portamHiLimit);
    portamMaxVal = constrain(portamThrVal+600, portamLoLimit, portamHiLimit);
    writeSetting(PORTAM_THR_ADDR, portamThrVal);
    writeSetting(PORTAM_MAX_ADDR, portamMaxVal);
  }
  // Touch sensors
  if(adjustOption == 4) {
    calRead = ctouchHiLimit;  
    for (byte i = 0; i < 6; i++) {
      calReadNext = touchSensorRollers.filteredData(i) * (300-calOffsetRollers[i])/300;
      if (calReadNext < calRead) calRead = calReadNext; //use lowest value
    }
    for (byte i = 0; i < 12; i++) {
      calReadNext = touchSensorRH.filteredData(i) * (300-calOffsetRH[i])/300;
      if (calReadNext < calRead) calRead = calReadNext; //use lowest value
    }
    for (byte i = 0; i < 12; i++) {
      calReadNext = touchSensorLH.filteredData(i) * (300-calOffsetLH[i])/300;
      if (calReadNext < calRead) calRead = calReadNext; //use lowest value
    }
    ctouchThrVal = constrain(calRead-20, ctouchLoLimit, ctouchHiLimit);
    touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
    writeSetting(CTOUCH_THR_ADDR, ctouchThrVal);
  }
#else // NuEVI sensor calibration
  // Bite sensor
  if(adjustOption == 1) {
    if (digitalRead(biteJumperPin)){ //PBITE (if pulled low with jumper, pressure sensor is used instead of capacitive bite sensing)
      // Capacitive sensor
      calRead = touchRead(bitePin);
      portamThrVal = constrain(calRead+200, portamLoLimit, portamHiLimit);
      portamMaxVal = constrain(portamThrVal+600, portamLoLimit, portamHiLimit);
      writeSetting(PORTAM_THR_ADDR, portamThrVal);
      writeSetting(PORTAM_MAX_ADDR, portamMaxVal);
    } else {
      // Pressure sensor
      calRead = analogRead(bitePressurePin);
      portamThrVal = constrain(calRead+300, portamLoLimit, portamHiLimit);
      portamMaxVal = constrain(portamThrVal+600, portamLoLimit, portamHiLimit);
      writeSetting(PORTAM_THR_ADDR, portamThrVal);
      writeSetting(PORTAM_MAX_ADDR, portamMaxVal);
    }
  }
  // Touch sensors
  if(adjustOption == 4) {
    calRead = ctouchHiLimit;  
    for (byte i = 0; i < 12; i++) {
      calReadNext = touchSensor.filteredData(i);
      if (calReadNext < calRead) calRead = calReadNext; //use lowest value
    }
    calReadNext=map(touchRead(halfPitchBendKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    if (calReadNext < calRead) calRead = calReadNext; //use lowest value
    calReadNext=map(touchRead(specialKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    if (calReadNext < calRead) calRead = calReadNext; //use lowest value
    ctouchThrVal = constrain(calRead-20, ctouchLoLimit, ctouchHiLimit);
    touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
    writeSetting(CTOUCH_THR_ADDR, ctouchThrVal);
  }
#endif
}


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
  display.fillRect(64,0,64,9,BLACK);
  display.setTextSize(1);
  if(haveSecondValue) {
    display.setCursor(68,2);
    display.print(*menu->entries[0].value);
    display.print("|");
    display.print(*menu->entries[1].value);
  } else {
    display.setCursor(104,2);
    display.print(*menu->entries[0].value);
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
    if (biteJumper) { //PBITE (if pulled low with jumper or if on a NuRAD, use pressure sensor instead of capacitive bite sensor)
      biteSensor=analogRead(bitePressurePin); // alternative kind bite sensor (air pressure tube and sensor)  PBITE
    } else {
      biteSensor = touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right)
    }
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
  #if defined(NURAD)
  else if(adjustOption == 4) {
        display.drawLine(28,37,118,37,BLACK);
    for (byte i=0; i<12; i++){
      //int pos = map(constrain(touchSensorRH.filteredData(i) - calOffsetRH[i], ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      int pos = map(constrain(touchSensorRH.filteredData(i) * (300-calOffsetRH[i])/300, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 37, WHITE);
    }
    display.drawLine(28,38,118,38,BLACK);
    for (byte i=0; i<12; i++){
      //int pos = map(constrain(touchSensorLH.filteredData(i) - calOffsetLH[i], ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      int pos = map(constrain(touchSensorLH.filteredData(i) * (300-calOffsetLH[i])/300, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 38, WHITE);
    }
    display.drawLine(28,39,118,39,BLACK);
    for (byte i=0; i<6; i++){
      //int pos = map(constrain(touchSensorRollers.filteredData(i) - calOffsetRollers[i], ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      int pos = map(constrain(touchSensorRollers.filteredData(i) * (300-calOffsetRollers[i])/300, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 39, WHITE);
    }
    redraw = 1;
  }
  #else //NuEVI
  else if(adjustOption == 4) {
    display.drawLine(28,39,118,39,BLACK);
    for (byte i=0; i<12; i++){
      int pos = map(constrain(touchSensor.filteredData(i), ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 39, WHITE);
    }

    int posRead = map(touchRead(halfPitchBendKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    int pos = map(constrain(posRead, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);

    posRead = map(touchRead(specialKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    int pos2 = map(constrain(posRead, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);

    updateSensorPixel(pos, pos2);

    redraw = 1;
  }
  #endif
  else if(adjustOption == 5) {
 #if defined(SEAMUS)
    int pos = map(constrain(touchRead(vibratoPin), leverLoLimit, leverHiLimit), leverLoLimit, leverHiLimit, 28, 118);
 #else
     int pos = map(constrain(3000-touchRead(vibratoPin), leverLoLimit, leverHiLimit), leverLoLimit, leverHiLimit, 28, 118);
 #endif
    redraw = updateSensorPixel(pos, -1);
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
  bool haveSecondValue = currentMenu->entries[1].value != nullptr;
  if (buttons) {
    if (buttons == BTN_DOWN+BTN_UP){
      display.fillRect(26,35,90,7,BLACK);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(53,35);
      display.println("AUTOCAL");
      display.display();
      delay(2000);
      autoCalSelected();
      display.fillRect(26,35,90,7,BLACK);
      display.display();
      drawAdjustMenu(currentMenu);
      forcePix = 1;
      plotSensorPixels();
    } else 
    drawAdjustBar( buttons, ypos, &currentMenu->entries[index], xpos );

    display.fillRect(64,0,64,9,BLACK);
    display.setTextSize(1);
    if(haveSecondValue) {
      display.setCursor(68,2);
      display.print(*currentMenu->entries[0].value);
      display.print("|");
      display.print(*currentMenu->entries[1].value);
    } else {
      display.setCursor(104,2);
      display.print(*currentMenu->entries[0].value);
    }
    
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
    redraw |= handleInput(currentMenu, timeNow, buttons, &pos1, 20, 0);
  } else {
    redraw |= handleInput(currentMenu, timeNow, buttons, &pos2, 50, 1);
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
