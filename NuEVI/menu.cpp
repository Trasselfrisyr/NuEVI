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

#define BTN_DOWN 1
#define BTN_ENTER 2
#define BTN_UP 4
#define BTN_MENU 8

// TODO: Ask Johan the reason for using this..
// static const uint16_t minOffset = 50;

static uint8_t lastDeumButtons = 0;
static uint8_t deumButtonState = 0;
static byte buttonPressedAndNotUsed = 0;

static byte mainMenuCursor = 0;
static byte setupBrMenuCursor = 0;
static byte setupCtMenuCursor = 0;
static byte rotatorMenuCursor = 0;
static byte vibratoMenuCursor = 0;


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
}

void drawMenuCursor(byte itemNo, byte color){
  byte ymid = 15 + 9 * itemNo;
  display.drawTriangle(57, ymid,61, ymid+2,61, ymid-2, color);
}


/*
 * Draw a regular list of text menu items
 * header - first header line
 * selectedItem - the currently selected item (draw a triangle next to it). -1 for none. 1..nItems (1-based, 0 is header row)
 * nItems - number of menu items
 * ... - a list (nItems long) of text items to show
 */
static void drawMenu(const char* header, byte nItems, ...) {
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

  drawMenu(menuTitle, 6,
    "TRANSPOSE",
    "OCTAVE",
    "MIDI CH",
    "ADJUST",
    "SETUP BR",
    "SETUP CTL");
}

static void drawRotatorMenuScreen(){
  drawMenu("ROTATOR SETUP", 6,
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

static void drawSubTranspose(){
  drawSubBox("TRANSPOSE");
  plotTranspose(WHITE);
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
    display.setCursor(93,33);
    display.println(octave-3);
  } else {
    display.println("-");
    display.setCursor(93,33);
    display.println(abs(octave-3));
  }
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
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(vibSens);
}

static void plotVibRetn(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(vibRetn);
}

static void plotVibSquelch(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(83,33);
  display.println(vibSquelch);
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
    display.setCursor(90,33);
    display.println(velBias);
  } else {
    display.setCursor(79,33);
    display.println("OFF");
  }
}

static void drawSubRotator(){
  drawSubBox("SEMITONES");
  //plotRotator(WHITE,value);
  forceRedraw = 1;
}


static void drawSubVelSmpDl(){
  drawSubBox("VEL DELAY");
  plotVelSmpDl(WHITE);
}

static void drawSubPriority(){
  drawSubBox("MONO PRIO");
  plotPriority(WHITE);
}

static void drawSubOctave(){
  drawSubBox("OCTAVE");
  plotOctave(WHITE);
}

static void drawSubMIDI(){
  drawSubBox("MIDI CHNL");
  plotMIDI(WHITE);
}

static void drawSubBreathCC(){
  drawSubBox("BREATH CC");
  plotBreathCC(WHITE);
}

static void drawSubBreathAT(){
  drawSubBox("BREATH AT");
  plotBreathAT(WHITE);
}

static void drawSubVelocity(){
  drawSubBox("VELOCITY");
  plotVelocity(WHITE);
}

static void drawSubCurve(){
  drawSubBox("CURVE");
  plotCurve(WHITE);
}

static void drawSubPort(){
  drawSubBox("PORT/GLD");
  plotPort(WHITE);
}

static void drawSubPB(){
  drawSubBox("PITCHBEND");
  plotPB(WHITE);
}

static void drawSubVelBias(){
  drawSubBox("VEL BIAS");
  plotVelBias(WHITE);
}

static void drawSubVibSquelch(){
  drawSubBox("LEVEL");
  plotVibSquelch(WHITE);
}

static void drawSubVibDirection(){
  drawSubBox("DIRECTION");
  plotVibDirection(WHITE);
}

static void drawSubExtra(){
  drawSubBox("EXTRA CTR");
  plotExtra(WHITE);
}

static void drawSubVibrato(){
  drawSubBox("LEVEL");
  plotVibrato(WHITE);
}

static void drawSubVibSens(){
  drawSubBox("LEVEL");
  plotVibSens(WHITE);
}

static void drawSubVibRetn(){
  drawSubBox("LEVEL");
  plotVibRetn(WHITE);
}

static void drawSubDeglitch(){
  drawSubBox("DEGLITCH");
  plotDeglitch(WHITE);
}

static void drawSubPinkyKey(){
  drawSubBox("PINKY KEY");
  plotPinkyKey(WHITE);
}



static void drawSetupBrMenuScreen(){
  drawMenu("SETUP BREATH", 6, "BREATH CC", "BREATH AT", "VELOCITY", "CURVE", "VEL DELAY", "VEL BIAS");
}

static void drawSetupCtMenuScreen(){
  drawMenu("SETUP CTRLS", 6, "PORT/GLD", "PITCHBEND", "EXTRA CTR", "VIBRATO", "DEGLITCH", "PINKY KEY");
}

static void drawVibratoMenuScreen(){
  drawMenu("VIBRATO", 5, "DEPTH","SENSE","RETURN", "SQUELCH", "DIRECTION");
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
  int redraw = 0;
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

enum MenuType {
  ESub,
  ESubRotator,
  EStateChange,
};

struct MenuEntry {
  enum MenuType type;
};

struct MenuEntrySub {
  enum MenuType type;
  byte* flag;
  void (*subMenuFunc)(void);
};

struct MenuEntrySubRotator {
  enum MenuType type;
  byte flagValue;
  byte* flag;
  void (*subMenuFunc)(void);
};

struct MenuEntryStateCh {
  enum MenuType type;
  byte state;
};

//***********************************************************

static bool ExecuteMenuSelection(int cursorPosition, const struct MenuEntry *menuEntry)
{
  switch(menuEntry->type) {
    case MenuType::ESub:
      *((const MenuEntrySub*)menuEntry)->flag = 1;
      drawMenuCursor(cursorPosition, WHITE);
      ((const MenuEntrySub*)menuEntry)->subMenuFunc();
      return true;

    case MenuType::EStateChange:
      state = ((const MenuEntryStateCh*)menuEntry)->state;
      stateFirstRun = 1;
      break;

    case MenuType::ESubRotator:
      *((const MenuEntrySubRotator*)menuEntry)->flag = ((const MenuEntrySubRotator*)menuEntry)->flagValue;
      drawMenuCursor(cursorPosition, WHITE);
      ((const MenuEntrySub*)menuEntry)->subMenuFunc();
      break;
  }

  return false;
}

//***********************************************************

const MenuEntrySub transposeMenu      = { MenuType::ESub, &subTranspose, drawSubTranspose };
const MenuEntrySub octaveMenu         = { MenuType::ESub, &subOctave, drawSubOctave };
const MenuEntrySub midiMenu           = { MenuType::ESub, &subMIDI, drawSubMIDI };
const MenuEntryStateCh adjustMenu     = { MenuType::EStateChange, BREATH_ADJ_IDL };
const MenuEntryStateCh breathMenu     = { MenuType::EStateChange, SETUP_BR_MENU };
const MenuEntryStateCh controlMenu    = { MenuType::EStateChange, SETUP_CT_MENU };

const MenuEntry* MainMenuEntries[] = { 
  (MenuEntry*)&transposeMenu,
  (MenuEntry*)&octaveMenu, 
  (MenuEntry*)&midiMenu, 
  (MenuEntry*)&adjustMenu, 
  (MenuEntry*)&breathMenu, 
  (MenuEntry*)&controlMenu
};

static bool selectMainMenu(int cursorPosition){
  cursorBlinkTime = millis();
  const struct MenuEntry* entry = MainMenuEntries[cursorPosition];
  return ExecuteMenuSelection( cursorPosition, entry );
}

//***********************************************************
// Rotator menu
const MenuEntrySub rotatorParaMenu      = { MenuType::ESub, &subParallel, drawSubRotator };
const MenuEntrySubRotator rotator1Menu  = { MenuType::ESubRotator, 1, &subRotator, drawSubRotator };
const MenuEntrySubRotator rotator2Menu  = { MenuType::ESubRotator, 2, &subRotator, drawSubRotator };
const MenuEntrySubRotator rotator3Menu  = { MenuType::ESubRotator, 3, &subRotator, drawSubRotator };
const MenuEntrySubRotator rotator4Menu  = { MenuType::ESubRotator, 4, &subRotator, drawSubRotator };
const MenuEntrySub rotatorPrioMenu      = { MenuType::ESub, &subPriority, drawSubPriority };

const MenuEntry* rotatorMenuEntries[] = { 
  (MenuEntry*)&rotatorParaMenu,
  (MenuEntry*)&rotator1Menu,
  (MenuEntry*)&rotator2Menu,
  (MenuEntry*)&rotator3Menu,
  (MenuEntry*)&rotator4Menu,
  (MenuEntry*)&rotatorPrioMenu
};
//***********************************************************

static bool selectRotatorMenu(int cursorPosition){
  cursorBlinkTime = millis();
  const MenuEntry* entry = rotatorMenuEntries[cursorPosition];
  return ExecuteMenuSelection( cursorPosition, entry );
}

//***********************************************************
// Breath menu
const MenuEntrySub breathCCMenu      = { MenuType::ESub, &subBreathCC, drawSubBreathCC };
const MenuEntrySub breathATMenu      = { MenuType::ESub, &subBreathAT, drawSubBreathAT };
const MenuEntrySub velocityMenu      = { MenuType::ESub, &subVelocity, drawSubVelocity };
const MenuEntrySub curveMenu         = { MenuType::ESub, &subCurve, drawSubCurve };
const MenuEntrySub velSmpDlMenu      = { MenuType::ESub, &subVelSmpDl, drawSubVelSmpDl };
const MenuEntrySub velBiasMenu       = { MenuType::ESub, &subVelBias, drawSubVelBias };

const MenuEntry* breathMenuEntries[] = { 
  (MenuEntry*)&breathCCMenu,
  (MenuEntry*)&breathATMenu,
  (MenuEntry*)&velocityMenu,
  (MenuEntry*)&curveMenu,
  (MenuEntry*)&velSmpDlMenu,
  (MenuEntry*)&velBiasMenu
};
//***********************************************************

static bool selectSetupBrMenu(int cursorPosition) {
  cursorBlinkTime = millis();
  const MenuEntry* entry = breathMenuEntries[cursorPosition];
  return ExecuteMenuSelection( cursorPosition, entry );
}

//***********************************************************
// Breath menu
const MenuEntrySub portMenu           = { MenuType::ESub, &subPort, drawSubPort };
const MenuEntrySub pitchBendMenu      = { MenuType::ESub, &subPB, drawSubPB };
const MenuEntrySub extraMenu          = { MenuType::ESub, &subExtra, drawSubExtra };
const MenuEntryStateCh vibratoSubMenu = { MenuType::EStateChange, VIBRATO_MENU };
const MenuEntrySub deglitchMenu       = { MenuType::ESub, &subDeglitch, drawSubDeglitch };
const MenuEntrySub pinkyMenu          = { MenuType::ESub, &subPinky, drawSubPinkyKey };

const MenuEntry* controlMenuEntries[] = { 
  (MenuEntry*)&portMenu,
  (MenuEntry*)&pitchBendMenu,
  (MenuEntry*)&extraMenu,
  (MenuEntry*)&vibratoSubMenu,
  (MenuEntry*)&deglitchMenu,
  (MenuEntry*)&pinkyMenu
};

//***********************************************************

static bool selectSetupCtMenu(int cursorPosition){
  cursorBlinkTime = millis();
  const MenuEntry* entry = controlMenuEntries[cursorPosition];
  return ExecuteMenuSelection( cursorPosition, entry );
}


//***********************************************************
// Vibrato menu
const MenuEntrySub vibDepthMenu          = { MenuType::ESub, &subVibrato, drawSubVibrato };
const MenuEntrySub vibSenseMenu          = { MenuType::ESub, &subVibSens, drawSubVibSens };
const MenuEntrySub vibRetnMenu           = { MenuType::ESub, &subVibRetn, drawSubVibRetn };
const MenuEntrySub vibSquelchMenu        = { MenuType::ESub, &subVibSquelch, drawSubVibSquelch };
const MenuEntrySub vibDirMenu            = { MenuType::ESub, &subVibDirection, drawSubVibDirection };

const MenuEntry* vibratorMenuEntries[] = {
    (MenuEntry*)&vibDepthMenu,
    (MenuEntry*)&vibSenseMenu,
    (MenuEntry*)&vibRetnMenu,
    (MenuEntry*)&vibSquelchMenu,
    (MenuEntry*)&vibDirMenu
};

static bool selectVibratoMenu(int cursorPosition){
  cursorBlinkTime = millis();
  const MenuEntry* entry = vibratorMenuEntries[cursorPosition];
  return ExecuteMenuSelection( cursorPosition, entry );
}


//***********************************************************



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

static bool updateAdjustCursor(uint32_t timeNow){
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

void menu() {
  unsigned long timeNow = millis();
  bool redraw = false;
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

  if (stateFirstRun) {
    redraw = true;
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
      writeSetting(PATCH_ADDR,patch);
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
          patchViewTime = timeNow;
          break;
        case BTN_MENU:
          // menu
          if (FPD < 2){
            state = DISPLAYOFF_IDL;
            stateFirstRun = 1;
            doPatchUpdate = 1;
          }
          writeSetting(PATCH_ADDR,patch);
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
            redraw = true;
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
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (transpose < 24){
              plotTranspose(BLACK);
              transpose++;
              plotTranspose(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotTranspose(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subTranspose = 0;
            writeSetting(TRANSP_ADDR,transpose);
            break;
        }
      }
    } else if (subOctave){
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotOctave(cursorNow);
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (octave < 6){
              plotOctave(BLACK);
              octave++;
              plotOctave(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotOctave(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subOctave = 0;
            writeSetting(OCTAVE_ADDR,octave);
            break;
        }
      }
    } else if (subMIDI) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotMIDI(cursorNow);
        redraw = true;
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
              redraw = true;
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
              redraw = true;
              dipSwBits = dipSwBits ^ (1<<3);
              writeSetting(DIPSW_BITS_ADDR,dipSwBits);
            } else {
              plotMIDI(WHITE);
              cursorNow = BLACK;
              redraw = true;
              subMIDI = 0;
              writeSetting(MIDI_ADDR,MIDIchannel);
            }
            break;
          case BTN_UP:
            // up
            if (MIDIchannel < 16){
              plotMIDI(BLACK);
              MIDIchannel++;
              plotMIDI(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_MENU:
            // menu
            plotMIDI(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subMIDI = 0;
            writeSetting(MIDI_ADDR,MIDIchannel);
            break;
        }
      }
    } else {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(mainMenuCursor, cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case BTN_DOWN:
            if (mainMenuCursor < 5){
              drawMenuCursor(mainMenuCursor, BLACK);
              mainMenuCursor++;
              drawMenuCursor(mainMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
            }
            break;
          case BTN_ENTER:
            // enter
            redraw |= selectMainMenu(mainMenuCursor);
            break;
          case BTN_UP:
            // up
            if (mainMenuCursor > 0){
              drawMenuCursor(mainMenuCursor, BLACK);
              mainMenuCursor--;
              drawMenuCursor(mainMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
            }
            break;
          case BTN_MENU:
            // menu
            state = DISPLAYOFF_IDL;
            stateFirstRun = 1;
            break;
          case BTN_MENU+BTN_DOWN:
            //menu+down

            break;
          case BTN_MENU+BTN_ENTER:
            //menu+enter
            if (trills){
              state = PATCH_VIEW;
              stateFirstRun = 1;
              setFPS(trills, patch);
            }
            break;
          case BTN_MENU+BTN_UP:
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
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (parallel < 24){
              plotRotator(BLACK,parallel);
              parallel++;
              plotRotator(WHITE,parallel);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotRotator(WHITE,parallel);
            cursorNow = BLACK;
            redraw = true;
            subParallel = 0;
            writeSetting(PARAL_ADDR,(parallel + 24));
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
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (rotations[subRotator-1] < 24){
              plotRotator(BLACK,rotations[subRotator-1]);
              rotations[subRotator-1]++;
              plotRotator(WHITE,rotations[subRotator-1]);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotRotator(WHITE,rotations[subRotator-1]);
            cursorNow = BLACK;
            redraw = true;
            writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subRotator = 0;
            break;
        }
      }
    } else if (subPriority){
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPriority(cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
          case BTN_UP:
            // up
            plotPriority(BLACK);
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
        plotPriority(WHITE);
        cursorNow = BLACK;
        redraw = true;
      }
    } else {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(rotatorMenuCursor, cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case BTN_DOWN:
            if (rotatorMenuCursor < 5){
              drawMenuCursor(rotatorMenuCursor, BLACK);
              rotatorMenuCursor++;
              drawMenuCursor(rotatorMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
            }
            break;
          case BTN_ENTER:
            // enter
            redraw |= selectRotatorMenu(rotatorMenuCursor);
            break;
          case BTN_UP:
            // up
            if (rotatorMenuCursor > 0){
              drawMenuCursor(rotatorMenuCursor, BLACK);
              rotatorMenuCursor--;
              drawMenuCursor(rotatorMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
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
              setFPS(trills, patch);
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
      sensorPixelPos1 = -1; // Force draw of sensor pixels
      stateFirstRun = 0;
    }
    redraw |=updateAdjustCursor(timeNow);
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
    redraw |= updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      redraw |= drawAdjustBar(deumButtonState, 20, &breathThrVal, breathLoLimit, breathHiLimit, &pos1);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
          state = BREATH_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
        case BTN_MENU:
          state = BREATH_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
      }
    }
  } else if (state == BREATH_ADJ_MAX){
    redraw |= updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      redraw |= drawAdjustBar(deumButtonState, 50, &breathMaxVal, breathLoLimit, breathHiLimit, &pos2);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = BREATH_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          redraw = true;
          break;
      }
    }
  } else if (state == PORTAM_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("PORTAMENTO", portamThrVal, portamMaxVal, portamLoLimit, portamHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }
    redraw |= updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = PITCHB_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(PORTAM_THR_ADDR,portamThrVal);
          writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = PORTAM_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = BREATH_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(PORTAM_THR_ADDR,portamThrVal);
          writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          writeSetting(PORTAM_THR_ADDR,portamThrVal);
          writeSetting(PORTAM_MAX_ADDR,portamMaxVal);
          break;
      }
    }
  } else if (state == PORTAM_ADJ_THR){
    redraw |= updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      redraw |= drawAdjustBar(deumButtonState, 20, &portamThrVal, portamLoLimit, portamHiLimit, &pos1);
      cursorBlinkTime = timeNow;

      switch (deumButtonState){
        case BTN_ENTER:
          // enter
          state = PORTAM_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
        case BTN_MENU:
          // menu
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
      }
    }
  } else if (state == PORTAM_ADJ_MAX){
    redraw |= updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      // TODO: Ask Johan what the minOffset is for...
      // if ((portamMaxVal - portamStep) > (portamThrVal + minOffset)){

      redraw |= drawAdjustBar(deumButtonState, 50, &portamMaxVal, portamLoLimit, portamHiLimit, &pos2);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = PORTAM_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          redraw = true;
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
    redraw |= updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = EXTRAC_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = PITCHB_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = PORTAM_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          writeSetting(PITCHB_THR_ADDR,pitchbThrVal);
          writeSetting(PITCHB_MAX_ADDR,pitchbMaxVal);
          break;
      }
    }
  } else if (state == PITCHB_ADJ_THR){
    redraw |= updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      cursorBlinkTime = timeNow;
      redraw |= drawAdjustBar(deumButtonState, 20, &pitchbThrVal, pitchbLoLimit, pitchbHiLimit, &pos1);
      switch (deumButtonState){
        case BTN_ENTER:
          // enter
          state = PITCHB_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
        case BTN_MENU:
          // menu
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
      }
    }
  } else if (state == PITCHB_ADJ_MAX){
    redraw |= updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      redraw |= drawAdjustBar(deumButtonState, 50, &portamMaxVal, portamLoLimit, portamHiLimit, &pos2);
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = PITCHB_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          redraw = true;
          break;
      }
    }

  } else if (state == EXTRAC_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("EXTRA CONTROLLER", extracThrVal, extracMaxVal, extracLoLimit, extracHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }

    redraw |= updateAdjustCursor(timeNow);

    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = CTOUCH_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
        case BTN_ENTER:
          // enter
          state = EXTRAC_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = PITCHB_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          writeSetting(EXTRAC_THR_ADDR,extracThrVal);
          writeSetting(EXTRAC_MAX_ADDR,extracMaxVal);
          break;
      }
    }
  } else if (state == EXTRAC_ADJ_THR){
    redraw |= updateAdjustLineCursor(timeNow, pos1, 20);

    if (buttonPressedAndNotUsed){
      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      redraw |= drawAdjustBar(deumButtonState, 20, &extracThrVal, extracLoLimit, extracHiLimit, &pos1);
      switch (deumButtonState){
        case BTN_ENTER:
          state = EXTRAC_ADJ_MAX;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
        case BTN_MENU:
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
          break;
      }
    }
  } else if (state == EXTRAC_ADJ_MAX){
    redraw |= updateAdjustLineCursor(timeNow, pos2, 50);
    if (buttonPressedAndNotUsed){
      redraw |= drawAdjustBar(deumButtonState, 50, &extracMaxVal, extracLoLimit, extracHiLimit, &pos2);

      cursorBlinkTime = timeNow;
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = EXTRAC_ADJ_IDL;
          display.drawLine(pos2,50,pos2,56,WHITE);
          redraw = true;
          break;
      }
    }

  } else if (state == CTOUCH_ADJ_IDL){
    if (stateFirstRun) {
      drawAdjustScreen("TOUCH SENSE", ctouchThrVal, -1, ctouchLoLimit, ctouchHiLimit);
      forcePix = 1;
      stateFirstRun = 0;
    }
    redraw |= updateAdjustCursor(timeNow);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case BTN_DOWN:
          // down
          state = BREATH_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
        case BTN_ENTER:
          // enter
          state = CTOUCH_ADJ_THR;
          break;
        case BTN_UP:
          // up
          state = EXTRAC_ADJ_IDL;
          stateFirstRun = 1;
          writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
        case BTN_MENU:
          // menu
          state = MAIN_MENU;
          stateFirstRun = 1;
          writeSetting(CTOUCH_THR_ADDR,ctouchThrVal);
          break;
      }
    }
  } else if (state == CTOUCH_ADJ_THR){
    redraw |= updateAdjustLineCursor(timeNow, pos1, 20);
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;

      bool updated = drawAdjustBar(deumButtonState, 20, &ctouchThrVal, ctouchLoLimit, ctouchHiLimit, &pos1);
      if(updated) {
        touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);
        redraw = true;
      }

      switch (deumButtonState){
        case BTN_ENTER:
        case BTN_MENU:
          state = CTOUCH_ADJ_IDL;
          display.drawLine(pos1,20,pos1,26,WHITE);
          redraw = true;
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
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotBreathCC(BLACK);
            if (breathCC > 0){
              breathCC--;
            } else {
              breathCC = 10;
            }
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotBreathCC(BLACK);
            if (breathCC < 10){
              breathCC++;
            } else {
              breathCC = 0;
            }
            cursorBlinkTime = timeNow;
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
        plotBreathCC(WHITE);
        cursorNow = BLACK;
        redraw = true;
      }
    } else if (subBreathAT) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotBreathAT(cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotBreathAT(BLACK);
            breathAT=!breathAT;
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotBreathAT(BLACK);
            breathAT=!breathAT;
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            subBreathAT = 0;
            if (readSetting(BREATH_AT_ADDR) != breathAT){
              writeSetting(BREATH_AT_ADDR,breathAT);
              midiReset();
            }
            break;
        }
        plotBreathAT(WHITE);
        cursorNow = BLACK;
        redraw = true;
      }
    } else if (subVelocity) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelocity(cursorNow);
        redraw = true;
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
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            plotVelocity(BLACK);
            if (velocity < 127){
              velocity++;
            } else velocity = 0;
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            subVelocity = 0;
            writeSetting(VELOCITY_ADDR,velocity);
            break;
        }
        plotVelocity(WHITE);
        cursorNow = BLACK;
        redraw = true;
      }

    } else if (subCurve) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotCurve(cursorNow);
        redraw = true;
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
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotCurve(BLACK);
            if (curve < 12){
              curve++;
            } else curve = 0;
            plotCurve(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotCurve(WHITE);
            subCurve = 0;
            writeSetting(BREATHCURVE_ADDR,curve);
            break;
        }
        cursorNow = BLACK;
        redraw = true;
      }

    } else if (subVelSmpDl) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelSmpDl(cursorNow);
        redraw = true;
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
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotVelSmpDl(BLACK);
            if (velSmpDl < 30){
              velSmpDl+=1;
            } else velSmpDl = 0;
            plotVelSmpDl(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotVelSmpDl(WHITE);
            subVelSmpDl = 0;
            writeSetting(VEL_SMP_DL_ADDR,velSmpDl);
            break;
        }
        cursorNow = BLACK;
        redraw = true;
      }

     } else if (subVelBias) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVelBias(cursorNow);
        redraw = true;
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
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER:
            // enter
            plotVelBias(WHITE);
            subVelBias = 0;
            writeSetting(VEL_BIAS_ADDR,velBias);
            break;
          case BTN_UP:
            // up
            plotVelBias(BLACK);
            if (velBias < 9){
              velBias++;
            } else velBias = 0;
            plotVelBias(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_MENU:
            // menu
            plotVelBias(WHITE);
            subVelBias = 0;
            writeSetting(VEL_BIAS_ADDR,velBias);
            break;
        }
        cursorNow = BLACK;
        redraw = true;
      }

    } else {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(setupBrMenuCursor, cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (setupBrMenuCursor < 5){
              drawMenuCursor(setupBrMenuCursor, BLACK);
              setupBrMenuCursor++;
              drawMenuCursor(setupBrMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
            }
            break;
          case BTN_ENTER:
            // enter
            redraw |= selectSetupBrMenu(setupBrMenuCursor);
            break;
          case BTN_UP:
            // up
            if (setupBrMenuCursor > 0){
              drawMenuCursor(setupBrMenuCursor, BLACK);
              setupBrMenuCursor--;
              drawMenuCursor(setupBrMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
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
        redraw = true;
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
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotPort(BLACK);
            if (portamento < 2){
              portamento++;
            } else portamento = 0;
            plotPort(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotPort(WHITE);
            subPort = 0;
            writeSetting(PORTAM_ADDR,portamento);
            break;
        }
        cursorNow = BLACK;
        redraw = true;
      }
    } else if (subPB) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPB(cursorNow);
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (PBdepth < 12){
              plotPB(BLACK);
              PBdepth++;
              plotPB(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotPB(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subPB = 0;
            writeSetting(PB_ADDR,PBdepth);
            break;
        }
      }
    } else if (subExtra) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotExtra(cursorNow);
        redraw = true;
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
            redraw = true;
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotExtra(BLACK);
            if (extraCT < 4){
              extraCT++;
            } else extraCT = 0;
            plotExtra(WHITE);
            cursorNow = BLACK;
            redraw = true;
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotExtra(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subExtra = 0;
            writeSetting(EXTRA_ADDR,extraCT);
            break;
        }
      }
    } else if (subDeglitch) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotDeglitch(cursorNow);
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (deglitch < 70){
              plotDeglitch(BLACK);
              deglitch+=1;
              plotDeglitch(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotDeglitch(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subDeglitch = 0;
            writeSetting(DEGLITCH_ADDR,deglitch);
            break;
        }
      }
    } else if (subPinky) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotPinkyKey(cursorNow);
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (pinkySetting < 24){
              plotPinkyKey(BLACK);
              pinkySetting+=1;
              plotPinkyKey(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotPinkyKey(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subPinky = 0;
            writeSetting(PINKY_KEY_ADDR,pinkySetting);
            break;
        }
      }
    } else {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(setupCtMenuCursor, cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (setupCtMenuCursor < 5){
              drawMenuCursor(setupCtMenuCursor, BLACK);
              setupCtMenuCursor++;
              drawMenuCursor(setupCtMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
            }
            break;
          case BTN_ENTER:
            // enter
            redraw |= selectSetupCtMenu(setupCtMenuCursor);
            break;
          case BTN_UP:
            // up
            if (setupCtMenuCursor > 0){
              drawMenuCursor(setupCtMenuCursor, BLACK);
              setupCtMenuCursor--;
              drawMenuCursor(setupCtMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
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
        redraw  = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (vibrato < 9){
              plotVibrato(BLACK);
              vibrato++;
              plotVibrato(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotVibrato(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subVibrato = 0;
            writeSetting(VIBRATO_ADDR,vibrato);
            break;
        }
      }
    } else if (subVibSens) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibSens(cursorNow);
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (vibSens < 12){
              plotVibSens(BLACK);
              vibSens++;
              plotVibSens(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotVibSens(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subVibSens = 0;
            writeSetting(VIB_SENS_ADDR,vibSens);
            break;
        }
      }
    } else if (subVibRetn) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibRetn(cursorNow);
        redraw = true;
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
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotVibRetn(BLACK);
            if (vibRetn < 4){
              vibRetn++;
            }
            plotVibRetn(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotVibRetn(WHITE);
            subVibRetn = 0;
            writeSetting(VIB_RETN_ADDR,vibRetn);
            break;
        }
        cursorNow = BLACK;
        redraw = true;
      }
    } else if (subVibSquelch) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibSquelch(cursorNow);
        redraw = true;
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
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_UP:
            // up
            if (vibSquelch < 30){
              plotVibSquelch(BLACK);
              vibSquelch++;
              plotVibSquelch(WHITE);
              cursorNow = BLACK;
              redraw = true;
              cursorBlinkTime = timeNow;
            }
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotVibSquelch(WHITE);
            cursorNow = BLACK;
            redraw = true;
            subVibSquelch = 0;
            writeSetting(VIB_SQUELCH_ADDR,vibSquelch);
            break;
        }
      }
    } else if (subVibDirection) {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        plotVibDirection(cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            plotVibDirection(BLACK);
            vibDirection = !vibDirection;
            plotVibDirection(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_UP:
            // up
            plotVibDirection(BLACK);
            vibDirection = !vibDirection;
            plotVibDirection(WHITE);
            cursorBlinkTime = timeNow;
            break;
          case BTN_ENTER: // fallthrough
          case BTN_MENU:
            // menu
            plotVibDirection(WHITE);
            subVibDirection = 0;
            writeSetting(VIB_DIRECTION_ADDR,vibDirection);
            break;
        }
        cursorNow = BLACK;
        redraw = true;
      }
    } else {
      if ((timeNow - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE;
        drawMenuCursor(vibratoMenuCursor, cursorNow);
        redraw = true;
        cursorBlinkTime = timeNow;
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case BTN_DOWN:
            if (vibratoMenuCursor < 4){
              drawMenuCursor(vibratoMenuCursor, BLACK);
              vibratoMenuCursor++;
              drawMenuCursor(vibratoMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
            }
            break;
          case BTN_ENTER:
            // enter
            redraw |= selectVibratoMenu(vibratoMenuCursor);
            break;
          case BTN_UP:
            // up
            if (vibratoMenuCursor > 0){
              drawMenuCursor(vibratoMenuCursor, BLACK);
              vibratoMenuCursor--;
              drawMenuCursor(vibratoMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              redraw = true;
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

  if(redraw) {
    display.display();
  }
}
