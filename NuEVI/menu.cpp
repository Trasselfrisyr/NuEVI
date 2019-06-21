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

#define ARR_LEN(a)  (sizeof (a) / sizeof (a[0]))

#define BTN_DOWN 1
#define BTN_ENTER 2
#define BTN_UP 4
#define BTN_MENU 8

// TODO: Ask Johan the reason for using this..
// static const uint16_t minOffset = 50;

static uint8_t lastDeumButtons = 0;
static uint8_t deumButtonState = 0;
static byte buttonPressedAndNotUsed = 0;

// Allocate some space for cursors

enum CursorIdx {
  EMain,
  EBreath,
  EControl,
  ERotator,
  EVibrato,

  // NEVER ADD 
  NUM_CURSORS
};

static byte cursors[CursorIdx::NUM_CURSORS];

static byte cursorNow;

static byte forceRedraw = 0;
static byte FPD = 0;

// Variables used for the adjust menu
static byte forcePix = 0;

static uint16_t pos1;
static uint16_t pos2;

static int16_t adjustOption = 0;
static int16_t adjustCurrent = 0;

static int16_t sensorPixelPos1 = -1;
static int16_t sensorPixelPos2 = -1;


// constants
static const unsigned long debounceDelay = 30;           // the debounce time; increase if the output flickers
static const unsigned long buttonRepeatInterval = 50;
static const unsigned long buttonRepeatDelay = 400;
static const unsigned long cursorBlinkInterval = 300;    // the cursor blink toggle interval time
static const unsigned long patchViewTimeUp = 2000;       // ms until patch view shuts off
static const unsigned long menuTimeUp = 60000;           // menu shuts off after one minute of button inactivity

static unsigned long lastDebounceTime = 0;         // the last time the output pin was toggled
static unsigned long buttonRepeatTime = 0;
static unsigned long buttonPressedTime = 0;
static unsigned long menuTime = 0;
static unsigned long patchViewTime = 0;
unsigned long cursorBlinkTime = 0;          // the last time the cursor was toggled

//Display state
static byte state = DISPLAYOFF_IDL;
static byte stateFirstRun = 1;

static byte subTranspose = 0;
static byte subOctave = 0;
static byte subMIDI = 0;
static byte subBreathCC = 0;
static byte subBreathAT = 0;
static byte subVelocity = 0;
static byte subCurve = 0;
static byte subPort = 0;
static byte subPB = 0;
static byte subExtra = 0;
static byte subVibrato = 0;
static byte subDeglitch = 0;
static byte subPinky = 0;
static byte subVelSmpDl = 0;
static byte subVelBias = 0;
static byte subParallel = 0;
static byte subRotator = 0;
static byte subPriority = 0;
static byte subVibSens = 0;
static byte subVibRetn = 0;
static byte subVibDirection = 0;

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

void drawMenuCursor(byte itemNo, byte color){
  byte ymid = 15 + 9 * itemNo;
  display.drawTriangle(57, ymid,61, ymid+2,61, ymid-2, color);
}


static bool drawSubMenu(const MenuPage &page, int color) {
    int index = cursors[page.cursor];
    // TODO: Make sure this is a MenuEntrySub
    // TODO: Handle MenuEntrySubRotator case
    // TODO: Null check subMenuFunc
    ((const MenuEntrySub*)page.entries[index])->subMenuFunc(color);
    return true;
}

static bool updateSubMenuCursor(const MenuPage &page, uint32_t timeNow)
{
  if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    if (cursorNow == WHITE) cursorNow = BLACK;
    else cursorNow = WHITE;
    cursorBlinkTime = timeNow;
    return drawSubMenu( page, cursorNow );
  }
  return false;
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

  int row = 0;
  for(int item = 0; (item < page.numEntries) && (row < 6); item++, row++) {
    int rowPixel = (row+1)*MENU_ROW_HEIGHT + MENU_HEADER_OFFSET;
    const char* lineText = page.entries[item]->title;
    display.setCursor(0,rowPixel);
    display.println(lineText);
  }
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
  display.display();
}

static void clearSub(){
  display.fillRect(63,11,64,52,BLACK);
}

static void clearSubValue() {
  display.fillRect(65, 24, 60, 37, BLACK);
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

static void plotTranspose(int color){
  int value = transpose - 12;
  const char *sign = (value < 0) ? "-":"+";
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(80,33);
  display.println(sign);
  display.setCursor(93,33);
  display.println(abs(value));
}

static void plotRotator(int color,int value){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(80,33);
  if ((value) > -1){
    display.println("+");
  } else {
    display.println("-");
  }
  display.setCursor(93,33);
  display.println(abs(value));
}

static void plotPriority(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(79,33);
  if (priority){
    display.println("ROT");
  } else {
    display.println("MEL");
  }
}

static void plotOctave(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(80,33);
  if ((octave-3) > -1){
    display.println("+");
  } else {
    display.println("-");
  }
  display.setCursor(93,33);
  display.println(abs(octave-3));
}

static void plotMIDI(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(MIDIchannel);
  if (slowMidi && color){
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(BLACK);
  }
  display.setTextSize(1);
  display.setCursor(116,51);
  display.print("S");
}

static void plotSubOption(const char* label, int color)
{
  display.setTextColor(color);
  display.setTextSize(2);
  int x_pos = 91-strlen(label)*4;
  display.setCursor(x_pos,33);
  display.println(label);
}

static void plotNum(int value, int color) {
  int s = 0;
  if(value > 99) s = 2*7;
  else if(value > 9) s = 1*7;
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90-s ,33);
  display.println(value);
}

static const char* breathCCMenuLabels[] = { "OFF", "MW", "BR", "VL", "EX", "MW+",
                                            "BR+", "VL+", "EX+", "CF", "20" };

static void plotBreathCC(int color){
  plotSubOption(breathCCMenuLabels[breathCC], color);
}


static void plotBreathAT(int color){
  if (breathAT){
    plotSubOption("ON", color);
  } else {
    plotSubOption("OFF", color);
  }
}


static void plotVelocity(int color){
  if (velocity){
    display.setTextColor(color);
    display.setTextSize(2);
    display.setCursor(79,33);
    display.println(velocity);
  } else {
    plotSubOption("DYN", color);
  }
}

static const char* curveMenuLabels[] = {"-4", "-3", "-2", "-1", "LIN", "+1", "+2",
                                         "+3", "+4", "S1", "S2", "Z1", "Z2" };


static void plotCurve(int color){
  // Assumes curve is in rage 0..12
  plotSubOption(curveMenuLabels[curve], color);
}

static void plotPort(int color){
  if (portamento == 1){
    plotSubOption("ON", color);
  } else if (portamento == 2){
    plotSubOption("SW", color);
  } else {
    plotSubOption("OFF", color);
  }
}

static void plotPB(int color){
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

static const char* extraMenuLabels[] = { "OFF", "MW", "FP", "CF", "SP" };

static void plotExtra(int color){
  plotSubOption(extraMenuLabels[extraCT], color);
}

static void plotVibrato(int color){
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

static void plotVibSens(int color){
  plotNum(vibSens, color);
}

static void plotVibRetn(int color){
  plotNum(vibRetn, color);
}

static void plotVibSquelch(int color){
  plotNum(vibSquelch, color);
}

static void plotVibDirection(int color){
  if (DNWD == vibDirection){
    plotSubOption("NRM", color);
  } else {
    plotSubOption("REV", color);
  }
}


static void plotDeglitch(int color){
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


static void plotPinkyKey(int color){
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


static void plotVelSmpDl(int color){
  display.setTextColor(color);
  display.setCursor(79,33);
  display.setTextSize(2);
  if (velSmpDl){
    display.println(velSmpDl);
    display.setCursor(105,40);
    display.setTextSize(1);
    display.println("ms");
  } else {
    display.println("OFF");
  }
}

static void plotVelBias(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  if (velBias){
    plotNum(velBias, color);
  } else {
    display.setCursor(79,33);
    display.println("OFF");
  }
}

static void drawSubRotator(int __unused color){
  // HACKY HACK ROTATOR MENU
  // drawSubBox("SEMITONES");
  //plotRotator(WHITE,value);
  forceRedraw = 1;
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
const MenuEntrySub transposeMenu      = { MenuType::ESub, "TRANSPOSE", "TRANSPOSE", &subTranspose, plotTranspose };
const MenuEntrySub octaveMenu         = { MenuType::ESub, "OCTAVE",    "OCTAVE",    &subOctave, plotOctave };
const MenuEntrySub midiMenu           = { MenuType::ESub, "MIDI CH",   "MIDI CHNL", &subMIDI, plotMIDI };
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
  CursorIdx::EMain,
  DISPLAYOFF_IDL,
  ARR_LEN(mainMenuEntries), mainMenuEntries
};

// Rotator menu
const MenuEntrySub rotatorParaMenu      = { MenuType::ESub, "PARALLEL", "SEMITONES", &subParallel, drawSubRotator };
const MenuEntrySubRotator rotator1Menu  = { MenuType::ESubRotator, "ROTATE 1", "SEMITONES", 1, &subRotator, drawSubRotator };
const MenuEntrySubRotator rotator2Menu  = { MenuType::ESubRotator, "ROTATE 2", "SEMITONES", 2, &subRotator, drawSubRotator };
const MenuEntrySubRotator rotator3Menu  = { MenuType::ESubRotator, "ROTATE 3", "SEMITONES", 3, &subRotator, drawSubRotator };
const MenuEntrySubRotator rotator4Menu  = { MenuType::ESubRotator, "ROTATE 4", "SEMITONES", 4, &subRotator, drawSubRotator };
const MenuEntrySub rotatorPrioMenu      = { MenuType::ESub, "PRIORITY", "MONO PRIO", &subPriority, plotPriority };

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
  CursorIdx::ERotator,
  DISPLAYOFF_IDL,
  ARR_LEN(rotatorMenuEntries), rotatorMenuEntries
};

// Breath menu
const MenuEntrySub breathCCMenu      = { MenuType::ESub, "BREATH CC", "BREATH CC",  &subBreathCC, plotBreathCC };
const MenuEntrySub breathATMenu      = { MenuType::ESub, "BREATH AT", "BREATH AT",  &subBreathAT, plotBreathAT };
const MenuEntrySub velocityMenu      = { MenuType::ESub, "VELOCITY",  "VELOCITY",   &subVelocity, plotVelocity };
const MenuEntrySub curveMenu         = { MenuType::ESub, "CURVE",     "CURVE",      &subCurve,    plotCurve };
const MenuEntrySub velSmpDlMenu      = { MenuType::ESub, "VEL DELAY", "VEL DELAY",  &subVelSmpDl, plotVelSmpDl };
const MenuEntrySub velBiasMenu       = { MenuType::ESub, "VEL BIAS",  "VEL BIAS",   &subVelBias,  plotVelBias };

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
  CursorIdx::EBreath,
  MAIN_MENU,
  ARR_LEN(breathMenuEntries), breathMenuEntries
};

// Control menu
const MenuEntrySub portMenu           = { MenuType::ESub, "PORT/GLD",   "PORT/GLD",   &subPort,   plotPort };
const MenuEntrySub pitchBendMenu      = { MenuType::ESub, "PITCHBEND",  "PITCHBEND",  &subPB,     plotPB };
const MenuEntrySub extraMenu          = { MenuType::ESub, "EXTRA CTR",  "EXTRA CTR",  &subExtra,  plotExtra };
const MenuEntryStateCh vibratoSubMenu = { MenuType::EStateChange, "VIBRATO", VIBRATO_MENU };
const MenuEntrySub deglitchMenu       = { MenuType::ESub, "DEGLITCH",   "DEGLITCH",   &subDeglitch, plotDeglitch };
const MenuEntrySub pinkyMenu          = { MenuType::ESub, "PINKY KEY",  "PINKY KEY",  &subPinky,   plotPinkyKey };

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
  CursorIdx::EControl,
  MAIN_MENU,
  ARR_LEN(controlMenuEntries), controlMenuEntries
};


// Vibrato menu
const MenuEntrySub vibDepthMenu          = { MenuType::ESub, "DEPTH",     "LEVEL",     &subVibrato, plotVibrato };
const MenuEntrySub vibSenseMenu          = { MenuType::ESub, "SENSE",     "LEVEL",     &subVibSens, plotVibSens };
const MenuEntrySub vibRetnMenu           = { MenuType::ESub, "RETURN",    "LEVEL",     &subVibRetn, plotVibRetn };
const MenuEntrySub vibSquelchMenu        = { MenuType::ESub, "SQUELCH",   "LEVEL",     &subVibSquelch, plotVibSquelch };
const MenuEntrySub vibDirMenu            = { MenuType::ESub, "DIRECTION", "DIRECTION", &subVibDirection, plotVibDirection };

const MenuEntry* vibratorMenuEntries[] = {
    (MenuEntry*)&vibDepthMenu,
    (MenuEntry*)&vibSenseMenu,
    (MenuEntry*)&vibRetnMenu,
    (MenuEntry*)&vibSquelchMenu,
    (MenuEntry*)&vibDirMenu
};

const MenuPage vibratoMenuPage = {
  "VIBRATO", 
  CursorIdx::EVibrato,
  SETUP_CT_MENU,
  ARR_LEN(vibratorMenuEntries), vibratorMenuEntries
};


//***********************************************************

const AdjustMenuEntry breathAdjustMenu  = {
  "BREATH",
  {
    { &breathThrVal, breathLoLimit, breathHiLimit },
    { &breathMaxVal, breathLoLimit, breathHiLimit }
  },
  [] (const AdjustMenuEntry& e) {
    writeSetting(BREATH_THR_ADDR, *e.entries[0].value);
    writeSetting(BREATH_MAX_ADDR, *e.entries[1].value);
  }
};

const AdjustMenuEntry portamentoAdjustMenu = {
  "PORTAMENTO", 
  {
    { &portamThrVal, portamLoLimit, portamHiLimit },
    { &portamMaxVal, portamLoLimit, portamHiLimit }
  }, 
  [] (const AdjustMenuEntry& e) {
    writeSetting(PORTAM_THR_ADDR, *e.entries[0].value);
    writeSetting(PORTAM_MAX_ADDR, *e.entries[1].value);
  }
};

const AdjustMenuEntry pitchBendAdjustMenu = {
  "PITCH BEND", 
  {
    { &pitchbThrVal, pitchbLoLimit, pitchbHiLimit },
    { &pitchbMaxVal, pitchbLoLimit, pitchbHiLimit }
  }, 
  [] (const AdjustMenuEntry& e) {
    writeSetting(PITCHB_THR_ADDR, *e.entries[0].value);
    writeSetting(PITCHB_MAX_ADDR, *e.entries[1].value);
  }
};

const AdjustMenuEntry extraSensorAdjustMenu = {
  "EXTRA CONTROLLER", 
  {
    { &extracThrVal, extracLoLimit, extracHiLimit },
    { &extracMaxVal, extracLoLimit, extracHiLimit }
  }, 
  [] (const AdjustMenuEntry& e) {
    writeSetting(EXTRAC_THR_ADDR, *e.entries[0].value);
    writeSetting(EXTRAC_MAX_ADDR, *e.entries[1].value);
  }
};

const AdjustMenuEntry ctouchAdjustMenu = {
  "TOUCH SENSE", 
  {
    { &ctouchThrVal, ctouchLoLimit, ctouchHiLimit },
    { nullptr, 0, 0 }
  }, 
  [] (const AdjustMenuEntry& e) {
    writeSetting(CTOUCH_THR_ADDR, *e.entries[0].value);
  }
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
  display.setTextColor(WHITE);
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

static bool updateAdjustLineCursor(uint32_t timeNow, uint16_t hPos, uint16_t vPos ) {
  if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
    if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
    display.drawLine(hPos, vPos,hPos, vPos+6, cursorNow);;
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

static bool updateAdjustMenu(uint32_t timeNow, uint8_t buttons) {
  bool redraw = false;
  const AdjustMenuEntry *currentMenu = adjustMenuEntries[adjustOption];

  if(stateFirstRun) {
    adjustCurrent = 0;
    drawAdjustMenu(currentMenu);
    stateFirstRun = 0;
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
      stateFirstRun = 1;
      save = true;
    }
    else if ( buttons == BTN_UP ) {
      adjustOption -= 1;
      stateFirstRun = 1;
      save = true;
    }
    else if ( buttons == BTN_ENTER ) {
      adjustCurrent = 1;
    }
    else if (buttons == BTN_MENU ) {
      adjustCurrent = 0;
      state = MAIN_MENU;
      stateFirstRun = 1;
      save = true;
    }

    if(save && currentMenu->saveFunc)
      currentMenu->saveFunc(*currentMenu);

  } else if( adjustCurrent == 1) {
    if (buttons) {
      drawAdjustBar( buttons, 20, &currentMenu->entries[0], &pos1 );
      if(buttons == BTN_ENTER)      adjustCurrent += 1;
      else if( buttons == BTN_MENU) adjustCurrent = 0;
      redraw = true;
    } else { 
      redraw |= updateAdjustLineCursor( timeNow, pos1, 20 );
    }
  } else {
    if (buttons) {
      drawAdjustBar( buttons, 50, &currentMenu->entries[1], &pos2 );
      if(buttons == BTN_ENTER)      adjustCurrent += 1;
      else if( buttons == BTN_MENU) adjustCurrent = 0;
      redraw = true;
    } else {
      redraw |= updateAdjustLineCursor( timeNow, pos2, 50 );
    }
  }

  // Keep adjustCurrent in range
  if(adjustCurrent > 2)
    adjustCurrent = 0;

  // Keep adjust option in range.
  if(adjustOption < 0)
    adjustOption = numAdjustEntries-1;
  else if (adjustOption >= numAdjustEntries)
    adjustOption = 0;

  return redraw;
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

void drawSensorPixels(){
  if( state != ADJUST_MENU )
    return;

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

static bool ExecuteMenuSelection(int cursorPosition, const struct MenuEntry *menuEntry)
{
  switch(menuEntry->type) {
    case MenuType::ESub:
      *((const MenuEntrySub*)menuEntry)->flag = 1;
      drawMenuCursor(cursorPosition, WHITE);
      drawSubBox( ((const MenuEntrySub*)menuEntry)->subTitle);
      ((const MenuEntrySub*)menuEntry)->subMenuFunc(WHITE);
      return true;

    case MenuType::EStateChange:
      state = ((const MenuEntryStateCh*)menuEntry)->state;
      stateFirstRun = 1;
      break;

    case MenuType::ESubRotator:
      *((const MenuEntrySubRotator*)menuEntry)->flag = ((const MenuEntrySubRotator*)menuEntry)->flagValue;
      drawMenuCursor(cursorPosition, WHITE);
      ((const MenuEntrySubRotator*)menuEntry)->subMenuFunc(WHITE);
      break;
  }

  return false;
}

//***********************************************************

static bool selectMenuOption(int cursorPosition, const MenuEntry** menuEntries){
  cursorBlinkTime = millis();
  const MenuEntry* entry = menuEntries[cursorPosition];
  return ExecuteMenuSelection( cursorPosition, entry );
}

//***********************************************************

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
        redraw |= selectMenuOption(cursorPos, page.entries);
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

//***********************************************************
static void checkForPatchView()
{
  int trills = readTrills();

  switch (deumButtonState){
    case BTN_MENU+BTN_DOWN:
      break;

    case BTN_MENU+BTN_ENTER:
      if (trills){
        state = PATCH_VIEW;
        stateFirstRun = 1;
        setFPS(trills, patch);
      }
      break;

    case BTN_MENU+BTN_UP:
      if (trills){
        state = PATCH_VIEW;
        stateFirstRun = 1;
        clearFPS(trills);
      }
      break;
  }
}

//***********************************************************

void menu() {
  unsigned long timeNow = millis();
  const MenuPage *currentPage = nullptr; 

  bool redrawSubValue = false;
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

  if (state && ((timeNow - menuTime) > menuTimeUp)) { // shut off menu system if not used for a while (changes not stored by exiting a setting manually will not be stored in EEPROM)
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

  if        (state == DISPLAYOFF_IDL) {
    if (stateFirstRun) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      stateFirstRun = 0;
    }
    if (buttonPressedAndNotUsed){
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
            digitalWrite(statusLedPin,LOW);
            delay(150);
            digitalWrite(statusLedPin,HIGH);
            delay(150);
            digitalWrite(statusLedPin, LOW);
            delay(150);
            digitalWrite(statusLedPin,HIGH);
          } else if ((exSensor >= ((extracThrVal+extracMaxVal)/2))) { // switch pb pad activated legacy settings control on/off
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
    if (stateFirstRun) {
      drawMenuScreen();
      stateFirstRun = 0;
    }
    currentPage = &mainMenuPage;
    if (subTranspose){
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (transpose > 0)
              transpose--;
            break;

          case BTN_UP:
            if (transpose < 24)
              transpose++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subTranspose = 0;
            writeSetting(TRANSP_ADDR,transpose);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subOctave){
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (octave > 0)
              octave--;
            break;

          case BTN_UP:
            if (octave < 6)
              octave++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subOctave = 0;
            writeSetting(OCTAVE_ADDR,octave);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subMIDI) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (MIDIchannel > 1)
              MIDIchannel--;
            break;

          case BTN_UP:
            if (MIDIchannel < 16)
              MIDIchannel++;
            break;

          case BTN_ENTER:
            readSwitches();
            if (pinkyKey){
              slowMidi = !slowMidi;
              dipSwBits = dipSwBits ^ (1<<3);
              writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            } else {
              subMIDI = 0;
              writeSetting(MIDI_ADDR,MIDIchannel);
            }
            break;

          case BTN_MENU:
            subMIDI = 0;
            writeSetting(MIDI_ADDR,MIDIchannel);
            break;
        }
        redrawSubValue = true;
      }
    } else {
      bool hadButtons = buttonPressedAndNotUsed;
      redraw |= updateMenuPage( *currentPage, timeNow );
      if (hadButtons)
        checkForPatchView();
    }
  } else if (state == ROTATOR_MENU) {    // ROTATOR MENU HERE <<<<<<<<<<<<<<<
    if (stateFirstRun) {
      drawMenu(rotatorMenuPage);
      stateFirstRun = 0;
    }
    currentPage = &rotatorMenuPage;
    if (subParallel){
      if (((timeNow - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotRotator(cursorNow,parallel);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (parallel > -24)
              parallel--;
            break;

          case BTN_UP:
            if (parallel < 24)
              parallel++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subParallel = 0;
            writeSetting(PARAL_ADDR,(parallel + 24));
            break;
        }
        clearSubValue();
        plotRotator(WHITE,parallel);
        cursorNow = BLACK;
        redraw = true;
        cursorBlinkTime = timeNow;
      }
    } else if (subRotator){
      if (((timeNow - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotRotator(cursorNow,rotations[subRotator-1]);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (rotations[subRotator-1] > -24)
              rotations[subRotator-1]--;
            break;

          case BTN_UP:
            if (rotations[subRotator-1] < 24)
              rotations[subRotator-1]++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subRotator = 0;
            break;
        }
        clearSubValue();
        plotRotator(WHITE,rotations[subRotator-1]);
        cursorNow = BLACK;
        redraw = true;
        cursorBlinkTime = timeNow;
      }
    } else if (subPriority){
      updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed) {
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
          case BTN_UP:
            // up
            priority = !priority;
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            subPriority = 0;
            writeSetting(PRIO_ADDR,priority);
            break;
        }
        redrawSubValue = true;
      }
    } else {
      bool hadButtons = buttonPressedAndNotUsed;
      redraw |= updateMenuPage( *currentPage, timeNow );
      if (hadButtons)
        checkForPatchView();
    }
  // end rotator menu

  } else if (state == ADJUST_MENU) {
    // This is a hack to update touch_Thr is it was changed..
    int old_thr = ctouchThrVal;
    redraw |= updateAdjustMenu( timeNow, buttonPressedAndNotUsed ? deumButtonState : 0 );
    buttonPressedAndNotUsed = 0;

    if( old_thr != ctouchThrVal) {
      touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
    }
  } else if (state == SETUP_BR_MENU) {  // SETUP BREATH MENU HERE <<<<<<<<<<<<<<
    if (stateFirstRun) {
      drawMenu( breathMenuPage );
      stateFirstRun = 0;
    }
    currentPage = &breathMenuPage;
    if (subBreathCC){
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (breathCC > 0){
              breathCC--;
            } else {
              breathCC = 10;
            }
            break;
          case BTN_UP:
            if (breathCC < 10){
              breathCC++;
            } else {
              breathCC = 0;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            subBreathCC = 0;
            if (readSetting(BREATH_CC_ADDR) != breathCC) {
              writeSetting(BREATH_CC_ADDR,breathCC);
              midiReset();
            }
            break;
        }
        redrawSubValue = true;
      }
    } else if (subBreathAT) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN: // fallthrough
            breathAT = !breathAT;
            breathAT = !breathAT;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subBreathAT = 0;
            if (readSetting(BREATH_AT_ADDR) != breathAT){
              writeSetting(BREATH_AT_ADDR, breathAT);
              midiReset();
            }
            break;
        }
        redrawSubValue = true;
      }
    } else if (subVelocity) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (velocity > 0){
              velocity--;
            } else velocity = 127;
            break;

          case BTN_UP:
            if (velocity < 127){
              velocity++;
            } else velocity = 0;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVelocity = 0;
            writeSetting(VELOCITY_ADDR,velocity);
            break;
        }
        redrawSubValue = true;
      }

    } else if (subCurve) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (curve > 0)
              curve--;
            else curve = 12;
            break;

          case BTN_UP:
            if (curve < 12)
              curve++;
            else curve = 0;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subCurve = 0;
            writeSetting(BREATHCURVE_ADDR,curve);
            break;
        }
        redrawSubValue = true;
      }

    } else if (subVelSmpDl) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (velSmpDl > 0){
              velSmpDl-=1;
            } else velSmpDl = 30;
            break;

          case BTN_UP:
            if (velSmpDl < 30){
              velSmpDl+=1;
            } else velSmpDl = 0;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVelSmpDl = 0;
            writeSetting(VEL_SMP_DL_ADDR,velSmpDl);
            break;
        }
        redrawSubValue = true;
      }

     } else if (subVelBias) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState) {
          case BTN_DOWN:
            if (velBias > 0){
              velBias--;
            } else velBias = 9;
            break;

          case BTN_ENTER:
            subVelBias = 0;
            writeSetting(VEL_BIAS_ADDR,velBias);
            break;

          case BTN_UP:
            if (velBias < 9){
              velBias++;
            } else velBias = 0;
            break;

          case BTN_MENU:
            subVelBias = 0;
            writeSetting(VEL_BIAS_ADDR,velBias);
            break;
        }
        clearSubValue();
        plotVelBias(WHITE);
        cursorBlinkTime = timeNow;
        cursorNow = BLACK;
        redraw = true;
      }

    } else {
      redraw |= updateMenuPage( *currentPage, timeNow );
    }


  } else if (state == SETUP_CT_MENU) {  // SETUP CONTROLLERS MENU HERE <<<<<<<<<<<<<
    if (stateFirstRun) {
      drawMenu( controlMenuPage );
      stateFirstRun = 0;
    }
    currentPage = &controlMenuPage;
    if (subPort){
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (portamento > 0){
              portamento--;
            } else portamento = 2;
            break;

          case BTN_UP:
            if (portamento < 2){
              portamento++;
            } else portamento = 0;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subPort = 0;
            writeSetting(PORTAM_ADDR,portamento);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subPB) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (PBdepth > 0)
              PBdepth--;
            break;

          case BTN_UP:
            if (PBdepth < 12)
              PBdepth++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subPB = 0;
            writeSetting(PB_ADDR,PBdepth);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subExtra) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (extraCT > 0){
              extraCT--;
            } else extraCT = 4;
            break;

          case BTN_UP:
            if (extraCT < 4){
              extraCT++;
            } else extraCT = 0;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subExtra = 0;
            writeSetting(EXTRA_ADDR,extraCT);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subDeglitch) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (deglitch > 0)
              deglitch-=1;
            break;

          case BTN_UP:
            if (deglitch < 70){
              deglitch+=1;
            }
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subDeglitch = 0;
            writeSetting(DEGLITCH_ADDR,deglitch);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subPinky) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (pinkySetting > 0)
              pinkySetting-=1;
            break;

          case BTN_UP:
            if (pinkySetting < 24)
              pinkySetting += 1;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subPinky = 0;
            writeSetting(PINKY_KEY_ADDR,pinkySetting);
            break;
        }
        redrawSubValue = true;
      }
    } else {
      redraw |= updateMenuPage( *currentPage, timeNow );
    }

  } else if (state == VIBRATO_MENU) {   // VIBRATO MENU HERE <<<<<<<<<<<<<
    if (stateFirstRun) {
      drawMenu(vibratoMenuPage);
      stateFirstRun = 0;
    }
    currentPage = &vibratoMenuPage;
    if (subVibrato) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibrato > 0)
              vibrato--;
            break;

          case BTN_UP:
            if (vibrato < 9)
              vibrato++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVibrato = 0;
            writeSetting(VIBRATO_ADDR,vibrato);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subVibSens) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibSens > 1)
              vibSens--;
            break;

          case BTN_UP:
            if (vibSens < 12)
              vibSens++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVibSens = 0;
            writeSetting(VIB_SENS_ADDR,vibSens);
            break;
        }
        clearSubValue();
        plotVibSens(WHITE);
        cursorNow = BLACK;
        redraw = true;
        cursorBlinkTime = timeNow;
      }
    } else if (subVibRetn) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibRetn > 0)
              vibRetn--;
            break;

          case BTN_UP:
            if (vibRetn < 4)
              vibRetn++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVibRetn = 0;
            writeSetting(VIB_RETN_ADDR,vibRetn);
            break;
        }
        redrawSubValue = true;
      }
    } else if (subVibSquelch) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibSquelch > 1)
              vibSquelch--;
            break;

          case BTN_UP:
            if (vibSquelch < 30)
              vibSquelch++;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVibSquelch = 0;
            writeSetting(VIB_SQUELCH_ADDR,vibSquelch);
            break;
        }
        clearSubValue();
        plotVibSquelch(WHITE);
        cursorBlinkTime = timeNow;
        cursorNow = BLACK;
        redraw = true;
      }
    } else if (subVibDirection) {
      redraw |= updateSubMenuCursor( *currentPage, timeNow );
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){

          case BTN_DOWN: // fallthrough
          case BTN_UP:
            vibDirection = !vibDirection;
            break;

          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVibDirection = 0;
            writeSetting(VIB_DIRECTION_ADDR,vibDirection);
            break;
        }
        redrawSubValue = true;
      }
    } else {
      redraw |= updateMenuPage( *currentPage, timeNow );
    }
  }

  if(redrawSubValue && currentPage) {
    clearSubValue();
    redraw |= drawSubMenu(*currentPage, WHITE);
    cursorNow = BLACK;
    cursorBlinkTime = timeNow;
  }

  if(redraw) {
    display.display();
  }
}
