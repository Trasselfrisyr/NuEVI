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


#define BTN_DOWN 1
#define BTN_ENTER 2
#define BTN_UP 4
#define BTN_MENU 8

// TODO: Ask Johan the reason for using this..
// static const uint16_t minOffset = 50;

static uint8_t lastDeumButtons = 0;
static uint8_t deumButtonState = 0;
static byte buttonPressedAndNotUsed = 0;

static byte mainMenuCursor = 1;
static byte setupBrMenuCursor = 1;
static byte setupCtMenuCursor = 1;
static byte rotatorMenuCursor = 1;
static byte vibratoMenuCursor = 1;


static byte cursorNow;
static byte forcePix = 0;
static byte forceRedraw = 0;
static byte FPD = 0;


static uint16_t pos1;
static uint16_t pos2;

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


#define OLED_RESET 4
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// NuMenu mainMenu(display);

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
  #if defined(CVSCALEBOARD)
  display.setCursor(15,0);
  display.print("CV");
  #endif
  display.setCursor(85,52);
  display.print("v.");
  display.println(FIRMWARE_VERSION);
  display.display();
}

void drawAdjCursor(byte color){
  display.drawTriangle(16,4,20,4,18,1,color);
  display.drawTriangle(16,6,20,6,18,9,color);
}

static void drawAdjustBase(const char* title, bool all) {
  display.clearDisplay();

  display.drawLine(25,17,120,17,WHITE);
  display.drawLine(25,18,25,19,WHITE);
  display.drawLine(120,18,120,19,WHITE);
  display.drawLine(25,29,120,29,WHITE);
  display.drawLine(25,27,25,28,WHITE);
  display.drawLine(120,27,120,28,WHITE);

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
    display.drawLine(25,47,120,47,WHITE);
    display.drawLine(25,48,25,49,WHITE);
    display.drawLine(120,48,120,49,WHITE);
    display.drawLine(25,59,120,59,WHITE);
    display.drawLine(25,57,25,58,WHITE);
    display.drawLine(120,57,120,58,WHITE);
    display.setCursor(0,50);
    display.println("MAX");
  }
  cursorNow = WHITE;
  drawAdjCursor(WHITE);
}

void drawAdjustScreen(const char* title, int threshold, int maxValue, uint16_t lowLimit, uint16_t highLimit){
  drawAdjustBase(title, maxValue >= 0);

  //display.drawLine(38,20,38,26,WHITE); // indikation thr
  pos1 = map(threshold, lowLimit, highLimit, 27, 119);
  display.drawLine(pos1,20,pos1,26,WHITE);

  //display.drawLine(115,50,115,56,WHITE); // indikation max
  if(maxValue >= 0) {
    pos2 = map(maxValue, lowLimit, highLimit, 27, 119);
    display.drawLine(pos2,50,pos2,56,WHITE);
  }
  display.display();
}

void drawMenuCursor(byte itemNo, byte color){
  byte xmid = 6 + 9 * itemNo;
  display.drawTriangle(57,xmid,61,xmid+2,61,xmid-2,color);
}


/*
 * Draw a regular list of text menu items
 * header - first header line
 * selectedItem - the currently selected item (draw a triangle next to it). -1 for none. 1..nItems (1-based, 0 is header row)
 * nItems - number of menu items
 * ... - a list (nItems long) of text items to show
 */
static void drawMenu(const char* header, byte selected, byte nItems, ...) {
  va_list valist;

  //Initialize display and draw menu header + line
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(header);
  display.drawLine(0,MENU_ROW_HEIGHT,127,MENU_ROW_HEIGHT, WHITE);

  va_start(valist, nItems);
  for(byte row=0; row<nItems; row++) {
    int rowPixel = (row+1)*MENU_ROW_HEIGHT + MENU_HEADER_OFFSET;
    const char* lineText = va_arg(valist, const char*);
    display.setCursor(0,rowPixel);
    display.println(lineText);
  }

  drawMenuCursor(selected, WHITE);

  va_end(valist);

  display.display();
}

void drawMenuScreen(){

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

  drawMenu(menuTitle, mainMenuCursor, 6,
    "TRANSPOSE",
    "OCTAVE",
    "MIDI CH",
    "ADJUST",
    "SETUP BR",
    "SETUP CTL");
}

static void drawRotatorMenuScreen(){
  drawMenu("ROTATOR SETUP", rotatorMenuCursor, 6,
    "PARALLEL",
    "ROTATE 1",
    "ROTATE 2",
    "ROTATE 3",
    "ROTATE 4",
    "PRIORITY");
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

static void clearSub(){
  display.fillRect(63,11,64,52,BLACK);
}

static void drawSubBox()
{
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
}

static void plotTranspose(int color){
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

static void drawSubTranspose(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("TRANSPOSE");
  plotTranspose(WHITE);
  display.display();
}


static void plotRotator(int color,int value){
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

static void drawSubRotator(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("SEMITONES");
  //plotRotator(WHITE,value);
  forceRedraw = 1;
  display.display();
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

static void drawSubPriority(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("MONO PRIO");
  plotPriority(WHITE);
  display.display();
}


static void plotOctave(int color){
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

static void drawSubOctave(){
  drawSubBox();
  display.setCursor(77,15);
  display.println("OCTAVE");
  plotOctave(WHITE);
  display.display();
}

static void plotMIDI(int color){
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

static void drawSubMIDI(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("MIDI CHNL");
  plotMIDI(WHITE);
  display.display();
}


static void plotSubOption(const char* label, int color)
{
  display.setTextColor(color);
  display.setTextSize(2);
  int x_pos = 91-strlen(label)*4;
  display.setCursor(x_pos,33);
  display.println(label);
}

static const char* breathCCMenuLabels[] = { "OFF", "MW", "BR", "VL", "EX", "MW+",
                                            "BR+", "VL+", "EX+", "CF", "20" };

static void plotBreathCC(int color){
  plotSubOption(breathCCMenuLabels[breathCC], color);
}

static void drawSubBreathCC(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("BREATH CC");
  plotBreathCC(WHITE);
  display.display();
}

static void plotBreathAT(int color){
  if (breathAT){
    plotSubOption("ON", color);
  } else {
    plotSubOption("OFF", color);
  }
}

static void drawSubBreathAT(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("BREATH AT");
  plotBreathAT(WHITE);
  display.display();
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

static void drawSubVelocity(){
  drawSubBox();
  display.setCursor(71,15);
  display.println("VELOCITY");
  plotVelocity(WHITE);
  display.display();
}


static const char* curveMenuLabels[] = {"-4", "-3", "-2", "-1", "LIN", "+1", "+2",
                                         "+3", "+4", "S1", "S2", "Z1", "Z2" };

static void plotCurve(int color){
  // Assumes curve is in rage 0..12
  plotSubOption(curveMenuLabels[curve], color);
}

static void drawSubCurve(){
  drawSubBox();
  display.setCursor(80,15);
  display.println("CURVE");
  plotCurve(WHITE);
  display.display();
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

static void drawSubPort(){
  drawSubBox();
  display.setCursor(71,15);
  display.println("PORT/GLD");
  plotPort(WHITE);
  display.display();
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

static void drawSubPB(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("PITCHBEND");
  plotPB(WHITE);
  display.display();
}

static const char* extraMenuLabels[] = { "OFF", "MW", "FP", "CF", "SP" };

static void plotExtra(int color){
  plotSubOption(extraMenuLabels[extraCT], color);
}

static void drawSubExtra(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("EXTRA CTR");
  plotExtra(WHITE);
  display.display();
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

static void drawSubVibrato(){
  drawSubBox();
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibrato(WHITE);
  display.display();
}

static void plotVibSens(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(vibSens);
}

static void drawSubVibSens(){
  drawSubBox();
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibSens(WHITE);
  display.display();
}

static void plotVibRetn(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(vibRetn);
}

static void drawSubVibRetn(){
  drawSubBox();
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibRetn(WHITE);
  display.display();
}


static void plotVibSquelch(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(83,33);
  display.println(vibSquelch);
}

static void drawSubVibSquelch(){
  drawSubBox();
  display.setCursor(81,15);
  display.println("LEVEL");
  plotVibSquelch(WHITE);
  display.display();
}




static void plotVibDirection(int color){
  if (DNWD == vibDirection){
    plotSubOption("NRM", color);
  } else {
    plotSubOption("NRM", color);
  }
}

static void drawSubVibDirection(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("DIRECTION");
  plotVibDirection(WHITE);
  display.display();
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

static void drawSubDeglitch(){
  drawSubBox();
  display.setCursor(71,15);
  display.println("DEGLITCH");
  plotDeglitch(WHITE);
  display.display();
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

static void drawSubPinkyKey(){
  drawSubBox();
  display.setCursor(68,15);
  display.println("PINKY KEY");
  plotPinkyKey(WHITE);
  display.display();
}

static void plotVelSmpDl(int color){
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

static void drawSubVelSmpDl(){
  drawSubBox();
  display.setCursor(69,15);
  display.println("VEL DELAY");
  plotVelSmpDl(WHITE);
  display.display();
}


static void plotVelBias(int color){
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

static void drawSubVelBias(){
  drawSubBox();
  display.setCursor(72,15);
  display.println("VEL BIAS");
  plotVelBias(WHITE);
  display.display();
}

static void drawSetupBrMenuScreen(){
  drawMenu("SETUP BREATH", -1, 6, "BREATH CC", "BREATH AT", "VELOCITY", "CURVE", "VEL DELAY", "VEL BIAS");
}

static void drawSetupCtMenuScreen(){
  drawMenu("SETUP CTRLS", -1, 6, "PORT/GLD", "PITCHBEND", "EXTRA CTR", "VIBRATO", "DEGLITCH", "PINKY KEY");
}

static void drawVibratoMenuScreen(){
  drawMenu("VIBRATO", -1, 5, "DEPTH","SENSE","RETURN", "SQUELCH", "DIRECTION");
}

static int sensorPixelPos1 = -1;
static int sensorPixelPos2 = -1;

bool updateSensorPixel(int pos, int pos2) {
  bool update = pos != sensorPixelPos1 || pos2 != sensorPixelPos2;
  if(update) {
    display.drawFastHLine(28, 38, 118-28, BLACK); // Clear old line
    display.drawPixel(pos, 38, WHITE);
    sensorPixelPos1 = pos;
    sensorPixelPos2 = pos2;
  }
  return update;
}

void drawSensorPixels(){
  int redraw=0;
  if(forcePix)
    sensorPixelPos1 = -1;

  if ((state == BREATH_ADJ_IDL) || (state == BREATH_ADJ_THR) || (state == BREATH_ADJ_MAX)){
    int pos = map(constrain(pressureSensor, breathLoLimit, breathHiLimit), breathLoLimit, breathHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, -1);
  }
  else if ((state == PORTAM_ADJ_IDL) || (state == PORTAM_ADJ_THR) || (state == PORTAM_ADJ_MAX)){
    int pos = map(constrain(biteSensor,portamLoLimit,portamHiLimit), portamLoLimit, portamHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, -1);
  }
  else if ((state == PITCHB_ADJ_IDL) || (state == PITCHB_ADJ_THR) || (state == PITCHB_ADJ_MAX)){
    int pos = map(constrain(pbUp, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    int pos2 = map(constrain(pbDn, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, pos2);
  }
  else if ((state == EXTRAC_ADJ_IDL) || (state == EXTRAC_ADJ_THR) || (state == EXTRAC_ADJ_MAX)){
    int pos = map(constrain(exSensor, extracLoLimit, extracHiLimit), extracLoLimit, extracHiLimit, 28, 118);
    redraw = updateSensorPixel(pos, -1);
  }
  else if ((state == CTOUCH_ADJ_IDL) || (state == CTOUCH_ADJ_THR)){
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

static void setFPS(int trills) {
  fastPatch[trills-1] = patch;
  writeSetting(FP1_ADDR+2*(trills-1),patch);
  FPD = 2;
}

//***********************************************************

static void clearFPS(int trills) {
  fastPatch[trills-1] = 0;
  writeSetting(FP1_ADDR+2*(trills-1),0);
  FPD = 3;
}

static void selectMainMenu(){
  cursorBlinkTime = millis();
  switch (mainMenuCursor){
    case 1:
      subTranspose = 1;
      drawMenuCursor(mainMenuCursor, WHITE);
      display.display();
      drawSubTranspose();
      break;
    case 2:
      subOctave = 1;
      drawMenuCursor(mainMenuCursor, WHITE);
      display.display();
      drawSubOctave();
      break;
    case 3:
      subMIDI = 1;
      drawMenuCursor(mainMenuCursor, WHITE);
      display.display();
      drawSubMIDI();
      break;
    case 4:
      state = BREATH_ADJ_IDL;
      sensorPixelPos1 = -1;
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

static void selectRotatorMenu(){
  drawMenuCursor(rotatorMenuCursor, WHITE);
  cursorBlinkTime = millis();

  switch (rotatorMenuCursor){
    case 1:
      subParallel = 1;
      drawSubRotator();
      break;
    case 2:
      subRotator = 1;
      drawSubRotator();
      break;
    case 3:
      subRotator = 2;
      drawSubRotator();
      break;
    case 4:
      subRotator = 3;
      drawSubRotator();
      break;
    case 5:
      subRotator = 4;
      drawSubRotator();
      break;
    case 6:
      subPriority = 1;
      drawSubPriority();
      break;
  }
}

static void selectSetupBrMenu(){
  drawMenuCursor(setupBrMenuCursor, WHITE);
  cursorBlinkTime = millis();
  switch (setupBrMenuCursor){
    case 1:
      subBreathCC = 1;
      drawSubBreathCC();
      break;
    case 2:
      subBreathAT = 1;
      drawSubBreathAT();
      break;
    case 3:
      subVelocity = 1;
      drawSubVelocity();
      break;
    case 4:
      subCurve = 1;
      drawSubCurve();
      break;
    case 5:
      subVelSmpDl = 1;
      drawSubVelSmpDl();
      break;
    case 6:
      subVelBias = 1;
      drawSubVelBias();
      break;
  }
}

static void selectSetupCtMenu(){
  drawMenuCursor(setupCtMenuCursor, WHITE);
  cursorBlinkTime = millis();

  switch (setupCtMenuCursor){
    case 1:
      subPort = 1;
      drawSubPort();
      break;
    case 2:
      subPB = 1;
      drawSubPB();
      break;
    case 3:
      subExtra = 1;
      drawSubExtra();
      break;
    case 4:
      //subVibrato = 1;
      //drawSubVibrato();
      state = VIBRATO_MENU;
      stateFirstRun = 1;
      break;
    case 5:
      subDeglitch = 1;
      drawSubDeglitch();
      break;
    case 6:
      subPinky = 1;
      drawSubPinkyKey();
  }
}

static void selectVibratoMenu(){
  drawMenuCursor(vibratoMenuCursor, WHITE);
  cursorBlinkTime = millis();

  switch (vibratoMenuCursor){
    case 1:
      subVibrato = 1;
      drawSubVibrato();
      break;
    case 2:
      subVibSens = 1;
      drawSubVibSens();
      break;
    case 3:
      subVibRetn = 1;
      drawSubVibRetn();
      break;
    case 4:
      subVibSquelch = 1;
      drawSubVibSquelch();
      break;
    case 5:
      subVibDirection = 1;
      drawSubVibDirection();
      break;
  }
}

bool drawAdjustBar(uint16_t buttons, int row, uint16_t *valPtr, uint16_t minVal, uint16_t maxVal, uint16_t *pos) {
  bool updated = false;
  uint16_t step = (maxVal-minVal)/92;
  int val = *valPtr;
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
  if(updated)
  {
    *valPtr = constrain(val, minVal, maxVal);
    auto p = *pos;
    display.drawLine(p, row, p, row+6, BLACK);
    *pos = p = map(*valPtr, minVal, maxVal, 27, 119);
    display.drawLine(p, row, p, row+6, WHITE);
    cursorNow = BLACK;
  }
  return updated;
}

void updateAdjustCursor(uint32_t timeNow){
    if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      drawAdjCursor(cursorNow);
      display.display();
      cursorBlinkTime = timeNow;
    }
}

static void updateAdjustLineCursor(uint32_t timeNow, uint16_t hPos, uint16_t vPos ) {
    if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
      if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
      display.drawLine(hPos, vPos,hPos, vPos+6, cursorNow);;
      display.display();
      cursorBlinkTime = timeNow;
    }
}

void menu() {
  unsigned long timeNow = millis();
  // read the state of the switches
  uint8_t deumButtons = 0x0f ^(digitalRead(dPin) | (digitalRead(ePin) << 1) | (digitalRead(uPin) << 2) | (digitalRead(mPin)<<3));

  // mainMenu.update(deumButtons);

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
      Serial.println(deumButtonState);
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



  if        (state == DISPLAYOFF_IDL){
    if (stateFirstRun) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      stateFirstRun = 0;
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      int trills = readTrills();
      switch (deumButtonState){
        case BTN_DOWN:
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
        case BTN_ENTER:
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
        case BTN_UP:
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
        case BTN_MENU:
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
  } else if (state == PATCH_VIEW){
    if (stateFirstRun) {
      drawPatchView();
      patchViewTime = timeNow;
      stateFirstRun = 0;
    }
    if ((timeNow - patchViewTime) > patchViewTimeUp) {
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
        case BTN_DOWN:
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
          patchViewTime = timeNow;
          break;
        case BTN_ENTER:
          // enter
          if (trills && (fastPatch[trills-1] > 0)){
            patch = fastPatch[trills-1];
            activePatch = 0;
            doPatchUpdate = 1;
            FPD = 1;
            drawPatchView();
          }
          patchViewTime = timeNow;
          break;
        case BTN_UP:
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
          patchViewTime = timeNow;
          break;
        case BTN_MENU:
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
            patchViewTime = timeNow;
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotTranspose(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (transpose > 0){
              plotTranspose(BLACK);
              transpose--;
              plotTranspose(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotTranspose(WHITE);
            cursorNow = BLACK;
            display.display();
            subTranspose = 0;
            if (readSetting(TRANSP_ADDR) != transpose) writeSetting(TRANSP_ADDR,transpose);
            break;
          case BTN_UP:
            // up
            if (transpose < 24){
              plotTranspose(BLACK);
              transpose++;
              plotTranspose(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotOctave(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (octave > 0){
              plotOctave(BLACK);
              octave--;
              plotOctave(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotOctave(WHITE);
            cursorNow = BLACK;
            display.display();
            subOctave = 0;
            if (readSetting(OCTAVE_ADDR) != octave) writeSetting(OCTAVE_ADDR,octave);
            break;
          case BTN_UP:
            // up
            if (octave < 6){
              plotOctave(BLACK);
              octave++;
              plotOctave(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotMIDI(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (MIDIchannel > 1){
              plotMIDI(BLACK);
              MIDIchannel--;
              plotMIDI(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
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
          case BTN_UP:
            // up
            if (MIDIchannel < 16){
              plotMIDI(BLACK);
              MIDIchannel++;
              plotMIDI(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(mainMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case BTN_DOWN:
            if (mainMenuCursor < 6){
              drawMenuCursor(mainMenuCursor, BLACK);
              mainMenuCursor++;
              drawMenuCursor(mainMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case BTN_ENTER:
            // enter
            selectMainMenu();
            break;
          case BTN_UP:
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
          case BTN_MENU:
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
      if (((timeNow - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotRotator(cursorNow,parallel);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (parallel > -24){
              plotRotator(BLACK,parallel);
              parallel--;
              plotRotator(WHITE,parallel);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotRotator(WHITE,parallel);
            cursorNow = BLACK;
            display.display();
            subParallel = 0;
            if (readSetting(PARAL_ADDR) != (parallel + 24)) writeSetting(PARAL_ADDR,(parallel + 24));
            break;
          case BTN_UP:
            // up
            if (parallel < 24){
              plotRotator(BLACK,parallel);
              parallel++;
              plotRotator(WHITE,parallel);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if (((timeNow - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotRotator(cursorNow,rotations[subRotator-1]);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (rotations[subRotator-1] > -24){
              plotRotator(BLACK,rotations[subRotator-1]);
              rotations[subRotator-1]--;
              plotRotator(WHITE,rotations[subRotator-1]);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotRotator(WHITE,rotations[subRotator-1]);
            cursorNow = BLACK;
            display.display();
            if (readSetting(ROTN1_ADDR+2*(subRotator-1)) != rotations[subRotator-1]) writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subRotator = 0;
            break;
          case BTN_UP:
            // up
            if (rotations[subRotator-1] < 24){
              plotRotator(BLACK,rotations[subRotator-1]);
              rotations[subRotator-1]++;
              plotRotator(WHITE,rotations[subRotator-1]);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPriority(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotPriority(BLACK);
            priority = !priority;
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            subPriority = 0;
            if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
          case BTN_UP:
            // up
            plotPriority(BLACK);
            priority = !priority;
            plotPriority(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(rotatorMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case BTN_DOWN:
            if (rotatorMenuCursor < 6){
              drawMenuCursor(rotatorMenuCursor, BLACK);
              rotatorMenuCursor++;
              drawMenuCursor(rotatorMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case BTN_ENTER:
            // enter
            selectRotatorMenu();
            break;
          case BTN_UP:
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
          case BTN_MENU:
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
      drawAdjustScreen("BREATH", breathThrVal, breathMaxVal, breathLoLimit, breathHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }
    updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = PORTAM_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(BREATH_THR_ADDR,breathThrVal);
          writeSetting(BREATH_MAX_ADDR,breathMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = BREATH_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = CTOUCH_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(BREATH_THR_ADDR,breathThrVal);
          writeSetting(BREATH_MAX_ADDR,breathMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          writeSetting(BREATH_THR_ADDR,breathThrVal);
          writeSetting(BREATH_MAX_ADDR,breathMaxVal);
          break;
      }
    }
  } else if (state == BREATH_ADJ_THR){
    updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      drawAdjustBar(deumButtonState, 20, &breathThrVal, breathLoLimit, breathHiLimit, &pos1);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
          state = BREATH_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case BTN_MENU:
          state = BREATH_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == BREATH_ADJ_MAX){
    updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      drawAdjustBar(deumButtonState, 50, &breathMaxVal, breathLoLimit, breathHiLimit, &pos2);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
          // enter
          state = BREATH_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          display.display();
          break;
        case BTN_MENU:
          // menu
          state = BREATH_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PORTAM_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("PORTAMENTO", portamThrVal, portamMaxVal, portamLoLimit, portamHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }
    updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = PITCHB_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PORTAM_THR_ADDR) != portamThrVal) writeSetting(PORTAM_THR_ADDR,portamThrVal);
          if (readSetting(PORTAM_MAX_ADDR) != portamMaxVal) writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = PORTAM_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = BREATH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PORTAM_THR_ADDR) != portamThrVal) writeSetting(PORTAM_THR_ADDR,portamThrVal);
          if (readSetting(PORTAM_MAX_ADDR) != portamMaxVal) writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(PORTAM_THR_ADDR) != portamThrVal) writeSetting(PORTAM_THR_ADDR,portamThrVal);
          if (readSetting(PORTAM_MAX_ADDR) != portamMaxVal) writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
      }
    }
  } else if (state == PORTAM_ADJ_THR){
    updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      drawAdjustBar(deumButtonState, 20, &portamThrVal, portamLoLimit, portamHiLimit, &pos1);
      cursorBlinkTime = timeNow;

      switch (deumButtonState){
        case BTN_ENTER:
          // enter
          state = PORTAM_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case BTN_MENU:
          // menu
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PORTAM_ADJ_MAX){
    updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      // TODO: Ask Johan what the minOffset is for...
      // if ((portamMaxVal - portamStep) > (portamThrVal + minOffset)){

      drawAdjustBar(deumButtonState, 50, &portamMaxVal, portamLoLimit, portamHiLimit, &pos2);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PITCHB_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("PITCH BEND", pitchbThrVal, pitchbMaxVal, pitchbLoLimit, pitchbHiLimit);
      // drawPitchbScreen();
      forcePix = 1;
      stateFirstRun = 0;
    }
    updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = EXTRAC_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PITCHB_THR_ADDR) != pitchbThrVal) writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          if (readSetting(PITCHB_MAX_ADDR) != pitchbMaxVal) writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = PITCHB_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = PORTAM_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(PITCHB_THR_ADDR) != pitchbThrVal) writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          if (readSetting(PITCHB_MAX_ADDR) != pitchbMaxVal) writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(PITCHB_THR_ADDR) != pitchbThrVal) writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          if (readSetting(PITCHB_MAX_ADDR) != pitchbMaxVal) writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
      }
    }
  } else if (state == PITCHB_ADJ_THR){
    updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      cursorBlinkTime = timeNow;
      drawAdjustBar(deumButtonState, 20, &pitchbThrVal, pitchbLoLimit, pitchbHiLimit, &pos1);
      switch (deumButtonState){
        case BTN_ENTER:
          // enter
          state = PITCHB_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case BTN_MENU:
          // menu
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == PITCHB_ADJ_MAX){
    updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      drawAdjustBar(deumButtonState, 50, &portamMaxVal, portamLoLimit, portamHiLimit, &pos2);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          display.display();
          break;
      }
    }

  } else if (state == EXTRAC_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("EXTRA CONTROLLER", extracThrVal, extracMaxVal, extracLoLimit, extracHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }

    updateAdjustCursor(timeNow);

    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = CTOUCH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(EXTRAC_THR_ADDR) != extracThrVal) writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          if (readSetting(EXTRAC_MAX_ADDR) != extracMaxVal) writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = EXTRAC_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = PITCHB_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(EXTRAC_THR_ADDR) != extracThrVal) writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          if (readSetting(EXTRAC_MAX_ADDR) != extracMaxVal) writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(EXTRAC_THR_ADDR) != extracThrVal) writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          if (readSetting(EXTRAC_MAX_ADDR) != extracMaxVal) writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
      }
    }
  } else if (state == EXTRAC_ADJ_THR){
    updateAdjustLineCursor(timeNow, pos1, 20);

    if (buttonPressedAndNotUsed){
      drawAdjustBar(deumButtonState, 20, &extracThrVal, extracLoLimit, extracHiLimit, &pos1);

      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
          state = EXTRAC_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
        case BTN_MENU:
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          display.display();
          break;
      }
    }
  } else if (state == EXTRAC_ADJ_MAX){
    updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      drawAdjustBar(deumButtonState, 50, &extracMaxVal, extracLoLimit, extracHiLimit, &pos2);

      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          display.display();
          break;
      }
    }

  } else if (state == CTOUCH_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("TOUCH SENSE", ctouchThrVal, -1, ctouchLoLimit, ctouchHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }
    updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = BREATH_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(CTOUCH_THR_ADDR) != ctouchThrVal) writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
        case BTN_ENTER:
          // enter
          state = CTOUCH_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = EXTRAC_ADJ_IDL;
          stateFirstRun = 1;
          if (readSetting(CTOUCH_THR_ADDR) != ctouchThrVal) writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          if (readSetting(CTOUCH_THR_ADDR) != ctouchThrVal) writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
      }
    }
  } else if (state == CTOUCH_ADJ_THR){
    updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;

      bool updated = drawAdjustBar(deumButtonState, 20, &ctouchThrVal, ctouchLoLimit, ctouchHiLimit, &pos1);
      if(updated)
        touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);

      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotBreathCC(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (breathCC > 0){
              plotBreathCC(BLACK);
              breathCC--;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            } else {
              plotBreathCC(BLACK);
              breathCC = 10;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
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
          case BTN_UP:
            // up
            if (breathCC < 10){
              plotBreathCC(BLACK);
              breathCC++;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            } else {
              plotBreathCC(BLACK);
              breathCC = 0;
              plotBreathCC(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotBreathAT(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotBreathAT(BLACK);
            breathAT=!breathAT;
            plotBreathAT(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
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
          case BTN_UP:
            // up
            plotBreathAT(BLACK);
            breathAT=!breathAT;
            plotBreathAT(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelocity(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotVelocity(BLACK);
            if (velocity > 0){
              velocity--;
            } else velocity = 127;
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelocity = 0;
            if (readSetting(VELOCITY_ADDR) != velocity) writeSetting(VELOCITY_ADDR,velocity);
            break;
          case BTN_UP:
            // up
            plotVelocity(BLACK);
            if (velocity < 127){
              velocity++;
            } else velocity = 0;
            plotVelocity(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotCurve(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotCurve(BLACK);
            if (curve > 0){
              curve--;
            } else curve = 12;
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            subCurve = 0;
            if (readSetting(BREATHCURVE_ADDR) != curve) writeSetting(BREATHCURVE_ADDR,curve);
            break;
          case BTN_UP:
            // up
            plotCurve(BLACK);
            if (curve < 12){
              curve++;
            } else curve = 0;
            plotCurve(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelSmpDl(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotVelSmpDl(BLACK);
            if (velSmpDl > 0){
              velSmpDl-=1;
            } else velSmpDl = 30;
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelSmpDl = 0;
            if (readSetting(VEL_SMP_DL_ADDR) != velSmpDl) writeSetting(VEL_SMP_DL_ADDR,velSmpDl);
            break;
          case BTN_UP:
            // up
            plotVelSmpDl(BLACK);
            if (velSmpDl < 30){
              velSmpDl+=1;
            } else velSmpDl = 0;
            plotVelSmpDl(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelBias(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotVelBias(BLACK);
            if (velBias > 0){
              velBias--;
            } else velBias = 9;
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            subVelBias = 0;
            if (readSetting(VEL_BIAS_ADDR) != velBias) writeSetting(VEL_BIAS_ADDR,velBias);
            break;
          case BTN_UP:
            // up
            plotVelBias(BLACK);
            if (velBias < 9){
              velBias++;
            } else velBias = 0;
            plotVelBias(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(setupBrMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (setupBrMenuCursor < 6){
              drawMenuCursor(setupBrMenuCursor, BLACK);
              setupBrMenuCursor++;
              drawMenuCursor(setupBrMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case BTN_ENTER:
            // enter
            selectSetupBrMenu();
            break;
          case BTN_UP:
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
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPort(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotPort(BLACK);
            if (portamento > 0){
              portamento--;
            } else portamento = 2;
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            subPort = 0;
            if (readSetting(PORTAM_ADDR) != portamento) writeSetting(PORTAM_ADDR,portamento);
            break;
          case BTN_UP:
            // up
            plotPort(BLACK);
            if (portamento < 2){
              portamento++;
            } else portamento = 0;
            plotPort(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPB(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (PBdepth > 0){
              plotPB(BLACK);
              PBdepth--;
              plotPB(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotPB(WHITE);
            cursorNow = BLACK;
            display.display();
            subPB = 0;
            if (readSetting(PB_ADDR) != PBdepth) writeSetting(PB_ADDR,PBdepth);
            break;
          case BTN_UP:
            // up
            if (PBdepth < 12){
              plotPB(BLACK);
              PBdepth++;
              plotPB(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotExtra(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotExtra(BLACK);
            if (extraCT > 0){
              extraCT--;
            } else extraCT = 4;
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            subExtra = 0;
            if (readSetting(EXTRA_ADDR) != extraCT) writeSetting(EXTRA_ADDR,extraCT);
            break;
          case BTN_UP:
            // up
            plotExtra(BLACK);
            if (extraCT < 4){
              extraCT++;
            } else extraCT = 0;
            plotExtra(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotDeglitch(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (deglitch > 0){
              plotDeglitch(BLACK);
              deglitch-=1;
              plotDeglitch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotDeglitch(WHITE);
            cursorNow = BLACK;
            display.display();
            subDeglitch = 0;
            if (readSetting(DEGLITCH_ADDR) != deglitch) writeSetting(DEGLITCH_ADDR,deglitch);
            break;
          case BTN_UP:
            // up
            if (deglitch < 70){
              plotDeglitch(BLACK);
              deglitch+=1;
              plotDeglitch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPinkyKey(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (pinkySetting > 0){
              plotPinkyKey(BLACK);
              pinkySetting-=1;
              plotPinkyKey(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotPinkyKey(WHITE);
            cursorNow = BLACK;
            display.display();
            subPinky = 0;
            if (readSetting(PINKY_KEY_ADDR) != pinkySetting) writeSetting(PINKY_KEY_ADDR,pinkySetting);
            break;
          case BTN_UP:
            // up
            if (pinkySetting < 24){
              plotPinkyKey(BLACK);
              pinkySetting+=1;
              plotPinkyKey(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(setupCtMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (setupCtMenuCursor < 6){
              drawMenuCursor(setupCtMenuCursor, BLACK);
              setupCtMenuCursor++;
              drawMenuCursor(setupCtMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case BTN_ENTER:
            // enter
            selectSetupCtMenu();
            break;
          case BTN_UP:
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
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibrato(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibrato > 0){
              plotVibrato(BLACK);
              vibrato--;
              plotVibrato(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotVibrato(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibrato = 0;
            if (readSetting(VIBRATO_ADDR) != vibrato) writeSetting(VIBRATO_ADDR,vibrato);
            break;
          case BTN_UP:
            // up
            if (vibrato < 9){
              plotVibrato(BLACK);
              vibrato++;
              plotVibrato(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibSens(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibSens > 1){
              plotVibSens(BLACK);
              vibSens--;
              plotVibSens(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotVibSens(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibSens = 0;
            if (readSetting(VIB_SENS_ADDR) != vibSens) writeSetting(VIB_SENS_ADDR,vibSens);
            break;
          case BTN_UP:
            // up
            if (vibSens < 12){
              plotVibSens(BLACK);
              vibSens++;
              plotVibSens(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibRetn(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotVibRetn(BLACK);
            if (vibRetn > 0){
              vibRetn--;
            }
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibRetn = 0;
            if (readSetting(VIB_RETN_ADDR) != vibRetn) writeSetting(VIB_RETN_ADDR,vibRetn);
            break;
          case BTN_UP:
            // up
            plotVibRetn(BLACK);
            if (vibRetn < 4){
              vibRetn++;
            }
            plotVibRetn(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibSquelch(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibSquelch > 1){
              plotVibSquelch(BLACK);
              vibSquelch--;
              plotVibSquelch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER:
            // enter
            plotVibSquelch(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibSquelch = 0;
            if (readSetting(VIB_SQUELCH_ADDR) != vibSquelch) writeSetting(VIB_SQUELCH_ADDR,vibSquelch);
            break;
          case BTN_UP:
            // up
            if (vibSquelch < 30){
              plotVibSquelch(BLACK);
              vibSquelch++;
              plotVibSquelch(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibDirection(cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotVibDirection(BLACK);
            vibDirection = !vibDirection;
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            subVibDirection = 0;
            if (readSetting(VIB_DIRECTION_ADDR) != vibDirection) writeSetting(VIB_DIRECTION_ADDR,vibDirection);
            break;
          case BTN_UP:
            // up
            plotVibDirection(BLACK);
            vibDirection = !vibDirection;
            plotVibDirection(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
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
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(vibratoMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibratoMenuCursor < 5){
              drawMenuCursor(vibratoMenuCursor, BLACK);
              vibratoMenuCursor++;
              drawMenuCursor(vibratoMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case BTN_ENTER:
            // enter
            selectVibratoMenu();
            break;
          case BTN_UP:
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
          case BTN_MENU:
            // menu
            state = SETUP_CT_MENU;
            stateFirstRun = 1;
            break;
        }
      }
    }
  }
}
