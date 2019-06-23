#include "menu.h"
#include "hardware.h"
#include "config.h"
#include "globals.h"
#include "midi.h"

#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include "settings.h"
#include "numenu.h"

static uint8_t lastDeumButtons = 0;
static uint8_t deumButtonState = 0;
static byte buttonPressedAndNotUsed = 0;


enum CursorIdx {
  EMain,
  EBreath,
  EControl,
  ERotator,
  EVibrato,

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

static unsigned long lastDebounceTime = 0;         // the last time the output pin was toggled
static unsigned long buttonRepeatTime = 0;
static unsigned long buttonPressedTime = 0;
static unsigned long menuTime = 0;
static unsigned long patchViewTime = 0;
unsigned long cursorBlinkTime = 0;          // the last time the cursor was toggled

//Display state
static byte state = DISPLAYOFF_IDL;
static byte stateFirstRun = 1;

// The external function of subSquelch has been broken,
// need to come up with a smart way to make it work again.
// The status led was update when the Squelch menu was open.
byte subVibSquelch = 0; //extern

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


extern void readSwitches(void);
extern Adafruit_MPR121 touchSensor;

extern const MenuPage mainMenuPage; // Forward declaration.


#define OLED_RESET 4
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

void initDisplay() {

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  display.clearDisplay();
  display.drawBitmap(0,0,nuevi_logo_bmp,LOGO16_GLCD_WIDTH,LOGO16_GLCD_HEIGHT,1);
  display.display();

  memset(cursors, 0, sizeof(cursors));
  memset(offsets, 0, sizeof(offsets));
  memset(activeSub, 0, sizeof(activeSub));
}

void showVersion() {
  display.setTextColor(WHITE);
  display.setTextSize(1);
  #if defined(CASSIDY)
  display.setCursor(0,0);
  display.print("BC");
  #endif
  #if defined(CVSCALEBOARD)
  display.setCursor(15,0);
  display.print("CV");
  #endif
  display.setCursor(85,52);
  display.print("v.");
  display.println(FIRMWARE_VERSION);
  display.display();
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

static void plotSubOption(const char* label, int color) {
  display.setTextColor(color);
  display.setTextSize(2);
  int x_pos = 96-strlen(label)*6;
  display.setCursor(x_pos,33);
  display.println(label);
}

static bool drawSubMenu(const MenuPage &page, int color) {
    int index = cursors[page.cursor];
    // TODO: Null check subMenuFunc
    const MenuEntry* subEntry =  page.entries[index];
    switch(subEntry->type) {
      case MenuType::ESub:
        {
          char buffer[12];
          const char* labelPtr = nullptr;
          const MenuEntrySub* sub = (const MenuEntrySub*)subEntry;
          sub->getSubTextFunc(*sub, buffer, &labelPtr);

          // If ECustom flag is set, we assume that the getSubTextFunc 
          // rendered by it self.
          if( !(sub->flags & EMenuEntryCustom)) {
            plotSubOption(buffer, color);
            if(labelPtr != nullptr) {
              // TODO: handle this better, we should center text + label
              display.setCursor(105,40);
              display.setTextSize(1);
              display.println(labelPtr);
            }
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

static bool updateSubMenuCursor(const MenuPage &page, uint32_t timeNow)
{
  if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    cursorBlinkTime = timeNow;
    if (cursorNow == WHITE) {
      cursorNow = BLACK;
      clearSubValue();
    } else {
      cursorNow = WHITE;
      return drawSubMenu( page, cursorNow );
    }
  }
  return false;
}

static void plotMenuEntries(const MenuPage &page, bool clear = false) {
  int row = 0;
  if(clear) {
    display.fillRect( 0, MENU_HEADER_OFFSET, 56, 64-MENU_HEADER_OFFSET, BLACK );
  }
  for(int item = offsets[page.cursor]; (item < page.numEntries) && (row < MENU_NUM_ROWS); item++, row++) {
    int rowPixel = (row)*MENU_ROW_HEIGHT + MENU_HEADER_OFFSET;
    const char* lineText = page.entries[item]->title;
    display.setCursor(0,rowPixel);
    display.println(lineText);
  }
}

static void drawMenu(const MenuPage &page, const char* customTitle = nullptr) {
  //Initialize display and draw menu header + line
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if(customTitle)
    display.println(customTitle);
  else
    display.println(page.title);
  display.drawLine(0, MENU_ROW_HEIGHT, 127, MENU_ROW_HEIGHT, WHITE);

  plotMenuEntries(page);
}

static void drawMenuScreen() {
  //Construct the title including voltage reading.
  //Involves intricate splicing of the title string with battery voltage
  char menuTitle[] = "MENU         XXX Y.YV"; //Allocate string buffer of appropriate size with some placeholders
  char* splice1 = menuTitle + 13;
  char* splice2 = menuTitle + 17;

  int vMeterReading = analogRead(vMeterPin);
  memcpy(splice1, (vMeterReading > 3000) ? "USB" : "BAT", 3);
  if (vMeterReading < 2294) {
    memcpy(splice2, "LOW ", 4);
  } else {
    int voltage = map(vMeterReading,0,3030,0,50);
    splice2[0] = (voltage/10)+'0';
    splice2[2] = (voltage%10)+'0';
  }
  drawMenu(mainMenuPage, menuTitle);
}

static void drawTrills(){
  if (K5) display.fillRect(0,0,5,5,WHITE); else display.drawRect(0,0,5,5,WHITE);
  if (K6) display.fillRect(10,0,5,5,WHITE); else display.drawRect(10,0,5,5,WHITE);
  if (K7) display.fillRect(20,0,5,5,WHITE); else display.drawRect(20,0,5,5,WHITE);
}

static void drawPatchView(){
  display.clearDisplay();
  if (FPD){
    drawTrills();
  }
  display.setTextColor(WHITE);
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
  display.setTextColor(WHITE);
  display.setTextSize(1);
  int len = strlen(label);

  display.setCursor(95-len*3,15);
  display.println(label);
}

void drawMenuCursor(byte itemNo, byte color){
  byte ymid = 15 + 9 * itemNo;
  display.drawTriangle(57, ymid,61, ymid+2,61, ymid-2, color);
}

static void plotMIDI(int color) {
  char buff[7];
  numToString(MIDIchannel, buff);
  plotSubOption(buff, color);
  if (slowMidi && color) {
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(BLACK);
  }
  display.setTextSize(1);
  display.setCursor(116,51);
  display.print("S");
}

//***********************************************************

// TODO: Move these to a settings.cpp maybe?
void writeSetting(byte address, unsigned short value){
  union {
    byte v[2];
    unsigned short val;
  } data;
  data.val = value;
  EEPROM.update(address, data.v[0]);
  EEPROM.update(address+1, data.v[1]);
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

static int readTrills() {
  readSwitches();
  return K5+2*K6+4*K7;
}

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
// Main menu
const MenuEntrySub transposeMenu = {
  MenuType::ESub, "TRANSPOSE", "TRANSPOSE", &transpose, 0, 24, MenuEntryFlags::ENone, 
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(transpose - 12, out, true);
  },
  [](const MenuEntrySub &sub) { writeSetting(TRANSP_ADDR,*sub.valuePtr); }
  , nullptr
};

const MenuEntrySub octaveMenu = {
  MenuType::ESub, "OCTAVE", "OCTAVE", &octave, 0, 6, MenuEntryFlags::ENone, 
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    numToString(octave-3, out, true);
  },
  [](SubMenuRef __unused) { writeSetting(OCTAVE_ADDR,octave); }
  , nullptr
};

const MenuEntrySub midiMenu = {
  MenuType::ESub, "MIDI CH", "MIDI CHNL", &MIDIchannel, 1, 16, EMenuEntryCustom | EMenuEntryEnterHandler, 
  [](SubMenuRef __unused, char* __unused out, const char** __unused unit) {
    plotMIDI(WHITE);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(MIDI_ADDR, MIDIchannel); },
  []() -> bool { // On enter handler
    readSwitches();
    if (pinkyKey){
      slowMidi = !slowMidi;
      dipSwBits = dipSwBits ^ (1<<3);
      writeSetting(DIPSW_BITS_ADDR,dipSwBits);
      return false;
    } else {
      writeSetting(MIDI_ADDR, MIDIchannel);
      return true;
    }
  }
};

const MenuEntryStateCh adjustMenu     = { MenuType::EStateChange, "ADJUST", ADJUST_MENU };
const MenuEntryStateCh breathMenu     = { MenuType::EStateChange, "SETUP BR", SETUP_BR_MENU };
const MenuEntryStateCh controlMenu    = { MenuType::EStateChange, "SETUP CTL", SETUP_CT_MENU };

const MenuEntry* mainMenuEntries[] = {
  (MenuEntry*)&transposeMenu,
  (MenuEntry*)&octaveMenu,
  (MenuEntry*)&midiMenu,
  (MenuEntry*)&adjustMenu,
  (MenuEntry*)&breathMenu,
  (MenuEntry*)&controlMenu
};

const MenuPage mainMenuPage = {
  nullptr,
  EMenuPageRoot,
  CursorIdx::EMain,
  DISPLAYOFF_IDL,
  ARR_LEN(mainMenuEntries), mainMenuEntries
};


//***********************************************************
// Rotator menu

static void rotatorSave(const MenuEntrySub& __unused sub) {
  int16_t stored;
  for(int i = 0; i < 4; ++i) {
    stored = readSetting(ROTN1_ADDR+2*i);
    if(stored != rotations[i])
      writeSetting(ROTN1_ADDR+2*i,(rotations[i]));
  }
}

static void rotatorOptionGet(SubMenuRef sub, char *out, const char** __unused unit) {
  numToString((*sub.valuePtr) - 24, out, true);
}

static void parallelOptionGet(SubMenuRef __unused, char *out, const char** __unused unit) {
  numToString(parallel-24, out, true);
}

static void parallelSave(SubMenuRef __unused) {
  writeSetting(PARAL_ADDR, parallel);
}

const MenuEntrySub rotatorParaMenu  = {
  MenuType::ESub, "PARALLEL", "SEMITONES", &parallel, 0, 48, MenuEntryFlags::ENone,
  parallelOptionGet, parallelSave, nullptr
};

const MenuEntrySub rotator1Menu  = {
  MenuType::ESub, "ROTATE 1", "SEMITONES", &rotations[0], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotator2Menu  = {
  MenuType::ESub, "ROTATE 2", "SEMITONES", &rotations[1], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotator3Menu  = {
  MenuType::ESub, "ROTATE 3", "SEMITONES", &rotations[2], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

const MenuEntrySub rotator4Menu  = {
  MenuType::ESub, "ROTATE 4", "SEMITONES", &rotations[3], 0, 48, MenuEntryFlags::ENone,
  rotatorOptionGet, rotatorSave, nullptr
};

static void rotatorPrioOptionGet(SubMenuRef __unused, char* out, const char** __unused) {
    if (priority) strncpy(out, "ROT", 4);
    else strncpy(out, "MEL", 4);
}

static void rotatorPrioSave(SubMenuRef __unused) {
  writeSetting(PRIO_ADDR,priority);
}

const MenuEntrySub rotatorPrioMenu = {
  MenuType::ESub, "PRIORITY", "MONO PRIO", &priority, 0,1, MenuEntryFlags::EMenuEntryWrap, 
  rotatorPrioOptionGet, rotatorPrioSave, nullptr,
};

const MenuEntry* rotatorMenuEntries[] = {
  (MenuEntry*)&rotatorParaMenu,
  (MenuEntry*)&rotator1Menu,
  (MenuEntry*)&rotator2Menu,
  (MenuEntry*)&rotator3Menu,
  (MenuEntry*)&rotator4Menu,
  (MenuEntry*)&rotatorPrioMenu
};

const MenuPage rotatorMenuPage = {
  "ROTATOR SETUP",
  EMenuPageRoot,
  CursorIdx::ERotator,
  DISPLAYOFF_IDL,
  ARR_LEN(rotatorMenuEntries), rotatorMenuEntries
};

//***********************************************************
// Breath menu
const MenuEntrySub breathCCMenu = {
  MenuType::ESub, "BREATH CC", "BREATH CC", &breathCC, 0, 10, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* breathCCMenuLabels[] = { "OFF", "MW", "BR", "VL", "EX", "MW+",
                                            "BR+", "VL+", "EX+", "CF", "20" };
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

const MenuEntrySub velocityMenu = {
  MenuType::ESub, "VELOCITY",  "VELOCITY", &velocity, 0, 127, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if(velocity) numToString(velocity, out);
    else strncpy(out, "DYN", 4);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VELOCITY_ADDR,velocity); }
  , nullptr
};

const MenuEntrySub curveMenu = {
  MenuType::ESub, "CURVE", "CURVE", &curve, 0, 12, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    const char* curveMenuLabels[] = {"-4", "-3", "-2", "-1", "LIN", "+1", "+2",
                                         "+3", "+4", "S1", "S2", "Z1", "Z2" };
    strncpy(out, curveMenuLabels[curve], 4);
  },
  [](const MenuEntrySub & __unused sub){ writeSetting(BREATHCURVE_ADDR,curve); }
  , nullptr
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
  MenuType::ESub, "VEL BIAS", "VEL BIAS", &velBias, 0, 9, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused, char* out, const char** __unused unit) {
    if (velBias) numToString(velBias, out);
    else strncpy(out, "OFF", 4);
  },
  [](SubMenuRef __unused){ writeSetting(VEL_BIAS_ADDR,velBias); }
  , nullptr
};

const MenuEntry* breathMenuEntries[] = {
  (MenuEntry*)&breathCCMenu,
  (MenuEntry*)&breathATMenu,
  (MenuEntry*)&velocityMenu,
  (MenuEntry*)&curveMenu,
  (MenuEntry*)&velSmpDlMenu,
  (MenuEntry*)&velBiasMenu
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
const MenuEntrySub portMenu = {
  MenuType::ESub, "PORT/GLD", "PORT/GLD", &portamento, 0, 2, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char ** __unused unit) {
    const char* labs[] = { "OFF", "ON", "SW" };
    strncpy(out, labs[portamento], 4);
  },
  [](SubMenuRef __unused sub) { writeSetting(PORTAM_ADDR,portamento); }
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
  MenuType::ESub, "EXTRA CTR", "EXTRA CTR", &extraCT, 0,4, MenuEntryFlags::EMenuEntryWrap,
  [](SubMenuRef __unused,char* out, const char** __unused unit) {
    const char* extraMenuLabels[] = { "OFF", "MW", "FP", "CF", "SP" };
    strncpy(out, extraMenuLabels[extraCT], 12);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(EXTRA_ADDR,extraCT); }
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

const MenuEntrySub pinkyMenu = {
  MenuType::ESub, "PINKY KEY", "PINKY KEY", &pinkySetting, 0, 24, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
    if (pinkySetting == PBD)
      strncpy(textBuffer, "PBD", 4);
    else
      numToString(pinkySetting-12, textBuffer, true);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(PINKY_KEY_ADDR,pinkySetting); }
  , nullptr
};

const MenuEntry* controlMenuEntries[] = {
  (MenuEntry*)&portMenu,
  (MenuEntry*)&pitchBendMenu,
  (MenuEntry*)&extraMenu,
  (MenuEntry*)&vibratoSubMenu,
  (MenuEntry*)&deglitchMenu,
  (MenuEntry*)&pinkyMenu
};

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

const MenuEntrySub vibSenseMenu = {
  MenuType::ESub, "SENSE", "LEVEL", &vibSens, 1, 12, MenuEntryFlags::ENone,
  [](SubMenuRef __unused,char* textBuffer, const char** __unused unit) {
    numToString(vibSens, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_SENS_ADDR,vibSens); }
  , nullptr
};

const MenuEntrySub vibRetnMenu = {
  MenuType::ESub, "RETURN", "LEVEL", &vibRetn, 0, 4, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* textBuffer, const char** __unused unit) {
    numToString(vibRetn, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_RETN_ADDR,vibRetn); }
  , nullptr
};

const MenuEntrySub vibSquelchMenu = {
  MenuType::ESub, "SQUELCH", "LEVEL", &vibSquelch, 1, 30, MenuEntryFlags::ENone,
  [](SubMenuRef __unused, char* textBuffer, const char** __unused unit) {
    numToString(vibSquelch, textBuffer);
  },
  [](const MenuEntrySub & __unused sub) { writeSetting(VIB_SQUELCH_ADDR,vibSquelch); }
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
    (MenuEntry*)&vibDepthMenu,
    (MenuEntry*)&vibSenseMenu,
    (MenuEntry*)&vibRetnMenu,
    (MenuEntry*)&vibSquelchMenu,
    (MenuEntry*)&vibDirMenu
};

const MenuPage vibratoMenuPage = {
  "VIBRATO",
  0,
  CursorIdx::EVibrato,
  SETUP_CT_MENU,
  ARR_LEN(vibratorMenuEntries), vibratorMenuEntries
};


//***********************************************************

static bool selectMenuOption(const MenuPage &page) {
  int cursorPosition = cursors[page.cursor];
  const MenuEntry* menuEntry = page.entries[cursorPosition];
  cursorBlinkTime = millis();

  switch(menuEntry->type) {
    case MenuType::ESub:
      activeSub[page.cursor] = cursorPosition+1;
      drawMenuCursor(cursorPosition, WHITE);
      drawSubBox( ((const MenuEntrySub*)menuEntry)->subTitle);
      drawSubMenu(page, WHITE);
      return true;

    case MenuType::EStateChange:
      state = ((const MenuEntryStateCh*)menuEntry)->state;
      stateFirstRun = 1;
      break;
  }

  return false;
}


//***********************************************************

static bool updateSubMenu(const MenuPage &page, uint32_t timeNow) {

  bool redraw = false;
  bool redrawSubValue = false;
  if (buttonPressedAndNotUsed) {
    buttonPressedAndNotUsed = 0;

    int current_sub = activeSub[page.cursor] -1;

    if( current_sub < 0)
      return false;

    auto sub = (const MenuEntrySub*)page.entries[current_sub];
    uint16_t currentVal = *sub->valuePtr;

    switch (deumButtonState){
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
            activeSub[page.cursor] = 0;
          }
        } else {
          activeSub[page.cursor] = 0;
          sub->applyFunc(*sub);
        }
        break;

      case BTN_MENU:
        activeSub[page.cursor] = 0;
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
    redraw |= drawSubMenu(page, WHITE);
    cursorNow = BLACK;
    cursorBlinkTime = timeNow;
  }

  return redraw;
}

static bool updateMenuPage( const MenuPage &page, uint32_t timeNow ) {
  byte cursorPos = cursors[page.cursor];
  byte newPos = cursorPos;
  bool redraw = false;

  if (buttonPressedAndNotUsed) {
    int lastEntry = page.numEntries-1;

    buttonPressedAndNotUsed = 0;
    switch (deumButtonState) {
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
        state = page.parentPage;
        stateFirstRun = 1;
        break;
    }

    if(newPos != cursorPos) {
      drawMenuCursor(cursorPos, BLACK);
      drawMenuCursor(newPos, WHITE);
      cursorNow = BLACK;
      clearSub();
      redraw = true;
      cursors[page.cursor] = newPos;
    }
  } else if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    // Only need to update cursor blink if no buttons were pressed
    if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
    drawMenuCursor(cursorPos, cursorNow);
    redraw = true;
    cursorBlinkTime = timeNow;
  }

  return redraw;
}

static bool updatePage(const MenuPage &page, uint32_t timeNow) {
  if (stateFirstRun) {
    drawMenu(page);
    stateFirstRun = 0;
  }
  if (activeSub[page.cursor]) {
    return updateSubMenu(page, timeNow);
  } else {
    return updateMenuPage(page, timeNow);
  }
}




static bool updateSensorPixelsFlag = false;
void drawSensorPixels() {
  updateSensorPixelsFlag = true;
}

bool adjustPageUpdate(uint16_t buttonChanges, uint32_t timeNow) {
  // This is a hack to update touch_Thr is it was changed..
  int old_thr = ctouchThrVal;
  int result = updateAdjustMenu(timeNow, buttonChanges, stateFirstRun, updateSensorPixelsFlag);
  bool redraw = false;

  updateSensorPixelsFlag = false;
  stateFirstRun = 0;
  buttonPressedAndNotUsed = 0;

  if(result < 0) {
    // Go back to main menu
    state = MAIN_MENU;
    stateFirstRun = true;
  } else {
    redraw = result;
  }

  if( old_thr != ctouchThrVal) {
    touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
  }
  return redraw;
}
//***********************************************************
static void checkForPatchView(int buttons) {
  int trills = readTrills();

  switch (buttons){
    case BTN_MENU+BTN_DOWN:
      break;

    case BTN_MENU+BTN_ENTER:
      if (trills) {
        state = PATCH_VIEW;
        stateFirstRun = 1;
        setFPS(trills, patch);
      }
      break;

    case BTN_MENU+BTN_UP:
      if (trills) {
        state = PATCH_VIEW;
        stateFirstRun = 1;
        clearFPS(trills);
      }
      break;
  }
}

// This should be moved to a separate file/process that handles only led
static void statusBlink() {
  digitalWrite(statusLedPin,LOW);
  delay(150);
  digitalWrite(statusLedPin,HIGH);
  delay(150);
  digitalWrite(statusLedPin,LOW);
  delay(150);
  digitalWrite(statusLedPin,HIGH);
}

//***********************************************************

void menu() {
  unsigned long timeNow = millis();
  const MenuPage *currentPage = nullptr;

  bool redraw = stateFirstRun;
  // read the state of the switches
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
      deumButtonState = deumButtons;
      menuTime = timeNow;
      buttonPressedAndNotUsed = 1;
      buttonPressedTime = timeNow;
    }

    if (((deumButtons == 1) || (deumButtons == 4)) && (timeNow - buttonPressedTime > buttonRepeatDelay) && (timeNow - buttonRepeatTime > buttonRepeatInterval)){
      buttonPressedAndNotUsed = 1;
      buttonRepeatTime = timeNow;
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastDeumButtons = deumButtons;

  // shut off menu system if not used for a while (changes not stored by exiting a setting manually will not be stored in EEPROM)
  if (state && ((timeNow - menuTime) > menuTimeUp)) {
    state = DISPLAYOFF_IDL;
    stateFirstRun = 1;

    subVibSquelch = 0;
    memset(activeSub, 0, sizeof(activeSub));
  }

  if        (state == DISPLAYOFF_IDL) {
    if (stateFirstRun) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      stateFirstRun = 0;
    }
    if (buttonPressedAndNotUsed) {
      buttonPressedAndNotUsed = 0;
      int trills = readTrills();
      switch (deumButtonState){
        case BTN_UP: // fallthrough
        case BTN_DOWN:
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
          } else if (!trills) buttonPressedAndNotUsed = 1;
          state = PATCH_VIEW;
          stateFirstRun = 1;
          break;

        case BTN_ENTER:
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
          }
          state = PATCH_VIEW;
          stateFirstRun = 1;
          break;

        case BTN_MENU:
          if (pinkyKey && (exSensor >= ((extracThrVal+extracMaxVal)/2))) { // switch breath activated legacy settings on/off
            legacyBrAct = !legacyBrAct;
            dipSwBits = dipSwBits ^ (1<<2);
            writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            statusBlink();
          } else if ((exSensor >= ((extracThrVal+extracMaxVal)/2))) { // switch pb pad activated legacy settings control on/off
            legacy = !legacy;
            dipSwBits = dipSwBits ^ (1<<1);
            writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            statusBlink();
          } else if (pinkyKey && !specialKey){ //hold pinky key for rotator menu, and if too high touch sensing blocks regular menu, touching special key helps
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
  } else if (state == PATCH_VIEW) {
    if (stateFirstRun) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      drawPatchView();
      patchViewTime = timeNow;
      stateFirstRun = 0;
    }
    if ((timeNow - patchViewTime) > patchViewTimeUp) {
      state = DISPLAYOFF_IDL;
      stateFirstRun = 1;
      doPatchUpdate = 1;
      FPD = 0;
      writeSetting(PATCH_ADDR,patch);
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      patchViewTime = timeNow;
      int trills = readTrills();
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            writeSetting(PATCH_ADDR,patch);
          } else if (!trills){
            if (patch > 1){
              patch--;
            } else patch = 128;
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 0;
          }
          drawPatchView();
          redraw = true;
          break;
        case BTN_ENTER:
          // enter
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            drawPatchView();
            redraw = true;
          }
          break;
        case BTN_UP:
          // up
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            writeSetting(PATCH_ADDR,patch);
          } else if (!trills){
            if (patch < 128){
              patch++;
            } else patch = 1;
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 0;
          }
          drawPatchView();
          redraw = true;
          break;

        case BTN_MENU:
          if (FPD < 2){
            state = DISPLAYOFF_IDL;
            stateFirstRun = 1;
            doPatchUpdate = 1;
          }
          writeSetting(PATCH_ADDR,patch);
          FPD = 0;
          break;

        case BTN_MENU+BTN_ENTER:
            midiPanic();
            display.clearDisplay();
            display.setTextColor(WHITE);
            display.setTextSize(2);
            display.setCursor(35,15);
            display.println("DON'T");
            display.setCursor(35,30);
            display.println("PANIC");
            redraw = true;
            break;

          case BTN_MENU+BTN_ENTER+BTN_UP+BTN_DOWN:
          //all keys depressed, reboot to programming mode
          _reboot_Teensyduino_();
      }
    }
  } else if (state == MAIN_MENU) {    // MAIN MENU HERE <<<<<<<<<<<<<<<
    currentPage = &mainMenuPage;
    if (stateFirstRun) {
      drawMenuScreen();
      stateFirstRun = 0;
    }

    if(activeSub[currentPage->cursor]) {
      redraw |= updateSubMenu(*currentPage, timeNow);
    } else {
      bool hadButtons = buttonPressedAndNotUsed;
      redraw |= updateMenuPage(*currentPage, timeNow);
      if (hadButtons)
        checkForPatchView(deumButtonState);
    }
  } else if (state == ROTATOR_MENU) {    // ROTATOR MENU HERE <<<<<<<<<<<<<<<
    currentPage = &rotatorMenuPage;
    if (stateFirstRun) {
      drawMenu(*currentPage);
      stateFirstRun = 0;
    }
    if(activeSub[currentPage->cursor]) {
      redraw |= updateSubMenu(*currentPage, timeNow);
    } else {
      bool hadButtons = buttonPressedAndNotUsed;
      redraw |= updateMenuPage(*currentPage, timeNow);
      if (hadButtons)
        checkForPatchView(deumButtonState);
    }
  // end rotator menu

  } else if (state == ADJUST_MENU) {
    adjustPageUpdate(buttonPressedAndNotUsed ? deumButtonState : 0, timeNow);
  } else if (state == SETUP_BR_MENU) {  // SETUP BREATH MENU HERE <<<<<<<<<<<<<<
    redraw |= updatePage(breathMenuPage, timeNow);
  } else if (state == SETUP_CT_MENU) {  // SETUP CONTROLLERS MENU HERE <<<<<<<<<<<<<
    redraw |= updatePage(controlMenuPage, timeNow);
  } else if (state == VIBRATO_MENU) {   // VIBRATO MENU HERE <<<<<<<<<<<<<
    redraw |= updatePage(vibratoMenuPage, timeNow);
  }

  if(redraw) {
    display.display();
  }
}
