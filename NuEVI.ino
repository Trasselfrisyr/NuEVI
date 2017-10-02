#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
NAME:                 NuEVI
WRITTEN BY:           JOHAN BERGLUND
DATE:                 2017-08-26
FILE SAVED AS:        NuEVI.ino
FOR:                  PJRC Teensy LC or 3.2 and a MPR121 capactive touch sensor board.
                      Uses an SSD1306 controlled OLED dispaly communicating over I2C.
PROGRAMME FUNCTION:   EVI Wind Controller using the Freescale MP3V5004GP breath sensor
                      and capacitive touch keys. Output to both USB MIDI and DIN MIDI.  
  
*/

//_______________________________________________________________________________________________ DECLARATIONS

// Compile options, comment/uncomment to change

//#define REVB


// Pin definitions

// Teensy pins

#define specialKeyPin 0       
#define halfPitchBendKeyPin 1 

#define bitePin 17
#define extraPin 16
#define pbUpPin 23
#define pbDnPin 22
#define vibratoPin 15

#define dPin 3
#define ePin 4
#define uPin 5
#define mPin 6

#define bLedPin 10
#define pLedPin 9 

#if defined(REVB)

// MPR121 pins Rev B (angled pins at top edge for main keys and rollers)

#define R1Pin 0
#define R2Pin 2
#define R3Pin 4
#define R4Pin 6
#define R5Pin 8

#define K4Pin 10
#define K1Pin 1
#define K2Pin 3
#define K3Pin 5
#define K5Pin 7
#define K6Pin 9
#define K7Pin 11

/*
 *    PINOUT ON PCB vs PINS ON MPR121 - Rev. B
 * 
 *    (R1)  (R2) (R3/6) (R4)  (R5)  (K4)  <-> (00)  (02)  (04)  (06)  (08)  (10)
 * 
 *    (K1)  (K2)  (K3)  (K5)  (K6)  (K7)  <-> (01)  (03)  (05)  (07)  (09)  (11)
 * 
 */
 
# else

// MPR121 pins Rev A (upright pins below MPR121 for main keys and rollers)

#define R1Pin 10
#define R2Pin 11
#define R3Pin 8
#define R4Pin 9
#define R5Pin 6

#define K4Pin 7
#define K1Pin 4
#define K2Pin 5
#define K3Pin 2
#define K5Pin 3
#define K6Pin 0
#define K7Pin 1

/*
 *    PINOUT ON PCB vs PINS ON MPR121 - Rev. A
 * 
 *    (R2)  (R4)  (K4)  (K2)  (K5)  (K7)  <-> (11)  (09)  (07)  (05)  (03)  (01)
 * 
 *    (R1) (R3/6) (R5)  (K1)  (K3)  (K6)  <-> (10)  (08)  (06)  (04)  (02)  (00)
 * 
 */
 
#endif

#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define touch_Thr 1200  // sensitivity for Teensy touch sensors
#define CCN_Port 5      // Controller number for portamento level
#define CCN_PortOnOff 65// Controller number for portamento on/off


// Send CC data no more than every CC_INTERVAL
// milliseconds
#define CC_INTERVAL 5 


// The three states of our main state machine

// No note is sounding
#define NOTE_OFF 1

// We've observed a transition from below to above the
// threshold value. We wait a while to see how fast the
// breath velocity is increasing
#define RISE_WAIT 2

// A note is sounding
#define NOTE_ON 3


//display states
#define DISPLAYOFF_IDL 0
#define MAIN_MENU 1
#define PATCH_VIEW 2
#define BREATH_ADJ_IDL 10
#define BREATH_ADJ_THR 11
#define BREATH_ADJ_MAX 12
#define PORTAM_ADJ_IDL 20
#define PORTAM_ADJ_THR 21
#define PORTAM_ADJ_MAX 22
#define PITCHB_ADJ_IDL 30
#define PITCHB_ADJ_THR 31
#define PITCHB_ADJ_MAX 32
#define EXTRAC_ADJ_IDL 40
#define EXTRAC_ADJ_THR 41
#define EXTRAC_ADJ_MAX 42
#define VIBRAT_ADJ_IDL 50
#define VIBRAT_ADJ_THR 51
#define VIBRAT_ADJ_DPT 52
#define CTOUCH_ADJ_IDL 60
#define CTOUCH_ADJ_THR 61
#define SETUP_BR_MENU 80
#define SETUP_CT_MENU 90

// EEPROM addresses for settings
#define VERSION_ADDR 0
#define BREATH_THR_ADDR 2
#define BREATH_MAX_ADDR 4
#define PORTAM_THR_ADDR 6
#define PORTAM_MAX_ADDR 8
#define PITCHB_THR_ADDR 10
#define PITCHB_MAX_ADDR 12
#define TRANSP_ADDR 14
#define MIDI_ADDR 16
#define BREATH_CC_ADDR 18
#define BREATH_AT_ADDR 20
#define VELOCITY_ADDR 22
#define PORTAM_ADDR 24
#define PB_ADDR 26
#define EXTRA_ADDR 28
#define VIBRATO_ADDR 30
#define DEGLITCH_ADDR 32
#define EXTRAC_THR_ADDR 34
#define EXTRAC_MAX_ADDR 36
#define PATCH_ADDR 38
#define OCTAVE_ADDR 40
#define CTOUCH_THR_ADDR 42
#define BREATHCURVE_ADDR 44

//"factory" values for settings
#define VERSION 22
#define BREATH_THR_FACTORY 1400
#define BREATH_MAX_FACTORY 4000
#define PORTAM_THR_FACTORY 1730
#define PORTAM_MAX_FACTORY 3300
#define PITCHB_THR_FACTORY 1300
#define PITCHB_MAX_FACTORY 2400
#define EXTRAC_THR_FACTORY 1200
#define EXTRAC_MAX_FACTORY 2400
#define TRANSP_FACTORY 12   // 12 is 0 transpose
#define MIDI_FACTORY 1      // 1-16
#define BREATH_CC_FACTORY 2 //thats CC#2, see ccList
#define BREATH_AT_FACTORY 0 //aftertouch default off
#define VELOCITY_FACTORY 0  // 0 is dynamic/breath controlled velocity
#define PORTAM_FACTORY 2    // 0 - OFF, 1 - ON, 2 - SW 
#define PB_FACTORY 1        // 0 - OFF, 1 - 12
#define EXTRA_FACTORY 2     // 0 - OFF, 1 - Modulation wheel, 2 - Foot pedal, 3 - Sustain pedal
#define VIBRATO_FACTORY 3   // 0 - OFF, 1 - 6 depth
#define DEGLITCH_FACTORY 20 // 0 - OFF, 5 to 70 ms in steps of 5
#define PATCH_FACTORY 1     // MIDI program change 1-128
#define OCTAVE_FACTORY 3    // 3 is 0 octave change
#define CTOUCH_THR_FACTORY 125  // MPR121 touch threshold
#define BREATHCURVE_FACTORY 2 // 0 to 4 (-2 to +2)

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

 // 'NuEVI' logo
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

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


//variables setup

unsigned short breathThrVal;// = 350;
unsigned short breathMaxVal;// = 1000;
unsigned short portamThrVal;// = 1730;
unsigned short portamMaxVal;// = 3300;
unsigned short pitchbThrVal;// = 1200;
unsigned short pitchbMaxVal;// = 2400;
unsigned short extracThrVal;// = 1200;
unsigned short extracMaxVal;// = 2400;
unsigned short ctouchThrVal;// = 120;
unsigned short transpose;
unsigned short MIDIchannel;
unsigned short breathCC;  // OFF:MW+:BR:VOL:EXP
unsigned short breathAT;
unsigned short velocity;
unsigned short portamento;// switching on cc65? just cc5 enabled? SW:ON:OFF
unsigned short PBdepth;   // OFF:1-12 divider
unsigned short extraCT;   // OFF:MW:FP:SP
unsigned short vibrato;   // OFF:1-6
unsigned short deglitch;  // 0-70 ms in steps of 5
unsigned short patch;     // 1-128
unsigned short octave;      
unsigned short curve;


int breathLoLimit = 0;
int breathHiLimit = 4095;
int portamLoLimit = 1000;
int portamHiLimit = 5000;
int pitchbLoLimit = 500;
int pitchbHiLimit = 4000;
int extracLoLimit = 500;
int extracHiLimit = 4000;
int ctouchLoLimit = 50;
int ctouchHiLimit = 350;

int breathStep;
int portamStep;
int pitchbStep;
int extracStep;
int ctouchStep;

int minOffset = 50;

int deumButtons = 0;
int lastDeumButtons = 0;
int deumButtonState = 0;
byte buttonPressedAndNotUsed = 0;

byte mainMenuCursor = 1;
byte setupBrMenuCursor = 1;
byte setupCtMenuCursor = 1;

byte state = 0;
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

byte ccList[9] = {0,1,2,7,11,1,2,7,11};  // OFF, Modulation (Hi-res), Breath, Volume, Expression (then same sent in hires)

int pbDepthList[13] = {0,8192,4096,2731,2048,1638,1365,1170,1024,910,819,744,683};

byte cursorNow;

int pos1;
int pos2;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;         // the last time the output pin was toggled
unsigned long debounceDelay = 20;           // the debounce time; increase if the output flickers
unsigned long buttonRepeatTime = 0;
unsigned long buttonPressedTime = 0;
unsigned long buttonRepeatInterval = 50;
unsigned long buttonRepeatDelay = 400;
unsigned long pixelUpdateTime = 0;
unsigned long pixelUpdateInterval = 100;
unsigned long cursorBlinkTime = 0;          // the last time the cursor was toggled
unsigned long cursorBlinkInterval = 300;    // the cursor blink toggle interval time
unsigned long patchViewTime = 0;
unsigned long patchViewTimeUp = 2000;       // ms until patch view shuts off
unsigned long menuTime = 0;
unsigned long menuTimeUp = 60000;           // menu shuts off after one minute of button inactivity
unsigned long lastDeglitchTime = 0;         // The last time the fingering was changed
unsigned long ccSendTime = 0L;              // The last time we sent CC values
unsigned long breath_on_time = 0L;          // Time when breath sensor value went over the ON threshold

int lastFingering = 0;             // Keep the last fingering value for debouncing

int mainState;                         // The state of the main state machine

int initial_breath_value;          // The breath value at the time we observed the transition

byte activeMIDIchannel=1;          // MIDI channel
byte activePatch=0;                
byte doPatchUpdate=1;

int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;
unsigned int oldbreathhires=0;

int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
int lastPressure;
byte velocitySend;   // remapped midi velocity from breath sensor (or set to static value if selected)

int biteSensor=0;    // capacitance data from bite sensor, for midi cc and threshold checks
byte portIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
int oldport=0;
int lastBite=0;

int exSensor=0;
byte extracIsOn=0;
int oldextrac=0;
int lastEx=0;

int pitchBend=8192;
int oldpb=8192;
int pbUp=0;
int pbDn=0;
int lastPbUp=0;
int lastPbDn=0;

int vibDepth[7] = {0,254,511,767,1023,1279,1535}; // max pitch bend values (+/-) for the vibrato settings 

unsigned int curveIn[] = {0,1023,2047,3071,4095,5119,6143,7167,8191,9215,10239,11263,12287,13311,14335,15359,16383};
unsigned int curveM2[] = {0,4300,7000,8700,9800,10800,11900,12500,13300,13900,14500,15000,15500,15700,16000,16250,16383};
unsigned int curveM1[] = {0,2000,3600,5000,6450,7850,9000,10100,11100,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveP1[] = {0,400,800,1300,2050,2650,3500,4300,5300,6250,7400,8500,9600,11050,12400,14100,16383};
unsigned int curveP2[] = {0,100,200,400,700,1050,1500,1950,2550,3200,4000,4900,6050,7500,9300,12100,16282};

int vibThr=1900;     // this gets auto calibrated in setup
int oldvibRead=0;
byte dirUp=0;        // direction of first vibrato wave

int fingeredNote;    // note calculated from fingering (switches), transpose and octave settings
byte activeNote;     // note playing
byte startNote=36;   // set startNote to C (change this value in steps of 12 to start in other octaves)
int slurBase;        // first note in slur sustain chord

int slurInterval[9] = {-5,0,0,0,0,0,0,0,0};
byte addedIntervals = 1;

// Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
byte K1;   // Valve 1 (pitch change -2) 
byte K2;   // Valve 2 (pitch change -1)
byte K3;   // Valve 3 (pitch change -3)
byte K4;   // Left Hand index finger (pitch change -5)
byte K5;   // Trill key 1 (pitch change +2)
byte K6;   // Trill key 2 (pitch change +1)
byte K7;   // Trill key 3 (pitch change +4)

byte octaveR = 0;

byte halfPitchBendKey;
byte specialKey;
byte lastSpecialKey = 0;

byte slurSustain = 0;
byte parallelChord = 0;
byte subOctaveDouble = 0;

byte breathLedBrightness = 100; // up to 255, PWM
byte portamLedBrightness = 100; // up to 255, PWM

Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor


//_______________________________________________________________________________________________ SETUP

void setup() {
  // if stored settings are not for current version, they are replaced by factory settings
  if (readSetting(VERSION_ADDR) != VERSION){
    writeSetting(VERSION_ADDR,VERSION);
    writeSetting(BREATH_THR_ADDR,BREATH_THR_FACTORY);
    writeSetting(BREATH_MAX_ADDR,BREATH_MAX_FACTORY);
    writeSetting(PORTAM_THR_ADDR,PORTAM_THR_FACTORY);  
    writeSetting(PORTAM_MAX_ADDR,PORTAM_MAX_FACTORY); 
    writeSetting(PITCHB_THR_ADDR,PITCHB_THR_FACTORY);
    writeSetting(PITCHB_MAX_ADDR,PITCHB_MAX_FACTORY);
    writeSetting(TRANSP_ADDR,TRANSP_FACTORY);
    writeSetting(MIDI_ADDR,MIDI_FACTORY);
    writeSetting(BREATH_CC_ADDR,BREATH_CC_FACTORY);
    writeSetting(BREATH_AT_ADDR,BREATH_AT_FACTORY);
    writeSetting(VELOCITY_ADDR,VELOCITY_FACTORY);
    writeSetting(PORTAM_ADDR,PORTAM_FACTORY);
    writeSetting(PB_ADDR,PB_FACTORY);
    writeSetting(EXTRA_ADDR,EXTRA_FACTORY);
    writeSetting(VIBRATO_ADDR,VIBRATO_FACTORY);
    writeSetting(DEGLITCH_ADDR,DEGLITCH_FACTORY);
    writeSetting(EXTRAC_THR_ADDR,EXTRAC_THR_FACTORY);
    writeSetting(EXTRAC_MAX_ADDR,EXTRAC_MAX_FACTORY);
    writeSetting(PATCH_ADDR,PATCH_FACTORY);
    writeSetting(OCTAVE_ADDR,OCTAVE_FACTORY);
    writeSetting(CTOUCH_THR_ADDR,CTOUCH_THR_FACTORY);
    writeSetting(BREATHCURVE_ADDR,BREATHCURVE_FACTORY);
  }
  // read settings from EEPROM
  breathThrVal = readSetting(BREATH_THR_ADDR);
  breathMaxVal = readSetting(BREATH_MAX_ADDR);
  portamThrVal = readSetting(PORTAM_THR_ADDR);
  portamMaxVal = readSetting(PORTAM_MAX_ADDR);
  pitchbThrVal = readSetting(PITCHB_THR_ADDR);
  pitchbMaxVal = readSetting(PITCHB_MAX_ADDR);
  transpose    = readSetting(TRANSP_ADDR);
  MIDIchannel  = readSetting(MIDI_ADDR);
  breathCC     = readSetting(BREATH_CC_ADDR);
  breathAT     = readSetting(BREATH_AT_ADDR);
  velocity     = readSetting(VELOCITY_ADDR);
  portamento   = readSetting(PORTAM_ADDR);
  PBdepth      = readSetting(PB_ADDR);
  extraCT      = readSetting(EXTRA_ADDR);
  vibrato      = readSetting(VIBRATO_ADDR);
  deglitch     = readSetting(DEGLITCH_ADDR);
  extracThrVal = readSetting(EXTRAC_THR_ADDR);
  extracMaxVal = readSetting(EXTRAC_MAX_ADDR);
  patch        = readSetting(PATCH_ADDR);
  octave       = readSetting(OCTAVE_ADDR);
  ctouchThrVal = readSetting(CTOUCH_THR_ADDR);
  curve        = readSetting(BREATHCURVE_ADDR);
 
  breathStep = (breathHiLimit - breathLoLimit)/92; // 92 is the number of pixels in the settings bar
  portamStep = (portamHiLimit - portamLoLimit)/92;
  pitchbStep = (pitchbHiLimit - pitchbLoLimit)/92;   
  extracStep = (extracHiLimit - extracLoLimit)/92;
  ctouchStep = (ctouchHiLimit - ctouchLoLimit)/92;

  analogReadResolution(12);   // set resolution of ADCs to 12 bit
     
  pinMode(dPin, INPUT_PULLUP);
  pinMode(ePin, INPUT_PULLUP);
  pinMode(uPin, INPUT_PULLUP);
  pinMode(mPin, INPUT_PULLUP);

  pinMode(bLedPin, OUTPUT); // breath indicator LED
  pinMode(pLedPin, OUTPUT);  // portam indicator LED
  
  if (!touchSensor.begin(0x5A)) {
    while (1);  // Touch sensor initialization failed - stop doing stuff
  }

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  display.clearDisplay();
  display.drawBitmap(0,0,nuevi_logo_bmp,128,64,1);
  display.display();
  
  //auto-calibrate the vibrato threshold while showing splash screen
  int cv1=touchRead(15);
  delay(1000);
  int cv2=touchRead(15);
  delay(1000);
  int cv3=touchRead(15);
  delay(1000);
  int cv4=touchRead(15);
  vibThr=(cv1+cv2+cv3+cv4)/4-70;
  
  delay(1000); 
  
  state = DISPLAYOFF_IDL;
  mainState = NOTE_OFF;       // initialize main state machine
  
  Serial3.begin(31250);   // start serial with midi baudrate 31250
  Serial3.flush();
  
}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {
  pressureSensor = analogRead(A0); // Get the pressure sensor reading from analog pin A0, input from sensor MP3V5004GP

  if (mainState == NOTE_OFF) {
    if (activeMIDIchannel != MIDIchannel) activeMIDIchannel = MIDIchannel; // only switch channel if no active note
    if ((activePatch != patch) && doPatchUpdate){
      activePatch = patch;
      usbMIDI.sendProgramChange(activePatch-1,activeMIDIchannel);
      dinMIDIsendProgramChange(activePatch-1,activeMIDIchannel-1);
      if (readSetting(PATCH_ADDR) != activePatch) writeSetting(PATCH_ADDR,activePatch); 
      slurSustain = 0;
      parallelChord = 0;
      subOctaveDouble = 0;
      doPatchUpdate = 0;
    }
    if (pressureSensor > breathThrVal) {
      // Value has risen above threshold. Move to the ON_Delay
      // state. Record time and initial breath value.
      breath_on_time = millis();
      initial_breath_value = pressureSensor;
      mainState = RISE_WAIT;  // Go to next state
    }
    specialKey=(touchRead(specialKeyPin) > touch_Thr);        //S2 on pcb
    if (lastSpecialKey != specialKey){
      if (specialKey){
        // special key just pressed, check other keys
        readSwitches();
        if (K4) {
          if (!slurSustain) {
            slurSustain = 1;
            parallelChord = 0;
          } else slurSustain = 0;
        }
        if (K5) {
          if (!parallelChord) {
            parallelChord = 1;
            slurSustain = 0;
          } else parallelChord = 0;
        }
        if (K1) subOctaveDouble = !subOctaveDouble;
        if (halfPitchBendKey) midiPanic();
      }
    }
    lastSpecialKey = specialKey;
  } else if (mainState == RISE_WAIT) {
    if (pressureSensor > breathThrVal) {
      // Has enough time passed for us to collect our second
      // sample?
      if (millis() - breath_on_time > ON_Delay) {
        // Yes, so calculate MIDI note and velocity, then send a note on event
        readSwitches();
        // We should be at tonguing peak, so set velocity based on current pressureSensor value unless fixed velocity is set     
        if (!velocity) {
          unsigned int breathValHires = breathCurve(map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,16383));
          velocitySend = (breathValHires >>7) & 0x007F;
          velocitySend = constrain(velocitySend,1,127);
          //velocitySend = map(constrain(max(pressureSensor,initial_breath_value),breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,1,127);
        } else velocitySend = velocity;
        breathLevel=constrain(max(pressureSensor,initial_breath_value),breathThrVal,breathMaxVal);           
        breath(); // send breath data
        fingeredNote=noteValueCheck(fingeredNote);
        usbMIDI.sendNoteOn(fingeredNote, velocitySend, activeMIDIchannel); // send Note On message for new note 
        dinMIDIsendNoteOn(fingeredNote, velocitySend, activeMIDIchannel - 1);
        if (parallelChord){
          for (int i=0; i < addedIntervals; i++){
            usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]), velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]), velocitySend, activeMIDIchannel - 1);
          }
        }
        if (slurSustain){
          usbMIDI.sendControlChange(64,127, activeMIDIchannel);
          dinMIDIsendControlChange(64,127, activeMIDIchannel - 1); 
          slurBase = fingeredNote;
          addedIntervals = 0;
        }
        if (subOctaveDouble){
          usbMIDI.sendNoteOn(noteValueCheck(fingeredNote-12), velocitySend, activeMIDIchannel);
          dinMIDIsendNoteOn(noteValueCheck(fingeredNote-12), velocitySend, activeMIDIchannel - 1);
          if (parallelChord){
            for (int i=0; i < addedIntervals; i++){
              usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]-12), velocitySend, activeMIDIchannel); // send Note On message for new note 
              dinMIDIsendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]-12), velocitySend, activeMIDIchannel - 1);
            }
          }
        }
        activeNote=fingeredNote;
        mainState = NOTE_ON;
      }
    } else {
      // Value fell below threshold before ON_Delay passed. Return to
      // NOTE_OFF state (e.g. we're ignoring a short blip of breath)
      mainState = NOTE_OFF;
    }
  } else if (mainState == NOTE_ON) {
    cursorBlinkTime = millis(); // keep display from updating with cursor blinking if note on
    if (pressureSensor < breathThrVal) {
      // Value has fallen below threshold - turn the note off
      activeNote=noteValueCheck(activeNote);
      usbMIDI.sendNoteOff(activeNote, velocitySend, activeMIDIchannel); //  send Note Off message 
      dinMIDIsendNoteOff(activeNote, velocitySend, activeMIDIchannel - 1);
      if (parallelChord){
        for (int i=0; i < addedIntervals; i++){
          usbMIDI.sendNoteOff(noteValueCheck(activeNote+slurInterval[i]), velocitySend, activeMIDIchannel); // send Note On message for new note 
          dinMIDIsendNoteOff(noteValueCheck(activeNote+slurInterval[i]), velocitySend, activeMIDIchannel - 1);
        }
      }
      if (subOctaveDouble){
        usbMIDI.sendNoteOff(noteValueCheck(activeNote-12), velocitySend, activeMIDIchannel);
        dinMIDIsendNoteOff(noteValueCheck(activeNote-12), velocitySend, activeMIDIchannel - 1);
        if (parallelChord){
          for (int i=0; i < addedIntervals; i++){
            usbMIDI.sendNoteOff(noteValueCheck(activeNote+slurInterval[i]-12), velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOff(noteValueCheck(activeNote+slurInterval[i]-12), velocitySend, activeMIDIchannel - 1);
          }
        }
      }      
      if (slurSustain){
          usbMIDI.sendControlChange(64,0, activeMIDIchannel);
          dinMIDIsendControlChange(64,0, activeMIDIchannel - 1); 
      }
      breathLevel=0;
      mainState = NOTE_OFF;
    } else {
      readSwitches();
      if (fingeredNote != lastFingering){ //
        // reset the debouncing timer
        lastDeglitchTime = millis();
      }
      if ((millis() - lastDeglitchTime) > deglitch) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state
        if (noteValueCheck(fingeredNote) != activeNote) {
          // Player has moved to a new fingering while still blowing.
          // Send a note off for the current note and a note on for
          // the new note.
          if (!velocity){      
            unsigned int breathValHires = breathCurve(map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,16383));
            velocitySend = (breathValHires >>7) & 0x007F;
            velocitySend = constrain(velocitySend,1,127);
            //velocitySend = map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,7,127); // set new velocity value based on current pressure sensor level
          }
          activeNote=noteValueCheck(activeNote);
          if (parallelChord || subOctaveDouble){ // poly playing, send old note off before new note on
          usbMIDI.sendNoteOff(activeNote, velocitySend, activeMIDIchannel); // send Note Off message for old note
          dinMIDIsendNoteOff(activeNote, velocitySend, activeMIDIchannel - 1);
          }
          
          if (parallelChord){
            for (int i=0; i < addedIntervals; i++){
              usbMIDI.sendNoteOff(noteValueCheck(activeNote+slurInterval[i]), velocitySend, activeMIDIchannel); // send Note Off message for old note
              dinMIDIsendNoteOff(noteValueCheck(activeNote+slurInterval[i]), velocitySend, activeMIDIchannel - 1);
            }
          }
          if (subOctaveDouble){
            usbMIDI.sendNoteOff(noteValueCheck(activeNote-12), velocitySend, activeMIDIchannel); // send Note Off message for old note
            dinMIDIsendNoteOff(noteValueCheck(activeNote-12), velocitySend, activeMIDIchannel - 1);
            if (parallelChord){
              for (int i=0; i < addedIntervals; i++){
                usbMIDI.sendNoteOff(noteValueCheck(activeNote+slurInterval[i]-12), velocitySend, activeMIDIchannel); // send Note Off message for old note
                dinMIDIsendNoteOff(noteValueCheck(activeNote+slurInterval[i]-12), velocitySend, activeMIDIchannel - 1);
              }
            }
          }
          
          fingeredNote=noteValueCheck(fingeredNote);
          usbMIDI.sendNoteOn(fingeredNote, velocitySend, activeMIDIchannel); // send Note On message for new note 
          dinMIDIsendNoteOn(fingeredNote, velocitySend, activeMIDIchannel - 1);
          if (parallelChord){
            for (int i=0; i < addedIntervals; i++){
              usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]), velocitySend, activeMIDIchannel); // send Note On message for new note 
              dinMIDIsendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]), velocitySend, activeMIDIchannel - 1);
            }
          }
          if (subOctaveDouble){
            usbMIDI.sendNoteOn(noteValueCheck(fingeredNote-12), velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOn(noteValueCheck(fingeredNote-12), velocitySend, activeMIDIchannel - 1);
            if (parallelChord){
              for (int i=0; i < addedIntervals; i++){
                usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]-12), velocitySend, activeMIDIchannel); // send Note On message for new note 
                dinMIDIsendNoteOn(noteValueCheck(fingeredNote+slurInterval[i]-12), velocitySend, activeMIDIchannel - 1);
              }
            }
          }

          if (!parallelChord && !subOctaveDouble){ // mono playing, send old note off after new note on
            usbMIDI.sendNoteOff(activeNote, velocitySend, activeMIDIchannel); //  send Note Off message 
            dinMIDIsendNoteOff(activeNote, velocitySend, activeMIDIchannel - 1);
          }

          if (slurSustain){
            addedIntervals++;
            slurInterval[addedIntervals-1] = fingeredNote - slurBase;
          }
          activeNote=fingeredNote;
        }
      }
    }
  }
  // Is it time to send more CC data?
  if (millis() - ccSendTime > CC_INTERVAL) {
    // deal with Breath, Pitch Bend, Modulation, etc.
    breath();
    pitch_bend();
    portamento_();
    extraController();
    statusLEDs();
    ccSendTime = millis();
  }
  if (millis() - pixelUpdateTime > pixelUpdateInterval){
    // even if we just alter a pixel, the whole display is redrawn (35ms of MPU lockup) and we can't do that all the time
    // this is one of the big reasons the display is for setup use only
    drawSensorPixels(); // live sensor monitoring for the setup screens
    pixelUpdateTime = millis();
  }
  lastFingering=fingeredNote; 
  //do menu stuff
  menu();
}

//_______________________________________________________________________________________________ FUNCTIONS

// non linear mapping function (http://playground.arduino.cc/Main/MultiMap)
// note: the _in array should have increasing values
unsigned int multiMap(unsigned int val, unsigned int* _in, unsigned int* _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

//**************************************************************

// map breath values to selected curve
unsigned int breathCurve(unsigned int inputVal){
  // 0 to 16383, moving mid value up or down
  switch (curve){
    case 0:
      // -2
      return multiMap(inputVal,curveIn,curveM2,17);
      break;
    case 1:
      // -1
      return multiMap(inputVal,curveIn,curveM1,17);
      break;
    case 2:
      // 0, linear
      return inputVal;
      break;
    case 3:
      // +1
      return multiMap(inputVal,curveIn,curveP1,17);
      break;
    case 4:
      // +2
      return multiMap(inputVal,curveIn,curveP2,17);
      break;
  }
}



// MIDI note value check with out of range octave repeat
int noteValueCheck(int note){
  if (note > 127){
    note = 115+(note-127)%12;
  } else if (note < 0) {
    note = 12-abs(note)%12;
  }
  return note;
}

//**************************************************************

void midiPanic(){
  for (int i = 0; i < 128; i++){
    usbMIDI.sendNoteOff(i,0,activeMIDIchannel);
    dinMIDIsendNoteOff(i,0,activeMIDIchannel - 1);
  }
}

//**************************************************************

//  Send a three byte din midi message  
void midiSend3B(byte midistatus, byte data1, byte data2) {
  Serial3.write(midistatus);
  Serial3.write(data1);
  Serial3.write(data2);
}

//**************************************************************

//  Send a two byte din midi message  
void midiSend2B(byte midistatus, byte data) {
  Serial3.write(midistatus);
  Serial3.write(data);
}

//**************************************************************

//  Send din pitchbend  
void dinMIDIsendPitchBend(int pb, byte ch) {
    int pitchLSB = pb & 0x007F;
    int pitchMSB = (pb >>7) & 0x007F; 
    midiSend3B((0xE0 | ch), pitchLSB, pitchMSB);
}

//**************************************************************

//  Send din control change  
void dinMIDIsendControlChange(byte ccNumber, int cc, byte ch) {
    midiSend3B((0xB0 | ch), ccNumber, cc);
}

//**************************************************************

//  Send din note on  
void dinMIDIsendNoteOn(byte note, int vel, byte ch) {
    midiSend3B((0x90 | ch), note, vel);
}

//**************************************************************

//  Send din note off 
void dinMIDIsendNoteOff(byte note, int vel, byte ch) {
    midiSend3B((0x80 | ch), note, vel);
}

//**************************************************************

//  Send din aftertouch 
void dinMIDIsendAfterTouch(byte value, byte ch) {
    midiSend2B((0xD0 | ch), value);
}

//**************************************************************

//  Send din program change 
void dinMIDIsendProgramChange(byte value, byte ch) {
    midiSend2B((0xC0 | ch), value);
}

//**************************************************************

void statusLEDs() {
  if (breathLevel > breathThrVal){ // breath indicator LED, labeled "B" on PCB
    analogWrite(bLedPin, breathLedBrightness);
  } else {
    analogWrite(bLedPin, 0);
  }
  if (biteSensor > portamThrVal){ // portamento indicator LED, labeled "P" on PCB
    analogWrite(pLedPin, portamLedBrightness);
  } else {
    analogWrite(pLedPin, 0);
  }
}

//**************************************************************

void breath(){
  int breathCCval,breathCCvalFine;
  unsigned int breathCCvalHires;
  breathLevel = breathLevel*0.8+pressureSensor*0.2; // smoothing of breathLevel value
  //breathCCval = map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,127);
  breathCCvalHires = breathCurve(map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,16383));
  breathCCval = (breathCCvalHires >>7) & 0x007F;
  breathCCvalFine = breathCCvalHires & 0x007F;

  if (breathCCval != oldbreath){ // only send midi data if breath has changed from previous value
    if (breathCC){
      // send midi cc
      usbMIDI.sendControlChange(ccList[breathCC], breathCCval, activeMIDIchannel);
      dinMIDIsendControlChange(ccList[breathCC], breathCCval, activeMIDIchannel - 1);
    }
    if (breathAT){
      // send aftertouch
      usbMIDI.sendAfterTouch(breathCCval, activeMIDIchannel);
      dinMIDIsendAfterTouch(breathCCval, activeMIDIchannel - 1);
    }
    oldbreath = breathCCval;
  }
  
  if (breathCCvalHires != oldbreathhires){
    if (breathCC > 4){ // send high resolution midi
        usbMIDI.sendControlChange(ccList[breathCC]+32, breathCCvalFine, activeMIDIchannel);
        dinMIDIsendControlChange(ccList[breathCC]+32, breathCCvalFine, activeMIDIchannel - 1);
    }
    oldbreathhires = breathCCvalHires;   
  }
}

//**************************************************************

void pitch_bend(){
  // handle input from pitchbend touchpads and
  // on-pcb variable capacitor for vibrato.
  float nudge;
  int calculatedPBdepth;
  byte vibratoMoved = 0;
  pbUp = touchRead(pbUpPin);                                      // SENSOR PIN 23 - PCB PIN "Pu"
  pbDn = touchRead(pbDnPin);                                      // SENSOR PIN 22 - PCB PIN "Pd"
  halfPitchBendKey=(touchRead(halfPitchBendKeyPin) > touch_Thr);  // SENSOR PIN 1  - PCB PIN "S1" - hold for 1/2 pitchbend value
  int vibRead = touchRead(vibratoPin);                            // SENSOR PIN 15 - built in var cap
  if ((vibRead < vibThr)&&(vibRead > oldvibRead)){
    nudge = 0.01*constrain(abs(vibRead - oldvibRead),0,100);
    if (!dirUp){
      pitchBend=oldpb*(1-nudge)+nudge*(8192 + vibDepth[vibrato]);
      vibratoMoved = 1;
    } else {
      pitchBend=oldpb*(1-nudge)+nudge*(8191 - vibDepth[vibrato]);
      vibratoMoved = 1;
    }
  } else if ((vibRead < vibThr)&&(vibRead < oldvibRead)){
    nudge = 0.01*constrain(abs(vibRead - oldvibRead),0,100);
    if (!dirUp ){
      pitchBend=oldpb*(1-nudge)+nudge*(8191 - vibDepth[vibrato]);
      vibratoMoved = 1;
    } else {
      pitchBend=oldpb*(1-nudge)+nudge*(8192 + vibDepth[vibrato]);
      vibratoMoved = 1;
    }
  } else {
    vibratoMoved = 0;
  }

  oldvibRead = vibRead;
  if (PBdepth){
    calculatedPBdepth = pbDepthList[PBdepth];
    if (halfPitchBendKey) calculatedPBdepth = calculatedPBdepth*0.5;
  }
  if ((pbUp > pitchbThrVal) && PBdepth){
    pitchBend=pitchBend*0.6+0.4*map(constrain(pbUp,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,(8193 + calculatedPBdepth));
  } else if ((pbDn > pitchbThrVal) && PBdepth){
    pitchBend=pitchBend*0.6+0.4*map(constrain(pbDn,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,(8192 - calculatedPBdepth));
  } else if (oldvibRead > vibThr){
    pitchBend = pitchBend*0.6+8192*0.4; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
  } else if (!vibratoMoved){
    pitchBend = oldpb*0.8+8192*0.2; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
  }
  pitchBend=constrain(pitchBend, 0, 16383);
  if (pitchBend != oldpb){// only send midi data if pitch bend has changed from previous value
    usbMIDI.sendPitchBend(pitchBend, activeMIDIchannel);
    dinMIDIsendPitchBend(pitchBend, activeMIDIchannel - 1);
    oldpb=pitchBend;
  }
}

//***********************************************************

void extraController(){
 // Extra Controller is the lip touch sensor (proportional) in front of the mouthpiece
 exSensor=exSensor*0.6+0.4*touchRead(extraPin);     // get sensor data, do some smoothing - SENSOR PIN 16 - PCB PIN "EC" (marked K4 on some prototype boards)
 if (extraCT && (exSensor >= extracThrVal)) {    // if we are enabled and over the threshold, send data
   if (!extracIsOn) {
     extracIsOn=1;
     if (extraCT == 3){ //Sustain ON
      usbMIDI.sendControlChange(64,127, activeMIDIchannel);
      dinMIDIsendControlChange(64,127, activeMIDIchannel - 1); 
     } 
    }
    if (extraCT == 1){ //Send modulation
      int extracCC = map(constrain(exSensor,extracThrVal,extracMaxVal),extracThrVal,extracMaxVal,1,127); 
      if (extracCC != oldextrac){
        usbMIDI.sendControlChange(1,extracCC, activeMIDIchannel);
        dinMIDIsendControlChange(1,extracCC, activeMIDIchannel - 1);      
      }
      oldextrac = extracCC; 
    }
    if (extraCT == 2){ //Send foot pedal (CC#4)
      int extracCC = map(constrain(exSensor,extracThrVal,extracMaxVal),extracThrVal,extracMaxVal,1,127); 
      if (extracCC != oldextrac){
        usbMIDI.sendControlChange(4,extracCC, activeMIDIchannel);
        dinMIDIsendControlChange(4,extracCC, activeMIDIchannel - 1);      
      }
      oldextrac = extracCC; 
    }
  } else if (extracIsOn) {                        // we have just gone below threshold, so send zero value
    extracIsOn=0;
    if (extraCT == 1){ //MW
      //send modulation 0
      usbMIDI.sendControlChange(1,0, activeMIDIchannel);
      dinMIDIsendControlChange(1,0, activeMIDIchannel - 1);
      oldextrac = 0;
    } else if (extraCT == 2){ //FP
      //send foot pedal 0
      usbMIDI.sendControlChange(4,0, activeMIDIchannel);
      dinMIDIsendControlChange(4,0, activeMIDIchannel - 1);
      oldextrac = 0;
    } else if (extraCT == 3){ //SP
      //send sustain off
      usbMIDI.sendControlChange(64,0, activeMIDIchannel);
      dinMIDIsendControlChange(64,0, activeMIDIchannel - 1); 
    } 
  }
}

//***********************************************************

void portamento_(){
 // Portamento is controlled with the bite sensor (variable capacitor) in the mouthpiece
 biteSensor=biteSensor*0.6+0.4*touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right)
 if (portamento && (biteSensor >= portamThrVal)) {    // if we are enabled and over the threshold, send portamento
   if (!portIsOn) {
     portOn();
    }
    port();        
  } else if (portIsOn) {                        // we have just gone below threshold, so send zero value
    portOff(); 
  }
}

//***********************************************************

void portOn(){
  if (portamento == 2){ // if portamento midi switching is enabled
    usbMIDI.sendControlChange(CCN_PortOnOff, 127, activeMIDIchannel);
    dinMIDIsendControlChange(CCN_PortOnOff, 127, activeMIDIchannel - 1);
  }
  portIsOn=1;
}

//***********************************************************

void port(){
  int portCC;
  portCC = map(constrain(biteSensor,portamThrVal,portamMaxVal),portamThrVal,portamMaxVal,0,127);
  if (portCC!=oldport){
    usbMIDI.sendControlChange(CCN_Port, portCC, activeMIDIchannel);
    dinMIDIsendControlChange(CCN_Port, portCC, activeMIDIchannel - 1);
  }
  oldport = portCC;
}

//***********************************************************

void portOff(){
  usbMIDI.sendControlChange(CCN_Port, 0, activeMIDIchannel);
  dinMIDIsendControlChange(CCN_Port, 0, activeMIDIchannel - 1);
  if (portamento == 2){ // if portamento midi switching is enabled
    usbMIDI.sendControlChange(CCN_PortOnOff, 0, activeMIDIchannel);
    dinMIDIsendControlChange(CCN_PortOnOff, 0, activeMIDIchannel - 1);
  }
  portIsOn=0;
  oldport = 0;
}

//***********************************************************

void readSwitches(){  
  
  // Read touch pads (MPR121) and put value in variables
  int touchValue[12]; 
  for (byte i=0; i<12; i++){
    touchValue[i]=touchSensor.filteredData(i);
  }

  // Octave rollers
  octaveR = 0;
  if      ((touchValue[R5Pin] < ctouchThrVal) && (touchValue[R3Pin] < ctouchThrVal)) octaveR = 6; //R6 = R5 && R3
  else if (touchValue[R5Pin] < ctouchThrVal) octaveR = 5;  //R5
  else if (touchValue[R4Pin] < ctouchThrVal) octaveR = 4;  //R4
  else if (touchValue[R3Pin] < ctouchThrVal) octaveR = 3;  //R3
  else if (touchValue[R2Pin] < ctouchThrVal) octaveR = 2;  //R2
  else if (touchValue[R1Pin] < ctouchThrVal) octaveR = 1;  //R1
  
  // Valves and trill keys
  K4=(touchValue[K4Pin] < ctouchThrVal);
  K1=(touchValue[K1Pin] < ctouchThrVal);
  K2=(touchValue[K2Pin] < ctouchThrVal);
  K3=(touchValue[K3Pin] < ctouchThrVal);
  K5=(touchValue[K5Pin] < ctouchThrVal);
  K6=(touchValue[K6Pin] < ctouchThrVal);
  K7=(touchValue[K7Pin] < ctouchThrVal);
  
  // Calculate midi note number from pressed keys  
  fingeredNote=startNote-2*K1-K2-3*K3-5*K4+2*K5+K6+4*K7+octaveR*12+(octave-3)*12+transpose-12;
}










// MENU STUFF FROM THIS POINT <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<











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
  }


  
  if        (state == DISPLAYOFF_IDL){
    if (stateFirstRun) {
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      stateFirstRun = 0;
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = PATCH_VIEW;
          buttonPressedAndNotUsed = 1;
          stateFirstRun = 1;
          break;
        case 2:
          // enter
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = PATCH_VIEW;
          stateFirstRun = 1;
          break;
        case 4:
          // up
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = PATCH_VIEW;
          buttonPressedAndNotUsed = 1;
          stateFirstRun = 1;
          break;
        case 8:
          // menu
          display.ssd1306_command(SSD1306_DISPLAYON);
          state = MAIN_MENU;
          stateFirstRun = 1;
          break;
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
    }
    if (buttonPressedAndNotUsed){
      buttonPressedAndNotUsed = 0;
      switch (deumButtonState){
        case 1:
          // down
          if (patch > 1){
            patch--;
          } else patch = 128;
          drawPatchView();
          patchViewTime = millis();
          break;
        case 2:
          // enter
          patchViewTime = millis();
          midiPanic();
          break;
        case 4:
          // up
          if (patch < 128){
            patch++;
          } else patch = 1;
          drawPatchView();
          patchViewTime = millis();
          break;
        case 8:
          // menu
          state = DISPLAYOFF_IDL;
          stateFirstRun = 1;
          doPatchUpdate = 1;
          break;
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
            plotMIDI(WHITE);
            cursorNow = BLACK;
            display.display();
            subMIDI = 0;
            if (readSetting(MIDI_ADDR) != MIDIchannel) writeSetting(MIDI_ADDR,MIDIchannel);
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
        }
      }
    }  
  } else if (state == BREATH_ADJ_IDL){
    if (stateFirstRun) {
      drawBreathScreen();
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
              breathCC = 8;
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
            if (readSetting(BREATH_CC_ADDR) != breathCC) writeSetting(BREATH_CC_ADDR,breathCC);
            break;
          case 4:
            // up
            if (breathCC < 8){
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
            if (readSetting(BREATH_CC_ADDR) != breathCC) writeSetting(BREATH_CC_ADDR,breathCC);
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
            if (readSetting(MIDI_ADDR) != MIDIchannel) writeSetting(MIDI_ADDR,MIDIchannel);
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
            if (readSetting(MIDI_ADDR) != MIDIchannel) writeSetting(MIDI_ADDR,MIDIchannel);
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
            } else curve = 4;
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
            if (curve < 4){
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
            if (setupBrMenuCursor < 4){
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
            } else extraCT = 3;
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
            if (extraCT < 3){
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
    } else if (subVibrato) {
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
            if (vibrato < 6){
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
              deglitch-=5;
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
              deglitch+=5;
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
            if (setupCtMenuCursor < 5){
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
      subVibrato = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVibrato();
      break;
    case 5:
      subDeglitch = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubDeglitch();
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
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("MENU");
  display.drawLine(0,9,127,9,WHITE);
  display.setCursor(0,12);
  display.println("TRANSPOSE"); 
  display.setCursor(0,21);
  display.println("OCTAVE");
  display.setCursor(0,30);
  display.println("MIDI CH"); 
  display.setCursor(0,39);
  display.println("ADJUST"); 
  display.setCursor(0,48);
  display.println("SETUP BR"); 
  display.setCursor(0,57);
  display.println("SETUP CTL"); 
  drawMenuCursor(mainMenuCursor, WHITE);
  display.display();
}

void drawPatchView(){
  display.clearDisplay();
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
  display.display();
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

void drawSubOctave(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(74,15);
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
  display.setCursor(68,15);
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
      display.println("-2");
      break;
    case 1:
      display.setCursor(83,33);
      display.println("-1");
      break;
    case 2:
      display.setCursor(79,33);
      display.println("LIN");
      break;
    case 3:
      display.setCursor(83,33);
      display.println("+1");
      break;
    case 4:
      display.setCursor(83,33);
      display.println("+2");
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
    display.println("SP");
    break;
  }
}

void drawSubVibrato(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(74,15);
  display.println("VIBRATO");
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

void drawSetupBrMenuScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("SETUP BREATH");
  display.drawLine(0,9,127,9,WHITE);
  display.setCursor(0,12);
  display.println("BREATH CC"); 
  display.setCursor(0,21);
  display.println("BREATH AT");
  display.setCursor(0,30);
  display.println("VELOCITY"); 
  display.setCursor(0,39);
  display.println("CURVE"); 
  display.setCursor(0,48);
  display.println(""); 
  display.setCursor(0,57);
  display.println(""); 

  display.display();
}

void drawSetupCtMenuScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("SETUP CTRLS");
  display.drawLine(0,9,127,9,WHITE);
  display.setCursor(0,12);
  display.println("PORT/GLD"); 
  display.setCursor(0,21);
  display.println("PITCHBEND");
  display.setCursor(0,30);
  display.println("EXTRA CTR"); 
  display.setCursor(0,39);
  display.println("VIBRATO"); 
  display.setCursor(0,48);
  display.println("DEGLITCH"); 
  display.setCursor(0,57);
  display.println(""); 

  display.display();
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
    }
    pos = map(constrain(pbDn, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    oldpos = map(constrain(lastPbDn, pitchbLoLimit, pitchbHiLimit), pitchbLoLimit, pitchbHiLimit, 28, 118);
    if (pos!=oldpos){
      display.drawPixel(oldpos, 38, BLACK);
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
    }
    lastEx=exSensor;
  }
  if ((state == CTOUCH_ADJ_IDL) || (state == CTOUCH_ADJ_THR)){
    display.drawLine(28,38,118,38,BLACK);
    for (byte i=0; i<12; i++){
      pos = map(constrain(touchSensor.filteredData(i), ctouchLoLimit, ctouchHiLimit), ctouchLoLimit, ctouchHiLimit, 28, 118);
      display.drawPixel(pos, 38, WHITE);
    }
    display.display();
  }
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


