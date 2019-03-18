#include "menu.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef SSD1306_128_64
#error("Incorrect display type, please fix Adafruit_SSD1306.h!");
#endif


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);



int minOffset = 50;

int deumButtons = 0;
int lastDeumButtons = 0;
int deumButtonState = 0;
byte buttonPressedAndNotUsed = 0;

byte mainMenuCursor = 1;
byte setupBrMenuCursor = 1;
byte setupCtMenuCursor = 1;
byte rotatorMenuCursor = 1;
byte vibratoMenuCursor = 1;

//Display state
byte state = DISPLAYOFF_IDL;
byte stateFirstRun = 1;

byte subTranspose = 0;
byte subOctave = 0;
byte subMIDI = 0;
byte subBreathCC = 0;
byte subBreathAT = 0;
byte subVelocity = 0;
byte subCurve = 0;
byte subPort = 0;
byte subPB = 0;
byte subExtra = 0;
byte subVibrato = 0;
byte subDeglitch = 0;
byte subPinky = 0;
byte subVelSmpDl = 0;
byte subVelBias = 0;
byte subParallel = 0;
byte subRotator = 0;
byte subPriority = 0;
byte subVibSquelch = 0; //extern
byte subVibSens = 0;
byte subVibRetn = 0;
byte subVibDirection = 0;


 // 'NuEVI' logo
#define LOGO16_GLCD_WIDTH  128
#define LOGO16_GLCD_HEIGHT 64
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

void initDisplay() {

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  display.clearDisplay();
  display.drawBitmap(0,0,nuevi_logo_bmp,LOGO16_GLCD_WIDTH,LOGO16_GLCD_HEIGHT,1);
  display.display();

}

void showVersion() {
  display.setTextColor(WHITE);
  display.setTextSize(1);
  #if defined(CASSIDY)
  display.setCursor(0,0);
  display.print("BC");
  #endif
  display.setCursor(85,52);
  display.print("v.");
  display.println(FIRMWARE_VERSION);
  display.display();
}

void menu() {

  // read the state of the switches
  deumButtons = !digitalRead(dPin)+2*!digitalRead(ePin)+4*!digitalRead(uPin)+8*!digitalRead(mPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (deumButtons != lastDeumButtons) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (deumButtons != deumButtonState) {
      deumButtonState = deumButtons;
      menuTime = millis();
      Serial.println(deumButtonState);
      buttonPressedAndNotUsed = 1;
      buttonPressedTime = millis();
    }

    if (((deumButtons == 1) || (deumButtons == 4)) && (millis() - buttonPressedTime > buttonRepeatDelay) && (millis() - buttonRepeatTime > buttonRepeatInterval)){
      buttonPressedAndNotUsed = 1;
      buttonRepeatTime = millis();
    }

  }


  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastDeumButtons = deumButtons;

  if (state && ((millis() - menuTime) > menuTimeUp)) { // shut off menu system if not used for a while (changes not stored by exiting a setting manually will not be stored in EEPROM)
    state = DISPLAYOFF_IDL;
    stateFirstRun = 1;
    subTranspose = 0;
    subMIDI = 0;
    subBreathCC = 0;
    subBreathAT = 0;
    subVelocity = 0;
    subPort = 0;
    subPB = 0;
    subExtra = 0;
    subVibrato = 0;
    subDeglitch = 0;
    subPinky = 0;
    subVelSmpDl = 0;
    subVelBias = 0;
    subParallel = 0;
    subRotator = 0;
    subPriority = 0;
    subVibSens = 0;
    subVibRetn = 0;
    subVibSquelch = 0;
    subVibDirection = 0;
  }



  if        (state == DISPLAYOFF_IDL){
    if (stateFirstRun) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      stateFirstRun = 0;
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      int trills = readTrills();
      switch (deumButtonState){
        case 1:
          // down
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
          } else if (!trills) buttonPressedAndNotUsed = 1;
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = PATCH_VIEW;
          stateFirstRun = 1;
          break;
        case 2:
          // enter
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
          }
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = PATCH_VIEW;
          stateFirstRun = 1;
          break;
        case 4:
          // up
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
          } else if (!trills) buttonPressedAndNotUsed = 1;
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = PATCH_VIEW;
          buttonPressedAndNotUsed = 1;
          stateFirstRun = 1;
          break;
        case 8:
          // menu
          if (pinkyKey && (exSensor >= ((extracThrVal+extracMaxVal)/2))){ // switch breath activated legacy settings on/off
            legacyBrAct = !legacyBrAct;
            dipSwBits = dipSwBits ^ (1<<2);
            writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            digitalWrite(statusLedPin,LOW);
            delay(150);
            digitalWrite(statusLedPin,HIGH);
            delay(150);
            digitalWrite(statusLedPin, LOW);
            delay(150);
            digitalWrite(statusLedPin,HIGH);
          } else if ((exSensor >= ((extracThrVal+extracMaxVal)/2))){ // switch pb pad activated legacy settings control on/off
            legacy = !legacy;
            dipSwBits = dipSwBits ^ (1<<1);
            writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            digitalWrite(statusLedPin,LOW);
            delay(150);
            digitalWrite(statusLedPin,HIGH);
            delay(150);
            digitalWrite(statusLedPin,LOW);
            delay(150);
            digitalWrite(statusLedPin,HIGH);
          } else if (pinkyKey){
            display.ssd1306_command(SSD1306_DISPLAYON);
            state = ROTATOR_MENU;
            stateFirstRun = 1;
          } else {
            display.ssd1306_command(SSD1306_DISPLAYON);
            state = MAIN_MENU;
            stateFirstRun = 1;
          }
          break;
        case 15:
          //all keys depressed, reboot to programming mode
          _reboot_Teensyduino_();
      }
    }
  } else if (state == PATCH_VIEW){
    if (stateFirstRun) {
      drawPatchView();
      patchViewTime = millis();
      stateFirstRun = 0;
    }
    if ((millis() - patchViewTime) > patchViewTimeUp) {
      state = DISPLAYOFF_IDL;
      stateFirstRun = 1;
      doPatchUpdate = 1;
      FPD = 0;
      if (readSetting(PATCH_ADDR) != patch) writeSetting(PATCH_ADDR,patch);
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      int trills = readTrills();
      switch (deumButtonState){
        case 1:
          // down
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            if (readSetting(PATCH_ADDR) != patch) writeSetting(PATCH_ADDR,patch);
          } else if (!trills){
            if (patch > 1){
              patch--;
            } else patch = 128;
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 0;
          }
          drawPatchView();
          patchViewTime = millis();
          break;
        case 2:
          // enter
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            drawPatchView();
          }
          patchViewTime = millis();
          break;
        case 4:
          // up
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            if (readSetting(PATCH_ADDR) != patch) writeSetting(PATCH_ADDR,patch);
          } else if (!trills){
            if (patch < 128){
              patch++;
            } else patch = 1;
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 0;
          }
          drawPatchView();
          patchViewTime = millis();
          break;
        case 8:
          // menu
          if (FPD < 2){
            state = DISPLAYOFF_IDL;
            stateFirstRun = 1;
            doPatchUpdate = 1;
          }
          if (readSetting(PATCH_ADDR) != patch) writeSetting(PATCH_ADDR,patch);
          FPD = 0;
          break;
        case 10:
          // enter + menu
            midiPanic();
            display.clearDisplay();
            display.setTextColor(WHITE);
            display.setTextSize(2);
            display.setCursor(35,15);
            display.println("DON'T");
            display.setCursor(35,30);
            display.println("PANIC");
            display.display();
            patchViewTime = millis();
            break;
          case 15:
          //all keys depressed, reboot to programming mode
          _reboot_Teensyduino_();
      }
    }
  } else if (state == MAIN_MENU){    // MAIN MENU HERE <<<<<<<<<<<<<<<
    if (stateFirstRun) {
      drawMenuScreen();
      stateFirstRun = 0;
    }
    if (subTranspose){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotTranspose(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (transpose > 0){
              plotTranspose(BLACK);
              transpose--;
              plotTranspose(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotTranspose(WHITE);
            cursorNow = BLACK;
            display.display();
            subTranspose = 0;
            if (readSetting(TRANSP_ADDR) != transpose) writeSetting(TRANSP_ADDR,transpose);
            break;
          case 4:
            // up
            if (transpose < 24){
              plotTranspose(BLACK);
              transpose++;
              plotTranspose(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotTranspose(WHITE);
            cursorNow = BLACK;
            display.display();
            subTranspose = 0;
            if (readSetting(TRANSP_ADDR) != transpose) writeSetting(TRANSP_ADDR,transpose);
            break;
        }
      }
    } else if (subOctave){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotOctave(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (octave > 0){
              plotOctave(BLACK);
              octave--;
              plotOctave(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotOctave(WHITE);
            cursorNow = BLACK;
            display.display();
            subOctave = 0;
            if (readSetting(OCTAVE_ADDR) != octave) writeSetting(OCTAVE_ADDR,octave);
            break;
          case 4:
            // up
            if (octave < 6){
              plotOctave(BLACK);
              octave++;
              plotOctave(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotOctave(WHITE);
            cursorNow = BLACK;
            display.display();
            subOctave = 0;
            if (readSetting(OCTAVE_ADDR) != octave) writeSetting(OCTAVE_ADDR,octave);
            break;
        }
      }
    } else if (subMIDI) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotMIDI(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (MIDIchannel > 1){
              plotMIDI(BLACK);
              MIDIchannel--;
              plotMIDI(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            readSwitches();
            if (pinkyKey){
              slowMidi = !slowMidi;
              plotMIDI(WHITE);
              cursorNow = BLACK;
              display.display();
              dipSwBits = dipSwBits ^ (1<<3);
              writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            } else {
              plotMIDI(WHITE);
              cursorNow = BLACK;
              display.display();
              subMIDI = 0;
              if (readSetting(MIDI_ADDR) != MIDIchannel) writeSetting(MIDI_ADDR,MIDIchannel);
            }
            break;
          case 4:
            // up
            if (MIDIchannel < 16){
              plotMIDI(BLACK);
              MIDIchannel++;
              plotMIDI(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotMIDI(WHITE);
            cursorNow = BLACK;
            display.display();
            subMIDI = 0;
            if (readSetting(MIDI_ADDR) != MIDIchannel) writeSetting(MIDI_ADDR,MIDIchannel);
            break;
        }
      }
    } else {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(mainMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case 1:
            // down
            if (mainMenuCursor < 6){
              drawMenuCursor(mainMenuCursor, BLACK);
              mainMenuCursor++;
              drawMenuCursor(mainMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 2:
            // enter
            selectMainMenu();
            break;
          case 4:
            // up
            if (mainMenuCursor > 1){
              drawMenuCursor(mainMenuCursor, BLACK);
              mainMenuCursor--;
              drawMenuCursor(mainMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 8:
            // menu
            state = DISPLAYOFF_IDL;
            stateFirstRun = 1;
            break;
          case 9:
            //menu+down

            break;
          case 10:
            //menu+enter
            if (trills){
              state = PATCH_VIEW;
              stateFirstRun = 1;
              setFPS(trills);
            }
            break;
          case 12:
            //menu+up
            if (trills){
              state = PATCH_VIEW;
              stateFirstRun = 1;
              clearFPS(trills);

            }
            break;
        }
      }
    }
  } else if (state == ROTATOR_MENU){    // ROTATOR MENU HERE <<<<<<<<<<<<<<<
    if (stateFirstRun) {
      drawRotatorMenuScreen();
      stateFirstRun = 0;
    }
    if (subParallel){
      if (((millis() - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotRotator(cursorNow,parallel);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (parallel > -24){
              plotRotator(BLACK,parallel);
              parallel--;
              plotRotator(WHITE,parallel);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotRotator(WHITE,parallel);
            cursorNow = BLACK;
            display.display();
            subParallel = 0;
            if (readSetting(PARAL_ADDR) != (parallel + 24)) writeSetting(PARAL_ADDR,(parallel + 24));
            break;
          case 4:
            // up
            if (parallel < 24){
              plotRotator(BLACK,parallel);
              parallel++;
              plotRotator(WHITE,parallel);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotRotator(WHITE,parallel);
            cursorNow = BLACK;
            display.display();
            subParallel = 0;
            if (readSetting(PARAL_ADDR) != (parallel + 24)) writeSetting(PARAL_ADDR,(parallel + 24));
            break;
        }
      }
    } else if (subRotator){
      if (((millis() - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotRotator(cursorNow,rotations[subRotator-1]);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (rotations[subRotator-1] > -24){
              plotRotator(BLACK,rotations[subRotator-1]);
              rotations[subRotator-1]--;
              plotRotator(WHITE,rotations[subRotator-1]);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotRotator(WHITE,rotations[subRotator-1]);
            cursorNow = BLACK;
            display.display();
            if (readSetting(ROTN1_ADDR+2*(subRotator-1)) != rotations[subRotator-1]) writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subRotator = 0;
            break;
          case 4:
            // up
            if (rotations[subRotator-1] < 24){
              plotRotator(BLACK,rotations[subRotator-1]);
              rotations[subRotator-1]++;
              plotRotator(WHITE,rotations[subRotator-1]);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotRotator(WHITE,rotations[subRotator-1]);
            cursorNow = BLACK;
            display.display();
            if (readSetting(ROTN1_ADDR+2*(subRotator-1)) != (rotations[subRotator-1]+24)) writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subRotator = 0;
            break;
        }
      }
    } else if (subPriority){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPriority(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotPriority(BLACK);
            priority = !priority;
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            subPriority = 0;
            if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
          case 4:
            // up
            plotPriority(BLACK);
            priority = !priority;
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            subPriority = 0;
            if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
        }
      }
    } else {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(rotatorMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case 1:
            // down
            if (rotatorMenuCursor < 6){
              drawMenuCursor(rotatorMenuCursor, BLACK);
              rotatorMenuCursor++;
              drawMenuCursor(rotatorMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 2:
            // enter
            selectRotatorMenu();
            break;
          case 4:
            // up
            if (rotatorMenuCursor > 1){
              drawMenuCursor(rotatorMenuCursor, BLACK);
              rotatorMenuCursor--;
              drawMenuCursor(rotatorMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 8:
            // menu
            state = DISPLAYOFF_IDL;
            stateFirstRun = 1;
            break;
          case 9:
            //menu+down

            break;
          case 10:
            //menu+enter
            if (trills){
              state = PATCH_VIEW;
              stateFirstRun = 1;
              setFPS(trills);
            }
            break;
          case 12:
            //menu+up
            if (trills){
              state = PATCH_VIEW;
              stateFirstRun = 1;
              clearFPS(trills);

            }
            break;
        }
      }
    }
  // end rotator menu
  } else if (state == BREATH_ADJ_IDL){
    if (stateFirstRun) {
      drawBreathScreen();
      forcePix = 1;
      stateFirstRun = 0;
    }
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      drawAdjCursor(cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          state = PORTAM_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(BREATH_THR_ADDR) != breathThrVal) writeSetting(BREATH_THR_ADDR,breathThrVal);
          if (readSetting(BREATH_MAX_ADDR) != breathMaxVal) writeSetting(BREATH_MAX_ADDR,breathMaxVal);
          break;
        case 2:
          // enter
          state = BREATH_ADJ_THR;
          break;
        case 4:
          // up
          state = CTOUCH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(BREATH_THR_ADDR) != breathThrVal) writeSetting(BREATH_THR_ADDR,breathThrVal);
          if (readSetting(BREATH_MAX_ADDR) != breathMaxVal) writeSetting(BREATH_MAX_ADDR,breathMaxVal);
          break;
        case 8:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(BREATH_THR_ADDR) != breathThrVal) writeSetting(BREATH_THR_ADDR,breathThrVal);
          if (readSetting(BREATH_MAX_ADDR) != breathMaxVal) writeSetting(BREATH_MAX_ADDR,breathMaxVal);
          break;
      }
    }
  } else if (state == BREATH_ADJ_THR){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos1,20,pos1,26,cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          if (breathThrVal - breathStep > breathLoLimit){
            breathThrVal -= breathStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(breathThrVal, breathLoLimit, breathHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = BREATH_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (breathThrVal + breathStep < breathHiLimit){
            breathThrVal += breathStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(breathThrVal, breathLoLimit, breathHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = BREATH_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == BREATH_ADJ_MAX){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos2,50,pos2,57,cursorNow);;
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
            if ((breathMaxVal - breathStep) > (breathThrVal + minOffset)){
            breathMaxVal -= breathStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(breathMaxVal, breathLoLimit, breathHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = BREATH_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (breathMaxVal + breathStep < breathHiLimit){
            breathMaxVal += breathStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(breathMaxVal, breathLoLimit, breathHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = BREATH_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PORTAM_ADJ_IDL){
    if (stateFirstRun) {
      drawPortamScreen();
      forcePix = 1;
      stateFirstRun = 0;
    }
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      drawAdjCursor(cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          state = PITCHB_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PORTAM_THR_ADDR) != portamThrVal) writeSetting(PORTAM_THR_ADDR,portamThrVal);
          if (readSetting(PORTAM_MAX_ADDR) != portamMaxVal) writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
        case 2:
          // enter
          state = PORTAM_ADJ_THR;
          break;
        case 4:
          // up
          state = BREATH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PORTAM_THR_ADDR) != portamThrVal) writeSetting(PORTAM_THR_ADDR,portamThrVal);
          if (readSetting(PORTAM_MAX_ADDR) != portamMaxVal) writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
        case 8:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(PORTAM_THR_ADDR) != portamThrVal) writeSetting(PORTAM_THR_ADDR,portamThrVal);
          if (readSetting(PORTAM_MAX_ADDR) != portamMaxVal) writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
      }
    }
  } else if (state == PORTAM_ADJ_THR){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos1,20,pos1,26,cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          if (portamThrVal - portamStep > portamLoLimit){
            portamThrVal -= portamStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(portamThrVal, portamLoLimit, portamHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = PORTAM_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (portamThrVal + portamStep < portamHiLimit){
            portamThrVal += portamStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(portamThrVal, portamLoLimit, portamHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PORTAM_ADJ_MAX){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos2,50,pos2,57,cursorNow);;
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
            if ((portamMaxVal - portamStep) > (portamThrVal + minOffset)){
            portamMaxVal -= portamStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(portamMaxVal, portamLoLimit, portamHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (portamMaxVal + portamStep < portamHiLimit){
            portamMaxVal += portamStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(portamMaxVal, portamLoLimit, portamHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PITCHB_ADJ_IDL){
    if (stateFirstRun) {
      drawPitchbScreen();
      forcePix = 1;
      stateFirstRun = 0;
    }
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      drawAdjCursor(cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          state = EXTRAC_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PITCHB_THR_ADDR) != pitchbThrVal) writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          if (readSetting(PITCHB_MAX_ADDR) != pitchbMaxVal) writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
        case 2:
          // enter
          state = PITCHB_ADJ_THR;
          break;
        case 4:
          // up
          state = PORTAM_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PITCHB_THR_ADDR) != pitchbThrVal) writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          if (readSetting(PITCHB_MAX_ADDR) != pitchbMaxVal) writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
        case 8:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(PITCHB_THR_ADDR) != pitchbThrVal) writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          if (readSetting(PITCHB_MAX_ADDR) != pitchbMaxVal) writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
      }
    }
  } else if (state == PITCHB_ADJ_THR){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos1,20,pos1,26,cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          if (pitchbThrVal - pitchbStep > pitchbLoLimit){
            pitchbThrVal -= pitchbStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(pitchbThrVal, pitchbLoLimit, pitchbHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = PITCHB_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (pitchbThrVal + pitchbStep < pitchbHiLimit){
            pitchbThrVal += pitchbStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(pitchbThrVal, pitchbLoLimit, pitchbHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PITCHB_ADJ_MAX){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos2,50,pos2,57,cursorNow);;
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
            if ((pitchbMaxVal - pitchbStep) > (pitchbThrVal + minOffset)){
            pitchbMaxVal -= pitchbStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(pitchbMaxVal, pitchbLoLimit, pitchbHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (pitchbMaxVal + pitchbStep < pitchbHiLimit){
            pitchbMaxVal += pitchbStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(pitchbMaxVal, pitchbLoLimit, pitchbHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
      }
    }

  } else if (state == EXTRAC_ADJ_IDL){
    if (stateFirstRun) {
      drawExtracScreen();
      forcePix = 1;
      stateFirstRun = 0;
    }
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      drawAdjCursor(cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          state = CTOUCH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(EXTRAC_THR_ADDR) != extracThrVal) writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          if (readSetting(EXTRAC_MAX_ADDR) != extracMaxVal) writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
        case 2:
          // enter
          state = EXTRAC_ADJ_THR;
          break;
        case 4:
          // up
          state = PITCHB_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(EXTRAC_THR_ADDR) != extracThrVal) writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          if (readSetting(EXTRAC_MAX_ADDR) != extracMaxVal) writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
        case 8:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(EXTRAC_THR_ADDR) != extracThrVal) writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          if (readSetting(EXTRAC_MAX_ADDR) != extracMaxVal) writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
      }
    }
  } else if (state == EXTRAC_ADJ_THR){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos1,20,pos1,26,cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          if (extracThrVal - extracStep > extracLoLimit){
            extracThrVal -= extracStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(extracThrVal, extracLoLimit, extracHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = EXTRAC_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (extracThrVal + extracStep < extracHiLimit){
            extracThrVal += extracStep;
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(extracThrVal, extracLoLimit, extracHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == EXTRAC_ADJ_MAX){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos2,50,pos2,57,cursorNow);;
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
            if ((extracMaxVal - extracStep) > (extracThrVal + minOffset)){
            extracMaxVal -= extracStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(extracMaxVal, extracLoLimit, extracHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (extracMaxVal + extracStep < extracHiLimit){
            extracMaxVal += extracStep;
            display.drawLine(pos2,50,pos2,57,BLACK);
            pos2 = map(extracMaxVal, extracLoLimit, extracHiLimit, 27, 119);
            display.drawLine(pos2,50,pos2,57,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos2,50,pos2,57,WHITE);
          display.display();
          break;
      }
    }

  } else if (state == CTOUCH_ADJ_IDL){
    if (stateFirstRun) {
      drawCtouchScreen();
      forcePix = 1;
      stateFirstRun = 0;
    }
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      drawAdjCursor(cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          state = BREATH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(CTOUCH_THR_ADDR) != ctouchThrVal) writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
        case 2:
          // enter
          state = CTOUCH_ADJ_THR;
          break;
        case 4:
          // up
          state = EXTRAC_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(CTOUCH_THR_ADDR) != ctouchThrVal) writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
        case 8:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(CTOUCH_THR_ADDR) != ctouchThrVal) writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
      }
    }
  } else if (state == CTOUCH_ADJ_THR){
    if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(pos1,20,pos1,26,cursorNow);
      display.display();
      cursorBlinkTime = millis();
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          if (ctouchThrVal - ctouchStep > ctouchLoLimit){
            ctouchThrVal -= ctouchStep;
            touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(ctouchThrVal, ctouchLoLimit, ctouchHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 2:
          // enter
          state = CTOUCH_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case 4:
          // up
          if (ctouchThrVal + ctouchStep < ctouchHiLimit){
            ctouchThrVal += ctouchStep;
            touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
            display.drawLine(pos1,20,pos1,26,BLACK);
            pos1 = map(ctouchThrVal, ctouchLoLimit, ctouchHiLimit, 27, 119);
            display.drawLine(pos1,20,pos1,26,WHITE);
            display.display();
            cursorBlinkTime = millis();
            cursorNow = BLACK;
          }
          break;
        case 8:
          // menu
          state = CTOUCH_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }


  } else if (state == SETUP_BR_MENU) {  // SETUP BREATH MENU HERE <<<<<<<<<<<<<<
    if (stateFirstRun) {
      drawSetupBrMenuScreen();
      stateFirstRun = 0;
    }
    if (subBreathCC){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotBreathCC(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (breathCC > 0){
              plotBreathCC(BLACK);
              breathCC--;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            } else {
              plotBreathCC(BLACK);
              breathCC = 10;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotBreathCC(WHITE);
            cursorNow = BLACK;
            display.display();
            subBreathCC = 0;
            if (readSetting(BREATH_CC_ADDR) != breathCC) {
              writeSetting(BREATH_CC_ADDR,breathCC);
              midiReset();
            }
            break;
          case 4:
            // up
            if (breathCC < 10){
              plotBreathCC(BLACK);
              breathCC++;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            } else {
              plotBreathCC(BLACK);
              breathCC = 0;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotBreathCC(WHITE);
            cursorNow = BLACK;
            display.display();
            subBreathCC = 0;
            if (readSetting(BREATH_CC_ADDR) != breathCC) {
              writeSetting(BREATH_CC_ADDR,breathCC);
              midiReset();
            }
            break;
        }
      }
    } else if (subBreathAT) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotBreathAT(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotBreathAT(BLACK);
            breathAT=!breathAT;
            plotBreathAT(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotBreathAT(WHITE);
            cursorNow = BLACK;
            display.display();
            subBreathAT = 0;
            if (readSetting(BREATH_AT_ADDR) != breathAT){
              writeSetting(BREATH_AT_ADDR,breathAT);
              midiReset();
            }
            break;
          case 4:
            // up
            plotBreathAT(BLACK);
            breathAT=!breathAT;
            plotBreathAT(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotBreathAT(WHITE);
            cursorNow = BLACK;
            display.display();
            subBreathAT = 0;
            if (readSetting(BREATH_AT_ADDR) != breathAT){
              writeSetting(BREATH_AT_ADDR,breathAT);
              midiReset();
            }
            break;
        }
      }
    } else if (subVelocity) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelocity(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotVelocity(BLACK);
            if (velocity > 0){
              velocity--;
            } else velocity = 127;
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelocity = 0;
            if (readSetting(VELOCITY_ADDR) != velocity) writeSetting(VELOCITY_ADDR,velocity);
            break;
          case 4:
            // up
            plotVelocity(BLACK);
            if (velocity < 127){
              velocity++;
            } else velocity = 0;
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelocity = 0;
            if (readSetting(VELOCITY_ADDR) != velocity) writeSetting(VELOCITY_ADDR,velocity);
            break;
        }
      }


    } else if (subCurve) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotCurve(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotCurve(BLACK);
            if (curve > 0){
              curve--;
            } else curve = 12;
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            subCurve = 0;
            if (readSetting(BREATHCURVE_ADDR) != curve) writeSetting(BREATHCURVE_ADDR,curve);
            break;
          case 4:
            // up
            plotCurve(BLACK);
            if (curve < 12){
              curve++;
            } else curve = 0;
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            subCurve = 0;
            if (readSetting(BREATHCURVE_ADDR) != curve) writeSetting(BREATHCURVE_ADDR,curve);
            break;
        }
      }

    } else if (subVelSmpDl) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelSmpDl(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotVelSmpDl(BLACK);
            if (velSmpDl > 0){
              velSmpDl-=1;
            } else velSmpDl = 30;
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelSmpDl = 0;
            if (readSetting(VEL_SMP_DL_ADDR) != velSmpDl) writeSetting(VEL_SMP_DL_ADDR,velSmpDl);
            break;
          case 4:
            // up
            plotVelSmpDl(BLACK);
            if (velSmpDl < 30){
              velSmpDl+=1;
            } else velSmpDl = 0;
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelSmpDl = 0;
            if (readSetting(VEL_SMP_DL_ADDR) != velSmpDl) writeSetting(VEL_SMP_DL_ADDR,velSmpDl);
            break;
        }
      }

     } else if (subVelBias) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelBias(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotVelBias(BLACK);
            if (velBias > 0){
              velBias--;
            } else velBias = 9;
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelBias = 0;
            if (readSetting(VEL_BIAS_ADDR) != velBias) writeSetting(VEL_BIAS_ADDR,velBias);
            break;
          case 4:
            // up
            plotVelBias(BLACK);
            if (velBias < 9){
              velBias++;
            } else velBias = 0;
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelBias = 0;
            if (readSetting(VEL_BIAS_ADDR) != velBias) writeSetting(VEL_BIAS_ADDR,velBias);
            break;
        }
      }

    } else {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(setupBrMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (setupBrMenuCursor < 6){
              drawMenuCursor(setupBrMenuCursor, BLACK);
              setupBrMenuCursor++;
              drawMenuCursor(setupBrMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 2:
            // enter
            selectSetupBrMenu();
            break;
          case 4:
            // up
            if (setupBrMenuCursor > 1){
              drawMenuCursor(setupBrMenuCursor, BLACK);
              setupBrMenuCursor--;
              drawMenuCursor(setupBrMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 8:
            // menu
            state = MAIN_MENU;
            stateFirstRun = 1;
            break;
        }
      }
    }


  } else if (state == SETUP_CT_MENU) {  // SETUP CONTROLLERS MENU HERE <<<<<<<<<<<<<
   if (stateFirstRun) {
      drawSetupCtMenuScreen();
      stateFirstRun = 0;
    }
    if (subPort){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPort(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotPort(BLACK);
            if (portamento > 0){
              portamento--;
            } else portamento = 2;
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            subPort = 0;
            if (readSetting(PORTAM_ADDR) != portamento) writeSetting(PORTAM_ADDR,portamento);
            break;
          case 4:
            // up
            plotPort(BLACK);
            if (portamento < 2){
              portamento++;
            } else portamento = 0;
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            subPort = 0;
            if (readSetting(PORTAM_ADDR) != portamento) writeSetting(PORTAM_ADDR,portamento);
            break;
        }
      }
    } else if (subPB) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPB(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (PBdepth > 0){
              plotPB(BLACK);
              PBdepth--;
              plotPB(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotPB(WHITE);
            cursorNow = BLACK;
            display.display();
            subPB = 0;
            if (readSetting(PB_ADDR) != PBdepth) writeSetting(PB_ADDR,PBdepth);
            break;
          case 4:
            // up
            if (PBdepth < 12){
              plotPB(BLACK);
              PBdepth++;
              plotPB(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotPB(WHITE);
            cursorNow = BLACK;
            display.display();
            subPB = 0;
            if (readSetting(PB_ADDR) != PBdepth) writeSetting(PB_ADDR,PBdepth);
            break;
        }
      }
    } else if (subExtra) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotExtra(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotExtra(BLACK);
            if (extraCT > 0){
              extraCT--;
            } else extraCT = 4;
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            subExtra = 0;
            if (readSetting(EXTRA_ADDR) != extraCT) writeSetting(EXTRA_ADDR,extraCT);
            break;
          case 4:
            // up
            plotExtra(BLACK);
            if (extraCT < 4){
              extraCT++;
            } else extraCT = 0;
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            subExtra = 0;
            if (readSetting(EXTRA_ADDR) != extraCT) writeSetting(EXTRA_ADDR,extraCT);
            break;
        }
      }
    } else if (subDeglitch) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotDeglitch(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (deglitch > 0){
              plotDeglitch(BLACK);
              deglitch-=1;
              plotDeglitch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotDeglitch(WHITE);
            cursorNow = BLACK;
            display.display();
            subDeglitch = 0;
            if (readSetting(DEGLITCH_ADDR) != deglitch) writeSetting(DEGLITCH_ADDR,deglitch);
            break;
          case 4:
            // up
            if (deglitch < 70){
              plotDeglitch(BLACK);
              deglitch+=1;
              plotDeglitch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotDeglitch(WHITE);
            cursorNow = BLACK;
            display.display();
            subDeglitch = 0;
            if (readSetting(DEGLITCH_ADDR) != deglitch) writeSetting(DEGLITCH_ADDR,deglitch);
            break;
        }
      }
    } else if (subPinky) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPinkyKey(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (pinkySetting > 0){
              plotPinkyKey(BLACK);
              pinkySetting-=1;
              plotPinkyKey(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotPinkyKey(WHITE);
            cursorNow = BLACK;
            display.display();
            subPinky = 0;
            if (readSetting(PINKY_KEY_ADDR) != pinkySetting) writeSetting(PINKY_KEY_ADDR,pinkySetting);
            break;
          case 4:
            // up
            if (pinkySetting < 24){
              plotPinkyKey(BLACK);
              pinkySetting+=1;
              plotPinkyKey(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotPinkyKey(WHITE);
            cursorNow = BLACK;
            display.display();
            subPinky = 0;
            if (readSetting(PINKY_KEY_ADDR) != pinkySetting) writeSetting(PINKY_KEY_ADDR,pinkySetting);
            break;
        }
      }
    } else {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(setupCtMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (setupCtMenuCursor < 6){
              drawMenuCursor(setupCtMenuCursor, BLACK);
              setupCtMenuCursor++;
              drawMenuCursor(setupCtMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 2:
            // enter
            selectSetupCtMenu();
            break;
          case 4:
            // up
            if (setupCtMenuCursor > 1){
              drawMenuCursor(setupCtMenuCursor, BLACK);
              setupCtMenuCursor--;
              drawMenuCursor(setupCtMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 8:
            // menu
            state = MAIN_MENU;
            stateFirstRun = 1;
            break;
        }
      }
    }





  } else if (state == VIBRATO_MENU) {  // VIBRATO MENU HERE <<<<<<<<<<<<<
   if (stateFirstRun) {
      drawVibratoMenuScreen();
      stateFirstRun = 0;
    }
    if (subVibrato) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibrato(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (vibrato > 0){
              plotVibrato(BLACK);
              vibrato--;
              plotVibrato(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotVibrato(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibrato = 0;
            if (readSetting(VIBRATO_ADDR) != vibrato) writeSetting(VIBRATO_ADDR,vibrato);
            break;
          case 4:
            // up
            if (vibrato < 9){
              plotVibrato(BLACK);
              vibrato++;
              plotVibrato(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotVibrato(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibrato = 0;
            if (readSetting(VIBRATO_ADDR) != vibrato) writeSetting(VIBRATO_ADDR,vibrato);
            break;
        }
      }
    } else if (subVibSens) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibSens(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (vibSens > 1){
              plotVibSens(BLACK);
              vibSens--;
              plotVibSens(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotVibSens(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibSens = 0;
            if (readSetting(VIB_SENS_ADDR) != vibSens) writeSetting(VIB_SENS_ADDR,vibSens);
            break;
          case 4:
            // up
            if (vibSens < 12){
              plotVibSens(BLACK);
              vibSens++;
              plotVibSens(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotVibSens(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibSens = 0;
            if (readSetting(VIB_SENS_ADDR) != vibSens) writeSetting(VIB_SENS_ADDR,vibSens);
            break;
        }
      }
    } else if (subVibRetn) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibRetn(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotVibRetn(BLACK);
            if (vibRetn > 0){
              vibRetn--;
            }
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibRetn = 0;
            if (readSetting(VIB_RETN_ADDR) != vibRetn) writeSetting(VIB_RETN_ADDR,vibRetn);
            break;
          case 4:
            // up
            plotVibRetn(BLACK);
            if (vibRetn < 4){
              vibRetn++;
            }
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibRetn = 0;
            if (readSetting(VIB_RETN_ADDR) != vibRetn) writeSetting(VIB_RETN_ADDR,vibRetn);
            break;
        }
      }
    } else if (subVibSquelch) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibSquelch(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (vibSquelch > 1){
              plotVibSquelch(BLACK);
              vibSquelch--;
              plotVibSquelch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotVibSquelch(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibSquelch = 0;
            if (readSetting(VIB_SQUELCH_ADDR) != vibSquelch) writeSetting(VIB_SQUELCH_ADDR,vibSquelch);
            break;
          case 4:
            // up
            if (vibSquelch < 30){
              plotVibSquelch(BLACK);
              vibSquelch++;
              plotVibSquelch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotVibSquelch(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibSquelch = 0;
            if (readSetting(VIB_SQUELCH_ADDR) != vibSquelch) writeSetting(VIB_SQUELCH_ADDR,vibSquelch);
            break;
        }
      }
    } else if (subVibDirection) {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibDirection(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotVibDirection(BLACK);
            vibDirection = !vibDirection;
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibDirection = 0;
            if (readSetting(VIB_DIRECTION_ADDR) != vibDirection) writeSetting(VIB_DIRECTION_ADDR,vibDirection);
            break;
          case 4:
            // up
            plotVibDirection(BLACK);
            vibDirection = !vibDirection;
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibDirection = 0;
            if (readSetting(VIB_DIRECTION_ADDR) != vibDirection) writeSetting(VIB_DIRECTION_ADDR,vibDirection);
            break;
        }
      }
    } else {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(vibratoMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (vibratoMenuCursor < 5){
              drawMenuCursor(vibratoMenuCursor, BLACK);
              vibratoMenuCursor++;
              drawMenuCursor(vibratoMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 2:
            // enter
            selectVibratoMenu();
            break;
          case 4:
            // up
            if (vibratoMenuCursor > 1){
              drawMenuCursor(vibratoMenuCursor, BLACK);
              vibratoMenuCursor--;
              drawMenuCursor(vibratoMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 8:
            // menu
            state = SETUP_CT_MENU;
            stateFirstRun = 1;
            break;
        }
      }
    }
  }



}

void selectMainMenu(){
  switch (mainMenuCursor){
    case 1:
      subTranspose = 1;
      drawMenuCursor(mainMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubTranspose();
      break;
    case 2:
      subOctave = 1;
      drawMenuCursor(mainMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubOctave();
      break;
    case 3:
      subMIDI = 1;
      drawMenuCursor(mainMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubMIDI();
      break;
    case 4:
      state = BREATH_ADJ_IDL;
      stateFirstRun = 1;
      break;
    case 5:
      state = SETUP_BR_MENU;
      stateFirstRun = 1;
      break;
    case 6:
      state = SETUP_CT_MENU;
      stateFirstRun = 1;
      break;
  }
}

void selectRotatorMenu(){
  switch (rotatorMenuCursor){
    case 1:
      subParallel = 1;
      drawMenuCursor(rotatorMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubRotator();
      break;
    case 2:
      subRotator = 1;
      drawMenuCursor(rotatorMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubRotator();
      break;
    case 3:
      subRotator = 2;
      drawMenuCursor(rotatorMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubRotator();
      break;
    case 4:
      subRotator = 3;
      drawMenuCursor(rotatorMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubRotator();
      break;
    case 5:
      subRotator = 4;
      drawMenuCursor(rotatorMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubRotator();
      break;
    case 6:
      subPriority = 1;
      drawMenuCursor(rotatorMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubPriority();
      break;
  }
}

void selectSetupBrMenu(){
  switch (setupBrMenuCursor){
    case 1:
      subBreathCC = 1;
      drawMenuCursor(setupBrMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubBreathCC();
      break;
    case 2:
      subBreathAT = 1;
      drawMenuCursor(setupBrMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubBreathAT();
      break;
    case 3:
      subVelocity = 1;
      drawMenuCursor(setupBrMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVelocity();
      break;
    case 4:
      subCurve = 1;
      drawMenuCursor(setupBrMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubCurve();
      break;
    case 5:
      subVelSmpDl = 1;
      drawMenuCursor(setupBrMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVelSmpDl();
      break;
    case 6:
      subVelBias = 1;
      drawMenuCursor(setupBrMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVelBias();
      break;
  }
}

void selectSetupCtMenu(){
  switch (setupCtMenuCursor){
    case 1:
      subPort = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubPort();
      break;
    case 2:
      subPB = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubPB();
      break;
    case 3:
      subExtra = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubExtra();
      break;
    case 4:
      //subVibrato = 1;
      //drawMenuCursor(setupCtMenuCursor, WHITE);
      //display.display();
      //cursorBlinkTime = millis();
      //drawSubVibrato();
      state = VIBRATO_MENU;
      stateFirstRun = 1;
      break;
    case 5:
      subDeglitch = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubDeglitch();
      break;
    case 6:
      subPinky = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubPinkyKey();
  }
}

void selectVibratoMenu(){
  switch (vibratoMenuCursor){
    case 1:
      subVibrato = 1;
      drawMenuCursor(vibratoMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVibrato();
      break;
    case 2:
      subVibSens = 1;
      drawMenuCursor(vibratoMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVibSens();
      break;
    case 3:
      subVibRetn = 1;
      drawMenuCursor(vibratoMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVibRetn();
      break;
    case 4:
      subVibSquelch = 1;
      drawMenuCursor(vibratoMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVibSquelch();
      break;
    case 5:
      subVibDirection = 1;
      drawMenuCursor(vibratoMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVibDirection();
      break;
  }
}


void drawBreathScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25,2);
  display.println("BREATH");
  //display.drawLine(0,10,127,10,WHITE);
  display.setCursor(0,20);
  display.println("THR");
  display.drawLine(25,17,120,17,WHITE);
  display.drawLine(25,18,25,19,WHITE);
  display.drawLine(120,18,120,19,WHITE);
  display.drawLine(25,29,120,29,WHITE);
  display.drawLine(25,27,25,28,WHITE);
  display.drawLine(120,27,120,28,WHITE);
  display.setCursor(0,35);
  display.println("SNS");
  //display.drawLine(25,38,120,38,WHITE);
  display.drawLine(25,36,25,40,WHITE);
  display.drawLine(120,36,120,40,WHITE);
  display.setCursor(0,50);
  display.println("MAX");
  display.drawLine(25,47,120,47,WHITE);
  display.drawLine(25,48,25,49,WHITE);
  display.drawLine(120,48,120,49,WHITE);
  display.drawLine(25,60,120,60,WHITE);
  display.drawLine(25,58,25,59,WHITE);
  display.drawLine(120,58,120,59,WHITE);

  //display.drawLine(38,20,38,26,WHITE); // indikation thr
  pos1 = map(breathThrVal, breathLoLimit, breathHiLimit, 27, 119);
  display.drawLine(pos1,20,pos1,26,WHITE);
  cursorNow = WHITE;
  //display.drawLine(115,50,115,57,WHITE); // indikation max
  pos2 = map(breathMaxVal, breathLoLimit, breathHiLimit, 27, 119);
  display.drawLine(pos2,50,pos2,57,WHITE);
  //display.drawPixel(34, 38, WHITE);
  drawAdjCursor(WHITE);
  display.display();
}

void drawPortamScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25,2);
  display.println("PORTAMENTO");
  //display.drawLine(0,10,127,10,WHITE);
  display.setCursor(0,20);
  display.println("THR");
  display.drawLine(25,17,120,17,WHITE);
  display.drawLine(25,18,25,19,WHITE);
  display.drawLine(120,18,120,19,WHITE);
  display.drawLine(25,29,120,29,WHITE);
  display.drawLine(25,27,25,28,WHITE);
  display.drawLine(120,27,120,28,WHITE);
  display.setCursor(0,35);
  display.println("SNS");
  //display.drawLine(25,38,120,38,WHITE);
  display.drawLine(25,36,25,40,WHITE);
  display.drawLine(120,36,120,40,WHITE);
  display.setCursor(0,50);
  display.println("MAX");
  display.drawLine(25,47,120,47,WHITE);
  display.drawLine(25,48,25,49,WHITE);
  display.drawLine(120,48,120,49,WHITE);
  display.drawLine(25,60,120,60,WHITE);
  display.drawLine(25,58,25,59,WHITE);
  display.drawLine(120,58,120,59,WHITE);

  //display.drawLine(38,20,38,26,WHITE); // indikation thr
  pos1 = map(portamThrVal, portamLoLimit, portamHiLimit, 27, 119);
  display.drawLine(pos1,20,pos1,26,WHITE);
  cursorNow = WHITE;
  //display.drawLine(115,50,115,57,WHITE); // indikation max
  pos2 = map(portamMaxVal, portamLoLimit, portamHiLimit, 27, 119);
  display.drawLine(pos2,50,pos2,57,WHITE);
  //display.drawPixel(34, 38, WHITE);
  drawAdjCursor(WHITE);
  display.display();
}

void drawPitchbScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25,2);
  display.println("PITCH BEND");
  //display.drawLine(0,10,127,10,WHITE);
  display.setCursor(0,20);
  display.println("THR");
  display.drawLine(25,17,120,17,WHITE);
  display.drawLine(25,18,25,19,WHITE);
  display.drawLine(120,18,120,19,WHITE);
  display.drawLine(25,29,120,29,WHITE);
  display.drawLine(25,27,25,28,WHITE);
  display.drawLine(120,27,120,28,WHITE);
  display.setCursor(0,35);
  display.println("SNS");
  //display.drawLine(25,38,120,38,WHITE);
  display.drawLine(25,36,25,40,WHITE);
  display.drawLine(120,36,120,40,WHITE);
  display.setCursor(0,50);
  display.println("MAX");
  display.drawLine(25,47,120,47,WHITE);
  display.drawLine(25,48,25,49,WHITE);
  display.drawLine(120,48,120,49,WHITE);
  display.drawLine(25,60,120,60,WHITE);
  display.drawLine(25,58,25,59,WHITE);
  display.drawLine(120,58,120,59,WHITE);

  //display.drawLine(38,20,38,26,WHITE); // indikation thr
  pos1 = map(pitchbThrVal, pitchbLoLimit, pitchbHiLimit, 27, 119);
  display.drawLine(pos1,20,pos1,26,WHITE);
  cursorNow = WHITE;
  //display.drawLine(115,50,115,57,WHITE); // indikation max
  pos2 = map(pitchbMaxVal, pitchbLoLimit, pitchbHiLimit, 27, 119);
  display.drawLine(pos2,50,pos2,57,WHITE);
  //display.drawPixel(34, 38, WHITE);
  drawAdjCursor(WHITE);
  display.display();
}

void drawExtracScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25,2);
  display.println("EXTRA CONTROLLER");
  //display.drawLine(0,10,127,10,WHITE);
  display.setCursor(0,20);
  display.println("THR");
  display.drawLine(25,17,120,17,WHITE);
  display.drawLine(25,18,25,19,WHITE);
  display.drawLine(120,18,120,19,WHITE);
  display.drawLine(25,29,120,29,WHITE);
  display.drawLine(25,27,25,28,WHITE);
  display.drawLine(120,27,120,28,WHITE);
  display.setCursor(0,35);
  display.println("SNS");
  //display.drawLine(25,38,120,38,WHITE);
  display.drawLine(25,36,25,40,WHITE);
  display.drawLine(120,36,120,40,WHITE);
  display.setCursor(0,50);
  display.println("MAX");
  display.drawLine(25,47,120,47,WHITE);
  display.drawLine(25,48,25,49,WHITE);
  display.drawLine(120,48,120,49,WHITE);
  display.drawLine(25,60,120,60,WHITE);
  display.drawLine(25,58,25,59,WHITE);
  display.drawLine(120,58,120,59,WHITE);

  //display.drawLine(38,20,38,26,WHITE); // indikation thr
  pos1 = map(extracThrVal, extracLoLimit, extracHiLimit, 27, 119);
  display.drawLine(pos1,20,pos1,26,WHITE);
  cursorNow = WHITE;
  //display.drawLine(115,50,115,57,WHITE); // indikation max
  pos2 = map(extracMaxVal, extracLoLimit, extracHiLimit, 27, 119);
  display.drawLine(pos2,50,pos2,57,WHITE);
  //display.drawPixel(34, 38, WHITE);
  drawAdjCursor(WHITE);
  display.display();
}


void drawCtouchScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25,2);
  display.println("TOUCH SENSE");
  //display.drawLine(0,10,127,10,WHITE);
  display.setCursor(0,20);
  display.println("THR");
  display.drawLine(25,17,120,17,WHITE);
  display.drawLine(25,18,25,19,WHITE);
  display.drawLine(120,18,120,19,WHITE);
  display.drawLine(25,29,120,29,WHITE);
  display.drawLine(25,27,25,28,WHITE);
  display.drawLine(120,27,120,28,WHITE);
  display.setCursor(0,35);
  display.println("SNS");
  //display.drawLine(25,38,120,38,WHITE);
  display.drawLine(25,36,25,40,WHITE);
  display.drawLine(120,36,120,40,WHITE);

  //display.drawLine(38,20,38,26,WHITE); // indikation thr
  pos1 = map(ctouchThrVal, ctouchLoLimit, ctouchHiLimit, 27, 119);
  display.drawLine(pos1,20,pos1,26,WHITE);
  cursorNow = WHITE;

  //display.drawPixel(34, 38, WHITE);
  drawAdjCursor(WHITE);
  display.display();
}

void drawMenuCursor(byte itemNo, byte color){
  byte xmid = 6 + 9 * itemNo;
  display.drawTriangle(57,xmid,61,xmid+2,61,xmid-2,color);
}

void drawAdjCursor(byte color){
  display.drawTriangle(16,4,20,4,18,1,color);
  display.drawTriangle(16,6,20,6,18,9,color);
}

void drawMenuScreen(){

  //Construct the title including voltage reading.
  //Involves intricate splicing of the title string with battery voltage
  char menuTitle[] = "MENU         XXX Y.Y "; //Allocate string buffer of appropriate size with some placeholders
  char* splice1 = menuTitle + 13;
  char* splice2 = menuTitle + 17;

  int vMeterReading = analogRead(vMeterPin);
  memcpy(splice1, (vMeterReading > 3000) ? "USB" : "BAT", 3);
  if (vMeterReading < 2294) {
    memcpy(splice2, "LOW ", 3);
  } else {
    double voltage = map(vMeterReading,0,3030,0,50)*0.1;
    dtostrf(voltage, 3, 1, splice2);
    splice2[3]='V'; //Put the V at the end (last char of buffer before \0)
  }

  drawMenu(menuTitle, mainMenuCursor, 6,
    "TRANSPOSE",
    "OCTAVE",
    "MIDI CH",
    "ADJUST",
    "SETUP BR",
    "SETUP CTL");
}

void drawRotatorMenuScreen(){
  drawMenu("ROTATOR SETUP", rotatorMenuCursor, 6,
    "PARALLEL",
    "ROTATE 1",
    "ROTATE 2",
    "ROTATE 3",
    "ROTATE 4",
    "PRIORITY");
}

void drawPatchView(){
  display.clearDisplay();
  if (FPD){
    drawTrills();
  }
  if (FPD < 2){
    display.setTextColor(WHITE);
    display.setTextSize(6);
    if (patch < 10){
      // 1-9
      display.setCursor(48,10);
    } else if (patch < 100){
      // 10-99
      display.setCursor(31,10);
    } else {
      // 99-128
      display.setCursor(10,10);
    }
    display.println(patch);
  } else if (FPD == 2){
    display.setTextColor(WHITE);
    display.setTextSize(6);
    display.setCursor(10,10);
    display.println("SET");
  } else {
    display.setTextColor(WHITE);
    display.setTextSize(6);
    display.setCursor(10,10);
    display.println("CLR");
  }
  display.display();
}

void drawTrills(){
  if (K5) display.fillRect(0,0,5,5,WHITE); else display.drawRect(0,0,5,5,WHITE);
  if (K6) display.fillRect(10,0,5,5,WHITE); else display.drawRect(10,0,5,5,WHITE);
  if (K7) display.fillRect(20,0,5,5,WHITE); else display.drawRect(20,0,5,5,WHITE);
}

void clearSub(){
  display.fillRect(63,11,64,52,BLACK);
}

void drawSubTranspose(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("TRANSPOSE");
  plotTranspose(WHITE);
  display.display();
}

void plotTranspose(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(80,33);
  if ((transpose-12) > -1){
    display.println("+");
    display.setCursor(93,33);
    display.println(transpose-12);
  } else {
    display.println("-");
    display.setCursor(93,33);
    display.println(abs(transpose-12));
  }
}
void drawSubRotator(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("SEMITONES");
  //plotRotator(WHITE,value);
  forceRedraw = 1;
  display.display();
}

void plotRotator(int color,int value){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(80,33);
  if ((value) > -1){
    display.println("+");
    display.setCursor(93,33);
    display.println(value);
  } else {
    display.println("-");
    display.setCursor(93,33);
    display.println(abs(value));
  }
}

void drawSubPriority(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("MONO PRIO");
  plotPriority(WHITE);
  display.display();
}

void plotPriority(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (priority){
    display.println("ROT");
  } else {
    display.println("MEL");
  }
}


void drawSubOctave(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(77,15);
  display.println("OCTAVE");
  plotOctave(WHITE);
  display.display();
}

void plotOctave(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(80,33);
  if ((octave-3) > -1){
    display.println("+");
    display.setCursor(93,33);
    display.println(octave-3);
  } else {
    display.println("-");
    display.setCursor(93,33);
    display.println(abs(octave-3));
  }
}

void drawSubMIDI(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("MIDI CHNL");
  plotMIDI(WHITE);
  display.display();
}

void plotMIDI(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(MIDIchannel);
  if (slowMidi){
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(BLACK);
  }
  display.setTextSize(1);
  display.setCursor(116,51);
  display.print("S");
}

void drawSubBreathCC(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("BREATH CC");
  plotBreathCC(WHITE);
  display.display();
}

void plotBreathCC(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  if (breathCC){
    switch (breathCC){
      case 1:
        display.setCursor(83,33);
        display.println("MW");
        break;
      case 2:
        display.setCursor(83,33);
        display.println("BR");
        break;
      case 3:
        display.setCursor(83,33);
        display.println("VL");
        break;
      case 4:
        display.setCursor(83,33);
        display.println("EX");
        break;
      case 5:
        display.setCursor(79,33);
        display.println("MW+");
        break;
      case 6:
        display.setCursor(79,33);
        display.println("BR+");
        break;
      case 7:
        display.setCursor(79,33);
        display.println("VL+");
        break;
      case 8:
        display.setCursor(79,33);
        display.println("EX+");
        break;
      case 9:
        display.setCursor(83,33);
        display.println("CF");
        break;
      case 10:
        display.setCursor(83,33);
        display.println("20");
      break;
    }
  } else {
    display.setCursor(79,33);
    display.println("OFF");
  }
}

void drawSubBreathAT(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("BREATH AT");
  plotBreathAT(WHITE);
  display.display();
}

void plotBreathAT(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (breathAT){
    display.println("ON");
  } else {
    display.println("OFF");
  }
}

void drawSubVelocity(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(71,15);
  display.println("VELOCITY");
  plotVelocity(WHITE);
  display.display();
}

void plotVelocity(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (velocity){
    display.println(velocity);
  } else {
    display.println("DYN");
  }
}


void drawSubCurve(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(80,15);
  display.println("CURVE");
  plotCurve(WHITE);
  display.display();
}

void plotCurve(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  switch (curve){
    case 0:
      display.setCursor(83,33);
      display.println("-4");
      break;
    case 1:
      display.setCursor(83,33);
      display.println("-3");
      break;
    case 2:
      display.setCursor(83,33);
      display.println("-2");
      break;
    case 3:
      display.setCursor(83,33);
      display.println("-1");
      break;
    case 4:
      display.setCursor(79,33);
      display.println("LIN");
      break;
    case 5:
      display.setCursor(83,33);
      display.println("+1");
      break;
    case 6:
      display.setCursor(83,33);
      display.println("+2");
      break;
    case 7:
      display.setCursor(83,33);
      display.println("+3");
      break;
    case 8:
      display.setCursor(83,33);
      display.println("+4");
      break;
    case 9:
      display.setCursor(83,33);
      display.println("S1");
      break;
    case 10:
      display.setCursor(83,33);
      display.println("S2");
      break;
    case 11:
      display.setCursor(83,33);
      display.println("Z1");
      break;
    case 12:
      display.setCursor(83,33);
      display.println("Z2");
      break;
  }
}


void drawSubPort(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(71,15);
  display.println("PORT/GLD");
  plotPort(WHITE);
  display.display();
}

void plotPort(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (portamento == 1){
    display.println("ON");
  } else if (portamento == 2){
    display.println("SW");
  } else {
    display.println("OFF");
  }
}

void drawSubPB(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("PITCHBEND");
  plotPB(WHITE);
  display.display();
}

void plotPB(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(76,33);
  if (PBdepth){
    display.println("1/");
    display.setCursor(101,33);
    display.println(PBdepth);
  } else {
    display.println("OFF");
  }
}

void drawSubExtra(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("EXTRA CTR");
  plotExtra(WHITE);
  display.display();
}

void plotExtra(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  switch (extraCT){
  case 0:
    display.setCursor(79,33);
    display.println("OFF");
    break;
  case 1:
    display.setCursor(83,33);
    display.println("MW");
    break;
  case 2:
    display.setCursor(83,33);
    display.println("FP");
    break;
  case 3:
    display.setCursor(83,33);
    display.println("CF");
    break;
  case 4:
    display.setCursor(83,33);
    display.println("SP");
    break;
  }
}

void drawSubVibrato(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibrato(WHITE);
  display.display();
}

void plotVibrato(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  if (vibrato){
    display.setCursor(90,33);
    display.println(vibrato);
  } else {
    display.setCursor(79,33);
    display.println("OFF");
  }
}

void drawSubVibSens(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibSens(WHITE);
  display.display();
}

void plotVibSens(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(vibSens);
}

void drawSubVibRetn(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibRetn(WHITE);
  display.display();
}

void plotVibRetn(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(vibRetn);
}

void drawSubVibSquelch(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibSquelch(WHITE);
  display.display();
}

void plotVibSquelch(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(83,33);
  display.println(vibSquelch);
}



void drawSubVibDirection(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("DIRECTION");
  plotVibDirection(WHITE);
  display.display();
}

void plotVibDirection(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (DNWD == vibDirection){
    display.println("NRM");
  } else {
    display.println("REV");
  }
}






void drawSubDeglitch(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(71,15);
  display.println("DEGLITCH");
  plotDeglitch(WHITE);
  display.display();
}

void plotDeglitch(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (deglitch){
    display.println(deglitch);
    display.setCursor(105,40);
    display.setTextSize(1);
    display.println("ms");
  } else {
    display.println("OFF");
  }
}
void drawSubPinkyKey(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(68,15);
  display.println("PINKY KEY");
  plotPinkyKey(WHITE);
  display.display();
}

void plotPinkyKey(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (pinkySetting < 12){
    display.println(pinkySetting - 12);
  } else if (pinkySetting == PBD) {
    display.println("PBD");
  } else {
    display.print("+");
    display.println(pinkySetting - 12);
  }
}
void drawSubVelSmpDl(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(69,15);
  display.println("VEL DELAY");
  plotVelSmpDl(WHITE);
  display.display();
}

void plotVelSmpDl(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (velSmpDl){
    display.println(velSmpDl);
    display.setCursor(105,40);
    display.setTextSize(1);
    display.println("ms");
  } else {
    display.println("OFF");
  }
}

void drawSubVelBias(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(72,15);
  display.println("VEL BIAS");
  plotVelBias(WHITE);
  display.display();
}

void plotVelBias(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  if (velBias){
    display.setCursor(90,33);
    display.println(velBias);
  } else {
    display.setCursor(79,33);
    display.println("OFF");
  }
}

void drawSetupBrMenuScreen(){
  drawMenu("SETUP BREATH", -1, 6, "BREATH CC", "BREATH AT", "VELOCITY", "CURVE", "VEL DELAY", "VEL BIAS");
}

void drawSetupCtMenuScreen(){
  drawMenu("SETUP CTRLS", -1, 6, "PORT/GLD", "PITCHBEND", "EXTRA CTR", "VIBRATO", "DEGLITCH", "PINKY KEY");
}

void drawVibratoMenuScreen(){
  drawMenu("VIBRATO", -1, 5, "DEPTH","SENSE","RETURN", "SQUELCH", "DIRECTION");
}

void drawSensorPixels(){
  int pos,oldpos;
  int redraw=0;
  if ((state == BREATH_ADJ_IDL) || (state == BREATH_ADJ_THR) || (state == BREATH_ADJ_MAX)){
    pos = map(constrain(pressureSensor, breathLoLimit, breathHiLimit), breathLoLimit, breathHiLimit, 28, 118);
    oldpos = map(constrain(lastPressure, breathLoLimit, breathHiLimit), breathLoLimit, breathHiLimit, 28, 118);
    if (pos!=oldpos){
      display.drawPixel(oldpos, 38, BLACK);
      display.drawPixel(pos, 38, WHITE);
      display.display();
    } else if (forcePix) {
      display.drawPixel(pos, 38, WHITE);
      display.display();
    }
    lastPressure=pressureSensor;
  }
    if ((state == PORTAM_ADJ_IDL) || (state == PORTAM_ADJ_THR) || (state == PORTAM_ADJ_MAX)){
    pos = map(constrain(biteSensor,portamLoLimit,portamHiLimit), portamLoLimit, portamHiLimit, 28, 118);
    oldpos = map(constrain(lastBite,portamLoLimit,portamHiLimit), portamLoLimit, portamHiLimit, 28, 118);
    if (pos!=oldpos){
      display.drawPixel(oldpos, 38, BLACK);
      display.drawPixel(pos, 38, WHITE);
      display.display();
    } else if (forcePix) {
      display.drawPixel(pos, 38, WHITE);
      display.display();
    }
    lastBite=biteSensor;
  }
  if ((state == PITCHB_ADJ_IDL) || (state == PITCHB_ADJ_THR) || (state == PITCHB_ADJ_MAX)){
    pos = map(constrain(pbUp, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    oldpos = map(constrain(lastPbUp, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    if (pos!=oldpos){
      display.drawPixel(oldpos, 38, BLACK);
      display.drawPixel(pos, 38, WHITE);
      redraw=1;
    } else if (forcePix) {
      display.drawPixel(pos, 38, WHITE);
      redraw=1;
    }
    pos = map(constrain(pbDn, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    oldpos = map(constrain(lastPbDn, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    if (pos!=oldpos){
      display.drawPixel(oldpos, 38, BLACK);
      display.drawPixel(pos, 38, WHITE);
      redraw=1;
    } else if (forcePix) {
      display.drawPixel(pos, 38, WHITE);
      redraw=1;
    }
    if (redraw){
      display.display();
      redraw=0;
    }
    lastPbUp=pbUp;
    lastPbDn=pbDn;
  }
  if ((state == EXTRAC_ADJ_IDL) || (state == EXTRAC_ADJ_THR) || (state == EXTRAC_ADJ_MAX)){
    pos = map(constrain(exSensor, extracLoLimit, extracHiLimit), extracLoLimit, extracHiLimit, 28, 118);
    oldpos = map(constrain(lastEx, extracLoLimit, extracHiLimit), extracLoLimit, extracHiLimit, 28, 118);
    if (pos!=oldpos){
      display.drawPixel(oldpos, 38, BLACK);
      display.drawPixel(pos, 38, WHITE);
      display.display();
    } else if (forcePix) {
      display.drawPixel(pos, 38, WHITE);
      display.display();
    }
    lastEx=exSensor;
  }
  if ((state == CTOUCH_ADJ_IDL) || (state == CTOUCH_ADJ_THR)){
    display.drawLine(28,38,118,38,BLACK);
    for (byte i=0; i<12; i++){
      pos = map(constrain(touchSensor.filteredData(i), ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 38, WHITE);
    }
    int posRead = map(touchRead(halfPitchBendKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    pos = map(constrain(posRead, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
    display.drawPixel(pos, 38, WHITE);
    posRead = map(touchRead(specialKeyPin),ttouchLoLimit,ttouchHiLimit,ctouchHiLimit,ctouchLoLimit);
    pos = map(constrain(posRead, ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
    display.drawPixel(pos, 38, WHITE);
    display.display();
  }
  forcePix = 0;
}

void writeSetting(byte address, unsigned short value){
  union {
    byte v[2];
    unsigned short val;
  } data;
  data.val = value;
  EEPROM.write(address, data.v[0]);
  EEPROM.write(address+1, data.v[1]);
}

unsigned short readSetting(byte address){
  union {
    byte v[2];
    unsigned short val;
  } data;
  data.v[0] = EEPROM.read(address);
  data.v[1] = EEPROM.read(address+1);
  return data.val;
}


//***********************************************************

int readTrills() {
  readSwitches();
  return K5+2*K6+4*K7;
}

//***********************************************************

void setFPS(int trills) {
  fastPatch[trills-1] = patch;
  writeSetting(FP1_ADDR+2*(trills-1),patch);
  FPD = 2;
}

//***********************************************************

void clearFPS(int trills) {
  fastPatch[trills-1] = 0;
  writeSetting(FP1_ADDR+2*(trills-1),0);
  FPD = 3;
}



/*
 * Draw a regular list of text menu items
 * header - first header line
 * selectedItem - the currently selected item (draw a triangle next to it). -1 for none. 1..nItems (1-based, 0 is header row)
 * nItems - number of menu items
 * ... - a list (nItems long) of text items to show
 */
void drawMenu(const char* header, byte selected, byte nItems, ...) {

  //Initialize display and draw menu header + line
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(header);
  display.drawLine(0,MENU_ROW_HEIGHT,127,MENU_ROW_HEIGHT, WHITE);

  va_list valist;
  va_start(valist, nItems);
  for(byte row=0; row<nItems; row++) {
    int rowPixel = (row+1)*MENU_ROW_HEIGHT + MENU_HEADER_OFFSET;
    const char* lineText = va_arg(valist, const char*);
    display.setCursor(0,rowPixel);
    display.println(lineText);
  }

  if(selected>=0) drawMenuCursor(selected, WHITE);

  va_end(valist);

  display.display();
}