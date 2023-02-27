#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <SPI.h>
#include <EEPROM.h>

#include "FilterOnePole.h"  // for the breath signal low-pass filtering, from https://github.com/JonHub/Filters
#include "globals.h"
#include "hardware.h"
#include "midi.h"
#include "menu.h"
#include "config.h"
#include "settings.h"
#include "led.h"

/*
NAME:                 NuEVI
WRITTEN BY:           JOHAN BERGLUND
DATE:                 2018-04-19
FILE SAVED AS:        NuEVI.ino
FOR:                  PJRC Teensy 3.2 and a MPR121 capactive touch sensor board.
                      Uses an SSD1306 controlled OLED display communicating over I2C.
PROGRAMME FUNCTION:   EVI Wind Controller using the Freescale MP3V5004GP breath sensor
                      and capacitive touch keys. Output to both USB MIDI and DIN MIDI.

*/

/*
but hey, it's also...

NAME:                 NuRAD
WRITTEN BY:           JOHAN BERGLUND
DATE:                 2019-08-09
FILE SAVED AS:        n/a
FOR:                  PJRC Teensy 3.2 and three MPR121 capactive touch sensor boards.
                      Uses an SSD1306 controlled OLED display communicating over I2C.
PROGRAMME FUNCTION:   EWI Wind Controller using the Freescale MP3V5004GP breath sensor
                      and capacitive touch keys. Output to both USB MIDI and DIN MIDI.

...if you just uncomment the #define NURAD in hardware.h
*/


//Make sure compiler is set to the appropriate platform
#ifndef __MK20DX256__
  #error "Wrong target platform. Please set to Teensy 3.1/3.2 (MK20DX256)."
#endif

#if !defined(USB_MIDI) && !defined(USB_MIDI_SERIAL)
  #error "USB MIDI not enabled. Please set USB type to 'MIDI' or 'Serial + MIDI'."
#endif



//_______________________________________________________________________________________________ DECLARATIONS

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
unsigned short leverThrVal;
unsigned short leverMaxVal;
unsigned short transpose;
unsigned short MIDIchannel;
unsigned short breathCC;  // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+:CF
unsigned short breathCC2;  // OFF:1-127
unsigned short breathCC2Rise;  // 1X:2X:3X:4X:5X
unsigned short breathAT;
unsigned short velocity;
unsigned short portamento;// switching on cc65? just cc5 enabled? SW:ON:OFF
unsigned short portLimit; // 1-127
unsigned short PBdepth;   // OFF:1-12 divider
unsigned short extraCT;   // OFF:MW:FP:CF:SP
unsigned short vibrato;   // OFF:1-9
unsigned short deglitch;  // 0-70 ms in steps of 5
unsigned short patch;     // 1-128
unsigned short octave;
unsigned short curve;
unsigned short velSmpDl;  // 0-30 ms
unsigned short velBias;   // 0-9
unsigned short pinkySetting; // 0 - 11 (QuickTranspose -12 to -1), 12 (pb/2), 13 - 24 (QuickTranspose +1 to +12), 25 (EC2), 26 (ECSW), 27 (LVL), 28 (LVLP)
unsigned short dipSwBits; // virtual dip switch settings for special modes (work in progress)
unsigned short priority; // mono priority for rotator chords

unsigned short extraCT2; // OFF:1-127
unsigned short levelCC; // 0-127
unsigned short levelVal; // 0-127
unsigned short fingering; // 0-4 EWI,EWX,SAX,EVI,EVR
unsigned short rollerMode; //0-2
unsigned short lpinky3; // 0-25 (OFF, -12 - MOD - +12)
unsigned short batteryType; // 0-2 ALK,NIM,LIP
unsigned short harmSetting; // 0-7
unsigned short harmSelect; // 0-5
unsigned short brHarmSetting; // 0-7
unsigned short brHarmSelect; // 0-3
PolySelect polySelect; // OFF, MGR, MGD, MND, MNH, FWC, RTA, RTB or RTC
unsigned short fwcType; // 6, m6, 7, m7 
unsigned short fwcLockH; // OFF:ON
unsigned short fwcDrop2; // OFF:ON
unsigned short hmzKey; // 0-11 (0 is C)
unsigned short hmzLimit; // 2-5
unsigned short otfKey; //OFF:ON
unsigned short breathInterval = 6; // 3-15

unsigned short vibSens = 2; // vibrato sensitivity
unsigned short vibRetn = 2; // vibrato return speed
unsigned short vibSquelch = 12; //vibrato signal squelch
unsigned short vibDirection = DNWD; //direction of first vibrato wave UPWD or DNWD
unsigned short vibSensBite = 2; // vibrato sensitivity (bite)
unsigned short vibSquelchBite = 12; //vibrato signal squelch (bite)
unsigned short vibControl = 0;
unsigned short biteControl = 0; // OFF, VIB, GLD, CC
unsigned short leverControl = 0; // OFF, VIB, GLD, CC
unsigned short biteCC = 0; // 0 - 127
unsigned short leverCC = 0; // 0 -127

unsigned short cvTune;  // 1 - 199 representing -99 to +99 in menu (offset of 100 to keep postitive)
unsigned short cvScale; // 1 - 199 representing -99 to +99 in menu (offset of 100 to keep postitive)
unsigned short cvVibRate; // OFF, 1 - 8 CV extra controller LFO vibrato rate 4.5Hz to 8Hz

unsigned short fastPatch[7] = {0,0,0,0,0,0,0};

uint16_t bcasMode; //Legacy CASSIDY compile flag
uint16_t trill3_interval;
uint16_t fastBoot;
uint16_t dacMode;

byte rotatorOn = 0;
byte currentRotation = 3;

Rotator rotations_a;
Rotator rotations_b;
Rotator rotations_c;

byte gateOpen = 0; // setting for gate always open, note on sent for every time fingering changes, no matter the breath status
uint16_t gateOpenEnable = 0;

uint16_t specialKeyEnable = 0;

int touch_Thr = 1300;

byte ccList[11] = {0,1,2,7,11,1,2,7,11,74,20};  // OFF, Modulation, Breath, Volume, Expression (then same sent in hires), CC74 (cutoff/brightness), CC20 (UNO Cutoff)

int pbDepthList[13] = {8192,8192,4096,2731,2048,1638,1365,1170,1024,910,819,744,683};

#if defined(NURAD)
#if defined(SEAMUS)
int calOffsetRollers[6] = {-70,20,20,20,20,120};
int calOffsetRH[12] = {0,0,0,0,0,-50,121,0,0,50,0,120};
int calOffsetLH[12] = {120,0,120,0,50,115,118,0,50,0,0,0};
#else
int calOffsetRollers[6] = {16,10,8,21,24,41};
int calOffsetRH[12] = {-88,-68,-31,13,4,120,121,-68,-85,-34,23,87};
int calOffsetLH[12] = {90,-13,-33,-93,-82,115,118,2,4,-40,-75,-94};
#endif
#endif

int battMeasured[50];
int battAvg = 0;
byte battCheckPos = 0;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

static unsigned long pixelUpdateTime = 0;
static const unsigned long pixelUpdateInterval = 80;

unsigned long lastDeglitchTime = 0;         // The last time the fingering was changed
unsigned long ccSendTime = 0L;              // The last time we sent CC values
unsigned long ccSendTime2 = 0L;             // The last time we sent CC values 2 (slower)
unsigned long ccSendTime3 = 0L;             // The last time we sent CC values 3 (and slower)
unsigned long lvlTime = 0L;
unsigned long ccBreathSendTime = 0L;        // The last time we sent breath CC values
unsigned long breath_on_time = 0L;          // Time when breath sensor value went over the ON threshold
unsigned long currentTime;

int lastFingering = 0;             // Keep the last fingering value for debouncing

int mainState;                         // The state of the main state machine

int initial_breath_value;          // The breath value at the time we observed the transition

byte activeMIDIchannel;          // MIDI channel
byte activePatch=0;
byte doPatchUpdate=0;

byte cvPortaTuneCount = 0;

uint16_t legacy = 0;
uint16_t legacyBrAct = 0;
byte halfTime = 0;
boolean programonce = false;
boolean oneroll;
byte widiOn = 0;

int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;
int oldbreathcc2=0;
unsigned int oldbreathhires=0;
float filterFreq = 30.0;

float filterVal = 0.15;
float smoothedVal;
int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
int lastPressure;
byte velocitySend;   // remapped midi velocity from breath sensor (or set to static value if selected)
int breathCalZero;

int leverPortZero;
#if defined(NURAD)
int leverPortThr = 70;
#else
int leverPortThr = 70;
#endif
int leverPortRead;


int biteSensor=0;    // capacitance data from bite sensor, for midi cc and threshold checks
byte portIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
byte biteIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
byte leverIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
int oldport=0;
int lastBite=0;
byte biteJumper=0;
byte widiJumper=0;
int oldbitecc=0;
int oldlevercc=0;

int cvPitch;
int targetPitch;

int exSensor=0;
int exSensorIndicator=0;
byte extracIsOn=0;
int oldextrac=0;
int oldextrac2=0;

int harmonics = 0;
int brHarmonics = 0;

int pitchBend=8192;
int pbSend=8192;
int oldpb=8192;
int vibSignal=0;
int pbUp=0;
int pbDn=0;
byte vibLedOff = 0;
byte oldpkey = 0;

byte lap = 0;
byte rSum = 0;

static const float vibDepth[10] = {0,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.40,0.45}; // max pitch bend values (+/-) for the vibrato settings
static const short vibMaxBiteList[17] = {1600,1400,1200,1000,900,800,700,600,500,400,300,250,200,150,100,50,25};
static const short vibMaxList[12] = {300,275,250,225,200,175,150,125,100,75,50,25};
static const int timeDividerList[9] = {0, 222, 200, 181, 167, 152, 143, 130, 125}; // For CV vibrato - 222 is 4.5Hz, 200 is 5Hz, 181 is 5.5Hz 167 is 6Hz, 152 is 6.5Hz, 143 is 7Hz, 130 is 7.5Hz, 125 is 8Hz

static const unsigned short curveM4[] = {0,4300,7000,8700,9900,10950,11900,12600,13300,13900,14500,15000,15450,15700,16000,16250,16383};
static const unsigned short curveM3[] = {0,2900,5100,6650,8200,9500,10550,11500,12300,13100,13800,14450,14950,15350,15750,16150,16383};
static const unsigned short curveM2[] = {0,2000,3600,5000,6450,7850,9000,10100,11100,12100,12900,13700,14400,14950,15500,16000,16383};
static const unsigned short curveM1[] = {0,1400,2850,4100,5300,6450,7600,8700,9800,10750,11650,12600,13350,14150,14950,15650,16383};
const unsigned short curveIn[] = {0,1023,2047,3071,4095,5119,6143,7167,8191,9215,10239,11263,12287,13311,14335,15359,16383};
static const unsigned short curveP1[] = {0,600,1350,2150,2900,3800,4700,5600,6650,7700,8800,9900,11100,12300,13500,14850,16383};
static const unsigned short curveP2[] = {0,400,800,1300,2000,2650,3500,4300,5300,6250,7400,8500,9600,11050,12400,14100,16383};
static const unsigned short curveP3[] = {0,200,500,900,1300,1800,2350,3100,3800,4600,5550,6550,8000,9500,11250,13400,16383};
static const unsigned short curveP4[] = {0,100,200,400,700,1050,1500,1950,2550,3200,4000,4900,6050,7500,9300,12100,16383};
static const unsigned short curveS1[] = {0,600,1350,2150,2900,3800,4700,6000,8700,11000,12400,13400,14300,14950,15500,16000,16383};
static const unsigned short curveS2[] = {0,600,1350,2150,2900,4000,6100,9000,11000,12100,12900,13700,14400,14950,15500,16000,16383};
//static const unsigned short curveS3[] = {0,600,1350,2300,3800,6200,8700,10200,11100,12100,12900,13700,14400,14950,15500,16000,16383};
//static const unsigned short curveS4[] = {0,600,1700,4000,6600,8550,9700,10550,11400,12200,12900,13700,14400,14950,15500,16000,16383};

static const unsigned short curveZ1[] = {0,1400,2100,2900,3200,3900,4700,5600,6650,7700,8800,9900,11100,12300,13500,14850,16383};
static const unsigned short curveZ2[] = {0,2000,3200,3800,4096,4800,5100,5900,6650,7700,8800,9900,11100,12300,13500,14850,16383};

const unsigned short* const curves[] = {
   curveM4, curveM3, curveM2, curveM1, curveIn, curveP1, curveP2,
   curveP3, curveP4 , curveS1, curveS2, curveZ1, curveZ2
};

// NuRAD Sax fingering
// LH1, LHb, LH2, LH3, LHp1, -LHp2-, -RHs-, RH1, RH2, RH3, RHp1, -RHp2-, RHp3  -excluded- LHp2 always -1, RHs always +1, RHp2 disabled
// 0 = not touched, 1 = touched, 2 = whatever

const byte saxFingerMatch[16][10] =
{
  {1, 2, 1, 1, 0, 1, 1, 1, 2, 1}, // C (-13 semis)
  {1, 2, 1, 1, 1, 1, 1, 1, 2, 1}, // C# (-12 semis)
  {1, 2, 1, 1, 2, 1, 1, 1, 0, 0}, // D (-11 semis)
  {1, 2, 1, 1, 2, 1, 1, 1, 1, 0}, // D# (-10 semis)
  {1, 2, 1, 1, 2, 1, 1, 0, 2, 2}, // E (-9 semis)
  {1, 2, 1, 1, 2, 1, 0, 2, 2, 2}, // F (-8 semis)
  {1, 2, 1, 1, 2, 0, 1, 2, 2, 2}, // F# (-7 semis)
  {1, 2, 1, 1, 0, 0, 0, 2, 2, 2}, // G (-6 semis)
  {1, 2, 1, 1, 1, 0, 0, 2, 2, 2}, // G# (-5 semis)
  {1, 2, 1, 0, 2, 2, 2, 2, 2, 2}, // A (-4 semis)
  {1, 2, 0, 2, 2, 1, 2, 2, 2, 2}, // A# (-3 semis) RH1 for Bb
  {1, 2, 0, 2, 2, 2, 1, 2, 2, 2}, // A# (-3 semis) RH2 for Bb
  {1, 1, 0, 2, 2, 2, 2, 2, 2, 2}, // A# (-3 semis) bis for Bb
  {1, 0, 0, 2, 2, 0, 2, 2, 2, 2}, // B (-2 semis)
  {0, 2, 1, 2, 2, 2, 2, 2, 2, 2}, // C (-1 semis)
  {0, 2, 0, 2, 2, 2, 2, 2, 2, 2}, // C# (-0 semis)
};

static int waveformsTable[maxSamplesNum] = {
  // Sine wave
  0x7ff, 0x86a, 0x8d5, 0x93f, 0x9a9, 0xa11, 0xa78, 0xadd, 0xb40, 0xba1,
  0xbff, 0xc5a, 0xcb2, 0xd08, 0xd59, 0xda7, 0xdf1, 0xe36, 0xe77, 0xeb4,
  0xeec, 0xf1f, 0xf4d, 0xf77, 0xf9a, 0xfb9, 0xfd2, 0xfe5, 0xff3, 0xffc,
  0xfff, 0xffc, 0xff3, 0xfe5, 0xfd2, 0xfb9, 0xf9a, 0xf77, 0xf4d, 0xf1f,
  0xeec, 0xeb4, 0xe77, 0xe36, 0xdf1, 0xda7, 0xd59, 0xd08, 0xcb2, 0xc5a,
  0xbff, 0xba1, 0xb40, 0xadd, 0xa78, 0xa11, 0x9a9, 0x93f, 0x8d5, 0x86a,
  0x7ff, 0x794, 0x729, 0x6bf, 0x655, 0x5ed, 0x586, 0x521, 0x4be, 0x45d,
  0x3ff, 0x3a4, 0x34c, 0x2f6, 0x2a5, 0x257, 0x20d, 0x1c8, 0x187, 0x14a,
  0x112, 0xdf, 0xb1, 0x87, 0x64, 0x45, 0x2c, 0x19, 0xb, 0x2,
  0x0, 0x2, 0xb, 0x19, 0x2c, 0x45, 0x64, 0x87, 0xb1, 0xdf,
  0x112, 0x14a, 0x187, 0x1c8, 0x20d, 0x257, 0x2a5, 0x2f6, 0x34c, 0x3a4,
  0x3ff, 0x45d, 0x4be, 0x521, 0x586, 0x5ed, 0x655, 0x6bf, 0x729, 0x794
};

int saxFingerResult[16] =
{-13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -3, -3, -2, -1, 0};

byte saxFinger[10];

byte bottom = 1;

// Four Way Close variants (FWC), http://www.thejazzpianosite.com/jazz-piano-lessons/jazz-chord-voicings/four-way-close/
// 6
const int blockFWC[4][12][3] =  {{{ -3, -5, -8 },   // C or key base
                                  { -3, -5, -8 },   // C# or base +1
                                  { -3, -6, -9 },   // D or base +2
                                  { -3, -5, -9 },   // D# or base +3
                                  { -4, -5, -9 },   // E or base +4
                                  { -3, -6, -9 },   // F or base +5
                                  { -3, -5, -8 },   // F# or base +6
                                  { -3, -5, -8 },   // G or base +7
                                  { -3, -6, -9 },   // G# or base +8
                                  { -4, -5, -9 },   // A or base +9
                                  { -3, -5, -9 },   // Bb or base +10
                                  { -3, -6, -9 }},  // B or base +11
// m6
                                 {{ -3, -5, -9 },   // C or key base
                                  { -3, -5, -9 },   // C# or base +1
                                  { -3, -6, -9 },   // D or base +2
                                  { -3, -6, -8 },   // D# or base +3
                                  { -3, -6, -9 },   // E or base +4
                                  { -3, -6, -9 },   // F or base +5
                                  { -3, -6, -8 },   // F# or base +6
                                  { -3, -6, -8 },   // G or base +7
                                  { -3, -6, -9 },   // G# or base +8
                                  { -4, -5, -9 },   // A or base +9
                                  { -3, -6, -9 },   // Bb or base +10
                                  { -3, -6, -9 }},  // B or base +11
// 7
                                 {{ -2, -5, -8 },   // C or key base
                                  { -2, -5, -8 },   // C# or base +1
                                  { -3, -6, -9 },   // D or base +2
                                  { -4, -6, -9 },   // D# or base +3
                                  { -4, -6, -9 },   // E or base +4
                                  { -3, -6, -9 },   // F or base +5
                                  { -3, -7, -9 },   // F# or base +6
                                  { -3, -7, -9 },   // G or base +7
                                  { -3, -6, -9 },   // G# or base +8
                                  { -3, -6, -9 },   // A or base +9
                                  { -3, -6, -10 },   // Bb or base +10
                                  { -3, -6, -9 }},  // B or base +11
// m7
                                 {{ -2, -5, -9 },   // C or key base
                                  { -2, -5, -9 },   // C# or base +1
                                  { -3, -6, -9 },   // D or base +2
                                  { -3, -5, -8 },   // D# or base +3
                                  { -3, -5, -8 },   // E or base +4   3 6 9
                                  { -3, -6, -9 },   // F or base +5
                                  { -4, -7, -9 },   // F# or base +6
                                  { -4, -7, -9 },   // G or base +7
                                  { -3, -6, -9 },   // G# or base +8
                                  { -3, -6, -9 },   // A or base +9
                                  { -3, -7, -10 },   // Bb or base +10
                                  { -3, -6, -9 }}};  // B or base +11

// Major Gospel Root (MGR), Bert Lochs
const int majGosRootHmz[12][3] = {{ -5, -8, -12 },   // C or key base
                                  { -6, -9, -12 },   // C# or base +1
                                  { -3, -7, -12 },   // D or base +2
                                  { -6, -9, -12 },   // D# or base +3
                                  { -4, -9, -12 },   // E or base +4
                                  { -5, -8, -12 },   // F or base +5
                                  { -3, -6, -12 },   // F# or base +6
                                  { -3, -7, -12 },   // G or base +7
                                  { -3, -8, -12 },   // G# or base +8
                                  { -4, -9, -12 },   // A or base +9
                                  { -5, -8, -12 },   // Bb or base +10
                                  { -4, -9, -12 }};  // B or base +11

// Major Gospel Dominant (MGD), Bert Lochs
const int majGosDomHmz[12][3] =  {{ -5, -8, -12 },   // C or key base
                                  { -6, -9, -12 },   // C# or base +1
                                  { -3, -7, -12 },   // D or base +2
                                  { -6, -9, -12 },   // D# or base +3
                                  { -4, -9, -12 },   // E or base +4
                                  { -5, -8, -12 },   // F or base +5
                                  { -3, -6, -12 },   // F# or base +6
                                  { -5, -8, -12 },   // G or base +7
                                  { -3, -8, -12 },   // G# or base +8
                                  { -4, -9, -12 },   // A or base +9
                                  { -5, -8, -12 },   // Bb or base +10
                                  { -4, -9, -12 }};  // B or base +11

// Major add9 (MA9), Bert Lochs
const int majAdd9Hmz[12][3] =    {{ -5, -8, -10 },   // C or key base
                                  { -6, -8, -10 },   // C# or base +1
                                  { -3, -5, -7 },   // D or base +2
                                  { -3, -7, -9 },   // D# or base +3
                                  { -2, -4, -9 },   // E or base +4
                                  { -5, -8, -10 },   // F or base +5
                                  { -4, -6, -10 },   // F# or base +6
                                  { -3, -5, -7 },   // G or base +7
                                  { -4, -6, -10 },   // G# or base +8
                                  { -2, -4, -9 },   // A or base +9
                                  { -5, -8, -10 },   // Bb or base +10
                                  { -2, -4, -9 }};  // B or base +11

// Minor Dorian (MND), Bert Lochs
const int minDorHmz[12][3] =     {{ -5, -9, -12 },   // C or key base
                                  { -5, -9, -12 },   // C# or base +1
                                  { -5, -9, -12 },   // D or base +2
                                  { -3, -8, -12 },   // D# or base +3
                                  { -3, -8, -12 },   // E or base +4
                                  { -3, -8, -12 },   // F or base +5
                                  { -5, -9, -12 },   // F# or base +6
                                  { -5, -9, -12 },   // G or base +7
                                  { -5, -8, -12 },   // G# or base +8
                                  { -4, -7, -12 },   // A or base +9
                                  { -3, -8, -12 },   // Bb or base +10
                                  { -4, -9, -12 }};  // B or base +11

// Minor Aeolian (MNA), Bert Lochs
const int minAeoHmz[12][3] =     {{ -5, -9, -12 },   // C or key base
                                  { -5, -9, -12 },   // C# or base +1
                                  { -3, -9, -12 },   // D or base +2
                                  { -3, -8, -12 },   // D# or base +3
                                  { -6, -9, -12 },   // E or base +4
                                  { -5, -9, -12 },   // F or base +5
                                  { -5, -9, -12 },   // F# or base +6
                                  { -5, -9, -12 },   // G or base +7
                                  { -3, -8, -12 },   // G# or base +8
                                  { -3, -8, -12 },   // A or base +9
                                  { -3, -8, -12 },   // Bb or base +10
                                  { -6, -8, -12 }};  // B or base +11

// Minor 4-voice Hip (MNH), Bert Lochs
const int minHipHmz[12][3] =     {{ -5, -9, -10 },   // C or key base
                                  { -5, -9, -10 },   // C# or base +1
                                  { -5, -9, -10 },   // D or base +2
                                  { -3, -4, -8 },   // D# or base +3
                                  { -3, -4, -8 },   // E or base +4
                                  { -3, -4, -8 },   // F or base +5
                                  { -5, -9, -10 },   // F# or base +6
                                  { -5, -9, -10 },   // G or base +7
                                  { -5, -6, -8 },   // G# or base +8
                                  { -4, -7, -8 },   // A or base +9
                                  { -3, -4, -8 },   // Bb or base +10
                                  { -4, -6, -9 }};  // B or base +11


const int harmonicResult[8][7] = {{  0,   7,  12,  16,  19,  24,  28 },  //HM1
                                  {  0,   7,  12,  16,  19,  22,  24 },  //HM2 (7th harmonic not excluded)
                                  {  0,   7,  12,  19,  24,  31,  36 },  //5TH
                                  {  0,  12,  24,  36,  48,  60,  72 },  //OCT
                                  {  0,   7,  12,  16,  19,  24,  28 },  //HM1
                                  {  0,   7,  12,  16,  19,  22,  24 },  //HM2 (7th harmonic not excluded)
                                  {  0,   7,  12,  19,  24,  31,  36 },  //5TH
                                  {  0,  12,  24,  36,  48,  60,  72 }};  //OCT

                                  /*
                                  { 28,  24,  19,  16,  12,   7,   0 },  //H1R
                                  { 24,  22,  19,  16,  12,   7,   0 },  //H2R (7th harmonic not excluded)
                                  { 36,  31,  24,  19,  12,   7,   0 },  //5TR
                                  { 72,  60,  48,  36,  24,  12,   0 }}; //OCR
                                  */


const int brHarmonicResult[4][7] = {{ 0,   7,  12,  16,  19,  24,  28 },  //HM1
                                    { 0,   7,  12,  16,  19,  22,  24 },  //HM2 (7th harmonic not excluded)
                                    { 0,   7,  12,  19,  24,  31,  36 },  //5TH
                                    { 0,  12,  24,  36,  48,  60,  72 }}; //OCT

const int rollerHarmonic[2][7] = {{ 0,   7,  12,  16,  19,  24,  26 },  //F horn 2,3,4,5,6,8,9 hrm
                                  { 7,  12,  16,  19,  24,  26,  31 }}; //Bb horn 3,4,5,6,8,9,12 hrm

const int trumpetHarmonic[2][7] = {{ 0,   7,  12,  16,  19,  26,  31 },  //!K4: hrm 8->9, 10->12
                                   { 0,   7,  12,  16,  19,  24,  28 }}; //trumpet 2,3,4,5,6,8,10 hrm


int vibThr;          // this gets auto calibrated in setup
int vibThrLo;
int vibZero;
int vibZeroBite;
int vibThrBite;
int vibThrBiteLo;


int fingeredNote;    // note calculated from fingering (switches), transpose and octave settings
int fingeredNoteUntransposed; // note calculated from fingering (switches), for on the fly settings
byte activeNote;     // note playing
byte startNote=36;   // set startNote to C (change this value in steps of 12 to start in other octaves)
int slurBase;        // first note in slur sustain chord

int slurInterval[9] = {-5,0,0,0,0,0,0,0,0};
byte addedIntervals = 1;

#if defined(NURAD)
            // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
byte LHs;
byte LH1;   // Left Hand key 1 (pitch change -2)
byte LHb;   // Left Hand bis key (pitch change -1 unless both LH1 and LH2 are pressed)
byte LH2;   // Left Hand key 2  (with LH1 also pressed pitch change is -2, otherwise -1)
byte LH3;   // Left Hand key 3 (pitch change -2)
byte LHp1;  // Left Hand pinky key 1 (pitch change +1)
byte LHp2;  // Left Hand pinky key 2 (pitch change -1)
byte LHp3;
byte RHs;   // Right Hand side key  (pitch change -2 unless LHp1 is pressed)
byte RH1;   // Right Hand key 1 (with LH3 also pressed pitch change is -2, otherwise -1)
byte RH2;   // Right Hand key 2 (pitch change -1)
byte RH3;   // Right Hand key 3 (pitch change -2)
byte RHp1;  // Right Hand pinky key 1 (pitch change +1)
byte RHp2;  // Right Hand pinky key 2 (pitch change -1)
byte RHp3;  // Right Hand pinky key 3 (pitch change -2)
byte Tr1;  // Trill key 1 (pitch change +2) (EVI fingering)
byte Tr2;  // Trill key 2 (pitch change +1)
byte Tr3;  // Trill key 3 (pitch change +4)
byte K1;   // Valve 1 (pitch change -2)
byte K2;   // Valve 2 (pitch change -1)
byte K3;   // Valve 3 (pitch change -3)
byte K4;   // Left Hand index finger (pitch change -5)
byte K5;   // Trill key 1 (pitch change +2)
byte K6;   // Trill key 2 (pitch change +1)
byte K7;   // Trill key 3 (pitch change +4)
#else
// Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
byte K1;   // Valve 1 (pitch change -2)
byte K2;   // Valve 2 (pitch change -1)
byte K3;   // Valve 3 (pitch change -3)
byte K4;   // Left Hand index finger (pitch change -5)
byte K5;   // Trill key 1 (pitch change +2)
byte K6;   // Trill key 2 (pitch change +1)
byte K7;   // Trill key 3 (pitch change +4)
#endif
byte R1;
byte R2;
byte R3;
byte R4;
byte R5;
byte R6;

byte octaveR = 0;
byte lastOctaveR = 0;

byte halfPitchBendKey;
byte quarterToneTrigger;
byte specialKey;
byte patchKey = 0;
byte pinkyKey = 0;
byte lastSpecialKey = 0;
byte lastPinkyKey = 0;
int pitchlatch;
int reverb;

byte pcCombo1 = 0;
byte pcCombo2 = 0;
byte lastpcc1 = 0;
byte lastpcc2 = 0;

byte slurSustain = 0;
byte parallelChord = 0;
byte subOctaveDouble = 0;
byte slurSostenuto = 0;

#if defined(NURAD)
Adafruit_MPR121 touchSensorRollers = Adafruit_MPR121();
Adafruit_MPR121 touchSensorRH = Adafruit_MPR121();
Adafruit_MPR121 touchSensorLH = Adafruit_MPR121();
#else
Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor
#endif
FilterOnePole breathFilter;
IntervalTimer cvTimer;

bool configManagementMode = false;
bool i2cScan = false;


//_______________________________________________________________________________________________ SETUP

//Update CV output pin, run from timer.
void cvUpdate(){
  int cvPressure = analogRead(breathSensorPin);
  #if defined(NURAD)
  analogWrite(pwmDacPin,map(constrain(cvPressure,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,4095));
  if(dacMode == DAC_MODE_BREATH){
    analogWrite(dacPin,map(constrain(cvPressure,breathThrVal,4095),breathThrVal,4095,0,4095));
  }
  #else
  if(dacMode == DAC_MODE_PITCH){
    analogWrite(pwmDacPin,cvPressure);
  } else { //DAC_MODE_BREATH
    analogWrite(dacPin,map(constrain(cvPressure,breathThrVal,4095),breathThrVal,4095,0,4095));
  }
  #endif
}



void setup() {

  analogReadResolution(12);   // set resolution of ADCs to 12 bit
  analogWriteResolution(12);
  analogWriteFrequency(pwmDacPin,11718.75);
  Wire.setClock(1000000);

  pinMode(dPin, INPUT_PULLUP);
  pinMode(ePin, INPUT_PULLUP);
  pinMode(uPin, INPUT_PULLUP);
  pinMode(mPin, INPUT_PULLUP);

  pinMode(bLedPin, OUTPUT);        // breath indicator LED
  pinMode(pLedPin, OUTPUT);        // portam indicator LED
  pinMode(statusLedPin,OUTPUT);    // Teensy onboard LED
  pinMode(dacPin, OUTPUT);         //DAC output for analog signal
  pinMode(pwmDacPin, OUTPUT);      //PWMed DAC output for analog signal

  #if defined(NURAD)
  pinMode(eLedPin, OUTPUT);        // breath indicator LED
  pinMode(sLedPin, OUTPUT);        // portam indicator LED
  #else
  pinMode(biteJumperPin, INPUT_PULLUP); //PBITE
  pinMode(biteJumperGndPin, OUTPUT);    //PBITE
  digitalWrite(biteJumperGndPin, LOW);  //PBITE
  #endif

  pinMode(widiPowerPin, OUTPUT);        //WIDI
  pinMode(widiJumperPin, INPUT_PULLUP); //WIDI
  pinMode(widiJumperGndPin, OUTPUT);    //WIDI
  digitalWrite(widiJumperGndPin, LOW);  //WIDI

  widiJumper = !digitalRead(widiJumperPin); //WIDI

  Serial2.setRX (26); //WIDI
  Serial2.setTX (31); //WIDI

  bool factoryReset = !digitalRead(ePin) && !digitalRead(mPin);
  configManagementMode = !factoryReset && !digitalRead(uPin) && !digitalRead(dPin);
  i2cScan = !factoryReset && !digitalRead(mPin);

  initDisplay(); //Start up display and show logo

  //If going into config management mode, stop here before we even touch the EEPROM.
  if(configManagementMode) {
    configModeSetup();
    return;
  }

  #if defined(I2CSCANNER)
  if(i2cScan){
    delay(2000);
    i2cScanDisplay();
  }
  #endif


  //Read eeprom data into global vars
  readEEPROM(factoryReset);

  touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);

  activePatch = patch;

  #if defined(NURAD)
  digitalWrite(statusLedPin,HIGH);
  delay(100);
  analogWrite(bLedPin, BREATH_LED_BRIGHTNESS);
  if (!touchSensorRollers.begin(0x5D)) {  //should be D
    while (1){  // Touch sensor initialization failed - stop doing stuff
      if (!digitalRead(dPin) && !digitalRead(ePin) && !digitalRead(uPin) && !digitalRead(mPin)) _reboot_Teensyduino_(); // reboot to program mode if all buttons pressed
    }
  }
  delay(100);
  analogWrite(bLedPin, 0);
  analogWrite(pLedPin, PORTAM_LED_BRIGHTNESS);
  if (!touchSensorLH.begin(0x5C)) {
    while (1){  // Touch sensor initialization failed - stop doing stuff
      if (!digitalRead(dPin) && !digitalRead(ePin) && !digitalRead(uPin) && !digitalRead(mPin)) _reboot_Teensyduino_(); // reboot to program mode if all buttons pressed
    }
  }
  delay(100);
  analogWrite(pLedPin, 0);
  analogWrite(eLedPin, PORTAM_LED_BRIGHTNESS);
  if (!touchSensorRH.begin(0x5B)) {
    while (1){  // Touch sensor initialization failed - stop doing stuff
      if (!digitalRead(dPin) && !digitalRead(ePin) && !digitalRead(uPin) && !digitalRead(mPin)) _reboot_Teensyduino_(); // reboot to program mode if all buttons pressed
    }
  }
  delay(100);
  analogWrite(eLedPin, 0);
  analogWrite(sLedPin, PORTAM_LED_BRIGHTNESS);
  delay(100);
  analogWrite(sLedPin, 0);
  delay(100);
  digitalWrite(statusLedPin,LOW);
  #else
  if (!touchSensor.begin(0x5A)) {
    while (1){  // Touch sensor initialization failed - stop doing stuff
      if (!digitalRead(dPin) && !digitalRead(ePin) && !digitalRead(uPin) && !digitalRead(mPin)) _reboot_Teensyduino_(); // reboot to program mode if all buttons pressed
    }
  }
  #endif

  breathFilter.setFilter(LOWPASS, filterFreq, 0.0);   // create a one pole (RC) lowpass filter

  #if defined(NURAD)
  biteJumper = true;
  #else
  biteJumper = !digitalRead(biteJumperPin);
  if (biteJumper){
    pinMode(bitePin, INPUT);
  }
  #endif

  //auto-calibrate the vibrato threshold while showing splash screen
  vibZero = vibZeroBite = breathCalZero = 0;
  const int sampleCount = 4;
  for(int i = 1 ; i <= sampleCount; ++i) {
    vibZero += touchRead(vibratoPin);
    breathCalZero += analogRead(breathSensorPin);
    if (biteJumper) vibZeroBite += analogRead(bitePressurePin); else vibZeroBite += touchRead(bitePin);
    statusLed(i&1);
    delay(fastBoot?75:220); //Shorter delay for fastboot
  }

  vibZero /= sampleCount;
  breathCalZero /= sampleCount;
  vibZeroBite /= sampleCount;
  leverPortZero = vibZero;
  vibThr = vibZero - vibSquelch;
  vibThrLo = vibZero + vibSquelch;
  vibThrBite = vibZeroBite - vibSquelchBite;
  vibThrBiteLo = vibZeroBite + vibSquelchBite;

  if (factoryReset) autoCal();

  if(!fastBoot) {
    statusLedFlash(500);
    statusLedOff();

    showVersion();

    delay(1400);
  }

  mainState = NOTE_OFF;       // initialize main state machine

  if (!digitalRead(ePin)) {
    activePatch=0;
    doPatchUpdate=1;
  }

  if ((pinkySetting == LVLP) && levelCC){
    midiSendControlChange(levelCC, levelVal);
  }

  for (int i=0; i<50; i++){
    battMeasured[i] = analogRead(vMeterPin);
    delay(1);
  }

  if (widiJumper && widiOn) digitalWrite(widiPowerPin, HIGH); else digitalWrite(widiPowerPin, LOW);

  activeMIDIchannel = MIDIchannel;
  midiInitialize(MIDIchannel);

  //Serial.begin(9600); // debug

  statusLedOn();    // Switch on the onboard LED to indicate power on/ready

  cvTimer.begin(cvUpdate,500); // Update breath CV output every 500 microseconds

}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {

  //If in config mgmt loop, do that and nothing else
  if(configManagementMode) {
    configModeLoop();
    return;
  }

  breathFilter.input(analogRead(breathSensorPin));
  pressureSensor = constrain((int) breathFilter.output(), 0, 4095); // Get the filtered pressure sensor reading from analog pin A0, input from sensor MP3V5004GP
  readSwitches();
  if (mainState == NOTE_OFF) {
    if (activeMIDIchannel != MIDIchannel) {
      activeMIDIchannel = MIDIchannel; // only switch channel if no active note
      midiSetChannel(activeMIDIchannel);
    }
    if ((activePatch != patch) && doPatchUpdate) {
      activePatch = patch;
      midiSendProgramChange(activePatch);
      slurSustain = 0;
      parallelChord = 0;
      subOctaveDouble = 0;
      doPatchUpdate = 0;
    }
    if ((pressureSensor > breathThrVal) || gateOpen) {
      // Value has risen above threshold. Move to the RISE_WAIT
      // state. Record time and initial breath value.
      breath_on_time = millis();
      initial_breath_value = pressureSensor;
      mainState = RISE_WAIT; // Go to next state
    }
    if (legacy || legacyBrAct) {

      bool bothPB = (pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) && (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2));
      bool justPbDn = !(pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) && (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2));
      bool justPbUp = (pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) && !(pbDn > ((pitchbMaxVal + pitchbThrVal) / 2));
      bool noPb = !(pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) && !(pbDn > ((pitchbMaxVal + pitchbThrVal) / 2));
      bool brSuck = analogRead(breathSensorPin) < (breathCalZero - 850);
      int pitchlatchForPatch = patchLimit(pitchlatch + 1);
      if (pcCombo1 && (pcCombo1 != lastpcc1)){ // latched note number to patch number, send with K1/K5 combo
        if (patch != pitchlatchForPatch) {
          patch = pitchlatchForPatch;
          doPatchUpdate = 1;
        }
      } else if (pcCombo2 && (pcCombo2 != lastpcc2)) { // hi and lo patch numbers, send with K2/K6 combo
        if (pitchlatch > 75) {
          if (patch != patchLimit(pitchlatchForPatch + 24)) {
            patch = patchLimit(pitchlatchForPatch + 24); // add 24 to get high numbers 108 to 127
            doPatchUpdate = 1;
          }
        } else {
          if (patch != patchLimit(pitchlatchForPatch - 36)) {
            patch = patchLimit(pitchlatchForPatch - 36); // subtract 36 to get low numbers 0 to 36
            doPatchUpdate = 1;
          }
        }
      }
      lastpcc1=pcCombo1;
      lastpcc2=pcCombo2;
      if (
          patchKey ||
          (bothPB && legacy) ||
          (brSuck && legacyBrAct && justPbUp) ||
          (brSuck && legacyBrAct && bcasMode && noPb)
          ) { // both pb pads touched or br suck


        int fingeredNoteUntransposedForPatch = patchLimit(fingeredNoteUntransposed + 1);
        if (exSensor >= ((extracThrVal + extracMaxVal) / 2)) { // instant midi setting
          if ((fingeredNoteUntransposedForPatch >= 73) && (fingeredNoteUntransposedForPatch <= 88)) {
            MIDIchannel = fingeredNoteUntransposedForPatch - 72; // Mid C and up
          }
        } else {
          if (!pinkyKey) { // note number to patch number
            if (patch != fingeredNoteUntransposedForPatch) {
              patch = fingeredNoteUntransposedForPatch;
              doPatchUpdate = 1;
            }
          } else { // hi and lo patch numbers
            if (fingeredNoteUntransposedForPatch > 75) {
              if (patch != patchLimit(fingeredNoteUntransposedForPatch + 24)) {
                patch = patchLimit(fingeredNoteUntransposedForPatch + 24); // add 24 to get high numbers 108 to 127
                doPatchUpdate = 1;
              }
            } else {
              if (patch != patchLimit(fingeredNoteUntransposedForPatch - 36)) {
                patch = patchLimit(fingeredNoteUntransposedForPatch - 36); // subtract 36 to get low numbers 0 to 36
                doPatchUpdate = 1;
              }
            }
          }
        }
      } else {
        if (justPbDn && legacyBrAct && brSuck && programonce == false) { // down bend for suck programming button
          programonce = true;

          if (octaveR == 0) { //lowest octave position

            if (K1 && K2 && !K3 && K4) { //  e28 send patch change -10
              patch = patch - 10;
              doPatchUpdate = 1;
            } else if (K1 && !K2 && !K3 && K4) { //f29 decrement and send patch change
              patch--;
              doPatchUpdate = 1;
            } else if (!K1 && K2 && !K3 && K4) { //f#30 send patch change +10
              patch = patch + 10;
              doPatchUpdate = 1;
            } else if (!K1 && !K2 && !K3 && K4) { //g31 increment and send patch change
              patch++;
              doPatchUpdate = 1;
            }

            if (!K1 && !K2 && K3 && !K4) { //send reverb pitchlatch value
              reverb = ((pitchlatch - 36) * 2);
              reverb = constrain(reverb, 0, 127);

              midiSendControlChange(91, reverb);
            }
          }

          if (octaveR == 3) { //middle octave position to set breath parameters
            // breathCC value is from cclist[] which assigns controller number
            if (K1) { //turn on midi volume
              breathCC = 3;
              midiSendControlChange(7, 0); //midi vol to 0
              midiSendControlChange(11, 127); //midi expression to 127
            }
            if (K3) { //turn on midi breath controller
              breathCC = 2;
              midiSendControlChange(7, 127); //midi vol to 127
              midiSendControlChange(11, 127); //midi expression to 127
            }
            if (K4) { //sb turn on midi expression
              breathCC = 4;
              midiSendControlChange(7, 127); //midi vol to 127
              midiSendControlChange(11, 0); //midi expression to 0
            }
            if (K2) { //2v turn on aftertouch
              breathAT = 1;
              midiSendControlChange(7, 127); //midi vol to 127
              midiSendControlChange(11, 127); //midi expression to 0
            } else {
              breathAT = 0;
            }
            if (K5) { //1tr turn on velocity
              velocity = 0;
              midiSendControlChange(7, 127); //midi vol to 127
              midiSendControlChange(11, 127); //midi expression to 0
            } else {
              velocity = 127;
            }
            if (!K1 && !K3 && !K4) {
              breathCC = 0;
              midiSendControlChange(7, 127); //midi vol to 127
              midiSendControlChange(11, 127); //midi expression to 127
            }
          }
        }
      }
    }

    if (analogRead(breathSensorPin) > (breathCalZero - 850)) programonce = false;
    #if defined(NURAD)
    #else
    specialKey = (touchRead(specialKeyPin) > touch_Thr); //S2 on pcb
    #endif
    if (polySelect != EHarmonizerOff) {
      if (lastSpecialKey != specialKey) {
        if (specialKey) {
          // special key just pressed, check other keys
          if (K4) {
            if (!slurSustain) {
              slurSustain = 1;
              slurSostenuto = 0;
              parallelChord = 0;
              rotatorOn = 0;
            } else slurSustain = 0;
          }
          if (K5) {
            if (!parallelChord) {
              parallelChord = 1;
              slurSustain = 0;
              rotatorOn = 0;
            } else parallelChord = 0;
          }
          if (K1) {
            if (!subOctaveDouble) {
              subOctaveDouble = 1;
              rotatorOn = 0;
            } else subOctaveDouble = 0;
          }
          if (K2) {
            if (!slurSostenuto) {
              slurSostenuto = 1;
              slurSustain = 0;
              //rotatorOn = 0;
            } else slurSostenuto = 0;
          }
          if (!K1 && !K4 && !K5 && !K2 && !pinkyKey) {
            slurSustain = 0;
            parallelChord = 0;
            subOctaveDouble = 0;
            rotatorOn = 0;
            slurSostenuto = 0;
          }
          if (pinkyKey) {
            if (!rotatorOn) {
              rotatorOn = 1;
              currentRotation = 3;
              slurSustain = 0;
              parallelChord = 0;
              subOctaveDouble = 0;
            } else rotatorOn = 0;
          }
        }
      }
      lastSpecialKey = specialKey;
    } else {
      rotatorOn = 0;
      slurSustain = 0;
      parallelChord = 0;
      subOctaveDouble = 0;
      slurSostenuto = 0;
    }
    if ((pinkySetting == LVL) || (pinkySetting == LVLP)){
      if (pinkyKey && K7){
        ledMeter(levelVal);
        if (K6 && (levelVal < 127)){
          if (currentTime - lvlTime > (LVL_TIMER_INTERVAL)){
            levelVal++;
            if (levelCC) midiSendControlChange(levelCC, levelVal);
            else midiSendAfterTouch(levelVal);
            lvlTime = currentTime;
          }
        } else if (K5 && (levelVal > 0)){
          if (currentTime - lvlTime > (LVL_TIMER_INTERVAL)){
            levelVal--;
            if (levelCC) midiSendControlChange(levelCC, levelVal);
            else midiSendAfterTouch(levelVal);
            lvlTime = currentTime;
          }
       }
      } else if (!pinkyKey && lastPinkyKey){
        writeSetting(LEVEL_VAL_ADDR,levelVal);
      }
      lastPinkyKey = pinkyKey;
    } else if (pinkySetting == GLD){
      /*if (pinkyKey && K7){
        ledMeter(portLimit);
        if (K6 && (portLimit < 127)){
          if (currentTime - lvlTime > (LVL_TIMER_INTERVAL)){
            portLimit++;
            if (portamento && (portamento != 5)) midiSendControlChange(CCN_Port, portLimit);
            lvlTime = currentTime;
          }
        } else if (K5 && (portLimit > 1)){
          if (currentTime - lvlTime > (LVL_TIMER_INTERVAL)){
            portLimit--;
            if (portamento && (portamento != 5)) midiSendControlChange(CCN_Port, portLimit);
            lvlTime = currentTime;
          }
        }
      } else if (!pinkyKey && lastPinkyKey){
        writeSetting(PORTLIMIT_ADDR,portLimit);
      }*/
      lastPinkyKey = pinkyKey;
    }
  } else if (mainState == RISE_WAIT) {
    if ((pressureSensor > breathThrVal) || gateOpen) {
      // Has enough time passed for us to collect our second
      // sample?
      if ((millis() - breath_on_time > velSmpDl) || (0 == velSmpDl) || velocity) {
        // Yes, so calculate MIDI note and velocity, then send a note on event
        // We should be at tonguing peak, so set velocity based on current pressureSensor value unless fixed velocity is set
        breathLevel = constrain(max(pressureSensor, initial_breath_value), breathThrVal, breathMaxVal);
        if (!velocity) {
          unsigned int breathValHires = breathCurve(map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, 0, 16383));
          velocitySend = (breathValHires >> 7) & 0x007F;
          velocitySend = constrain(velocitySend + velocitySend * .1 * velBias, 1, 127);
          //velocitySend = map(constrain(max(pressureSensor,initial_breath_value),breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,1,127);
        } else velocitySend = velocity;
        breath(); // send breath data
        fingeredNote = noteValueCheck(fingeredNote);
        if (priority) { // mono prio to last chord note
          midiSendNoteOn(fingeredNote, velocitySend); // send Note On message for new note
        }
        if (parallelChord) {
          for (int i = 0; i < addedIntervals; i++) {
            midiSendNoteOn(noteValueCheck(fingeredNote + slurInterval[i]), velocitySend); // send Note On message for new note
          }
        }
        if (slurSustain) {
          midiSendControlChange(64, 127);
          slurBase = fingeredNote;
          addedIntervals = 0;
        }
        if (subOctaveDouble) {
          midiSendNoteOn(noteValueCheck(fingeredNote - 12), velocitySend);
          if (parallelChord) {
            for (int i = 0; i < addedIntervals; i++) {
              midiSendNoteOn(noteValueCheck(fingeredNote + slurInterval[i] - 12), velocitySend); // send Note On message for new note
            }
          }
        }
        if (rotatorOn) {
          startHarmonizerNotes(fingeredNote);
        }
        if (!priority) { // mono prio to base note
          midiSendNoteOn(fingeredNote, velocitySend); // send Note On message for new note
        }
        if (slurSostenuto) {
          midiSendControlChange(66, 127);
        }
        activeNote = fingeredNote;
        mainState = NOTE_ON;
      }
    } else {
      // Value fell below threshold before velocity sample delay time passed. Return to
      // NOTE_OFF state (e.g. we're ignoring a short blip of breath)
      mainState = NOTE_OFF;
    }
  } else if (mainState == NOTE_ON) {
    if ((pressureSensor < breathThrVal) && !gateOpen) {
      // Value has fallen below threshold - turn the note off
      activeNote = noteValueCheck(activeNote);
      if (priority) {
        midiSendNoteOff(activeNote); //  send Note Off message
      }
      if (parallelChord) {
        for (int i = 0; i < addedIntervals; i++) {
          midiSendNoteOff(noteValueCheck(activeNote + slurInterval[i])); // send Note On message for new note
        }
      }
      if (subOctaveDouble) {
        midiSendNoteOff(noteValueCheck(activeNote - 12));
        if (parallelChord) {
          for (int i = 0; i < addedIntervals; i++) {
            midiSendNoteOff(noteValueCheck(activeNote + slurInterval[i] - 12)); // send Note On message for new note
          }
        }
      }
      if (rotatorOn) {
        stopHarmonizerNotes(activeNote);
      }
      if (!priority) {
        midiSendNoteOff(activeNote); //  send Note Off message
      }
      if (slurSustain) {
        midiSendControlChange(64, 0);
      }
      if (slurSostenuto) {
        midiSendControlChange(66, 0);
      }
      breathLevel = 0;
      mainState = NOTE_OFF;
    } else {
      if (noteValueCheck(fingeredNote) != activeNote) {
        // Player has moved to a new fingering while still blowing.
        // Send a note off for the current note and a note on for
        // the new note.
        if (!velocity) {
          unsigned int breathValHires = breathCurve(map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, 0, 16383));
          velocitySend = (breathValHires >> 7) & 0x007F;
          velocitySend = constrain(velocitySend + velocitySend * .1 * velBias, 1, 127);
          //velocitySend = map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,7,127); // set new velocity value based on current pressure sensor level
        }
        activeNote = noteValueCheck(activeNote);
        if ((parallelChord || subOctaveDouble || rotatorOn) && priority) { // poly playing, send old note off before new note on
          midiSendNoteOff(activeNote); // send Note Off message for old note
        }

        if (parallelChord) {
          for (int i = 0; i < addedIntervals; i++) {
            midiSendNoteOff(noteValueCheck(activeNote + slurInterval[i])); // send Note Off message for old note
          }
        }
        if (subOctaveDouble) {
          midiSendNoteOff(noteValueCheck(activeNote - 12)); // send Note Off message for old note
          if (parallelChord) {
            for (int i = 0; i < addedIntervals; i++) {
              midiSendNoteOff(noteValueCheck(activeNote + slurInterval[i] - 12)); // send Note Off message for old note
            }
          }
        }
        if (rotatorOn) {
          stopHarmonizerNotes(activeNote);
        }

        if ((parallelChord || subOctaveDouble || rotatorOn) && !priority) { // poly playing, send old note off before new note on
          midiSendNoteOff(activeNote); // send Note Off message for old note
        }

        fingeredNote = noteValueCheck(fingeredNote);
        if (priority) {
          midiSendNoteOn(fingeredNote, velocitySend); // send Note On message for new note
        }
        if (parallelChord) {
          for (int i = 0; i < addedIntervals; i++) {
            midiSendNoteOn(noteValueCheck(fingeredNote + slurInterval[i]), velocitySend); // send Note On message for new note
          }
        }
        if (subOctaveDouble) {
          midiSendNoteOn(noteValueCheck(fingeredNote - 12), velocitySend); // send Note On message for new note
          if (parallelChord) {
            for (int i = 0; i < addedIntervals; i++) {
              midiSendNoteOn(noteValueCheck(fingeredNote + slurInterval[i] - 12), velocitySend); // send Note On message for new note
            }
          }
        }
        if (rotatorOn) {
          startHarmonizerNotes(fingeredNote);
        }

        if (!priority) {
          midiSendNoteOn(fingeredNote, velocitySend); // send Note On message for new note
        }

        if (!parallelChord && !subOctaveDouble && !rotatorOn) { // mono playing, send old note off after new note on
          delayMicroseconds(2000); //delay for midi recording fix
          midiSendNoteOff(activeNote); //  send Note Off message
        }

        if (slurSustain) {
          if (addedIntervals < 9) {
            addedIntervals++;
            slurInterval[addedIntervals - 1] = fingeredNote - slurBase;
          }
        }
        activeNote = fingeredNote;
      }
    }
    if (pressureSensor > breathThrVal) cursorBlinkTime = millis(); // keep display from updating with cursor blinking if breath is over thr
  }
  // Is it time to send more CC data?
  currentTime = millis();
  if ((currentTime - ccBreathSendTime) > (breathInterval-1u)){
    breath();
    ccBreathSendTime = currentTime;
  }
  if (currentTime - ccSendTime > CC_INTERVAL) {
    // deal with Pitch Bend, Modulation, etc.
    pitch_bend();
    extraController();
    biteCC_();
    leverCC_();
    ccSendTime = currentTime;
  }
  if (currentTime - ccSendTime2 > CC_INTERVAL2) {
    portamento_();
    ccSendTime2 = currentTime;
  }
  if (currentTime - ccSendTime3 > CC_INTERVAL3) {
    if (gateOpenEnable || gateOpen) doorKnobCheck();
    battCheck();
    if (((pinkySetting == LVL) || (pinkySetting == LVLP) || (pinkySetting == GLD)) && pinkyKey && K7 && (mainState == NOTE_OFF)){
      // show LVL indication
    } else updateSensorLEDs();
    ccSendTime3 = currentTime;
  }
  if (currentTime - pixelUpdateTime > pixelUpdateInterval) {
    // even if we just alter a pixel, the whole display is redrawn (35ms of MPU lockup) and we can't do that all the time
    // this is one of the big reasons the display is for setup use only
    drawSensorPixels(); // live sensor monitoring for the setup screens
    if (rotatorOn || slurSustain || parallelChord || subOctaveDouble || slurSostenuto || gateOpen) {
      statusLedFlip();
    } else {
      statusLedOn();
    }
    pixelUpdateTime = currentTime;
  }

  if(dacMode == DAC_MODE_PITCH) { // pitch CV from DAC and breath CV from PWM on pin 6, for filtering and scaling on separate board
    targetPitch = (fingeredNote-24)*42;
    targetPitch += map(pitchBend,0,16383,-84,84);
    targetPitch -=quarterToneTrigger*21;
    if (portIsOn){
      if (targetPitch > cvPitch){
        if (!cvPortaTuneCount) {
          cvPitch += 1+(127-oldport)/4;
        }
        else {
          cvPortaTuneCount++;
          if (cvPortaTuneCount > CVPORTATUNE) cvPortaTuneCount=0;
        }
        if (cvPitch > targetPitch) cvPitch = targetPitch;
      } else if (targetPitch < cvPitch){
        if (!cvPortaTuneCount) {
          cvPitch -= 1+(127-oldport)/4;
        }
        else {
          cvPortaTuneCount++;
          if (cvPortaTuneCount > CVPORTATUNE) cvPortaTuneCount=0;
        }
        if (cvPitch < targetPitch) cvPitch = targetPitch;
      } else {
        cvPitch = targetPitch;
      }
    } else {
      cvPitch = targetPitch;
    }

    if (cvVibRate){
      int timeDivider = timeDividerList[cvVibRate];
      int cvVib = map(((waveformsTable[map(currentTime%timeDivider, 0, timeDivider, 0, maxSamplesNum-1)] - 2047) * exSensorIndicator), -259968,259969,-11,11);
      cvPitch += cvVib;
    }
    int cvPitchTuned = 2*(cvTune-100)+map(cvPitch,0,4032,0,4032+2*(cvScale-100));
    analogWrite(dacPin,constrain(cvPitchTuned,0,4095));
  } else if(dacMode == DAC_MODE_BREATH) { // else breath CV on DAC pin, directly to unused pin of MIDI DIN jack
    //analogWrite(dacPin,breathCurve(map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,4095)));
  }

  midiDiscardInput();

  //do menu stuff
  menu();
}

//_______________________________________________________________________________________________ FUNCTIONS

static void sendHarmonizerData( uint8_t note, const int harmony[][3], bool sendOn )
{
  const int* offs = harmony[(note-hmzKey)%12];
  if(sendOn) {
    midiSendNoteOn(noteValueCheck(note+offs[0]), velocitySend);
    if (hmzLimit>2) midiSendNoteOn(noteValueCheck(note+offs[1]), velocitySend);
    if (hmzLimit>3) midiSendNoteOn(noteValueCheck(note+offs[2]), velocitySend);
  } else {
    midiSendNoteOff(noteValueCheck(note+offs[0]));
    if (hmzLimit>2) midiSendNoteOff(noteValueCheck(note+offs[1]));
    if (hmzLimit>3) midiSendNoteOff(noteValueCheck(note+offs[2]));
  }
}

//**************************************************************

static void updateRotator(byte note, const Rotator *rotator) {
  auto parallel = rotator->parallel;
  auto rotations = rotator->rotations;

  if (parallel-24) {
    midiSendNoteOn(noteValueCheck(note + parallel-24), velocitySend); // send Note On message for new note
  }

  currentRotation = (currentRotation +1) % 4;
    
  int allCheck=4;
  while ((0 == rotations[currentRotation]-24) && allCheck){
    if (currentRotation < 3) currentRotation++;
    else currentRotation = 0;
    allCheck--;
  }
  if (rotations[currentRotation]-24) midiSendNoteOn(noteValueCheck(note + rotations[currentRotation]-24), velocitySend); // send Note On message for new note
}

//**************************************************************

static void stopRotatorNotes(byte note, const Rotator *rotator) {
  if (rotator->parallel - 24) midiSendNoteOff(noteValueCheck(note + rotator->parallel-24 )); // send Note Off message for old note
  if (rotator->rotations[currentRotation]-24) midiSendNoteOff(noteValueCheck(note + rotator->rotations[currentRotation]-24)); // send Note Off message for old note
}

static void stopHarmonizerNotes(byte note)
{
  switch(polySelect) {
    case ETriadMajorGospelRoot:      sendHarmonizerData(note, majGosRootHmz, false); break;
    case ETriadMajorGospelDominant:  sendHarmonizerData(note, majGosDomHmz, false);  break;
    case EMajorAddNine:              sendHarmonizerData(note, majAdd9Hmz, false);    break;
    case EMinorDorian:               sendHarmonizerData(note, minDorHmz, false);     break;
    case EMinorAeolian:              sendHarmonizerData(note, minAeoHmz, false);     break;
    case EMinorFourVoiceHip:         sendHarmonizerData(note, minHipHmz, false);     break;
    case EFourWayCloseHarmonizer:
    {
      if (!fwcDrop2 || (hmzLimit>(3+fwcLockH))) midiSendNoteOff(noteValueCheck(note+blockFWC[fwcType][(note-hmzKey)%12][0]-12*fwcDrop2));
      if ((hmzLimit+fwcDrop2)>2) midiSendNoteOff(noteValueCheck(note+blockFWC[fwcType][(note-hmzKey)%12][1]));
      if ((hmzLimit+fwcDrop2)>3) midiSendNoteOff(noteValueCheck(note+blockFWC[fwcType][(note-hmzKey)%12][2]));
      if (((hmzLimit+fwcDrop2)>4) && (1 == fwcLockH)) midiSendNoteOff(noteValueCheck(note-12));
    }
    break;

    case ERotatorA: stopRotatorNotes(note, &rotations_a); break;
    case ERotatorB: stopRotatorNotes(note, &rotations_b); break;
    case ERotatorC: stopRotatorNotes(note, &rotations_c); break;

    default: break;
  }
}

static void startHarmonizerNotes(byte note)
{
  switch(polySelect) {
    case ETriadMajorGospelRoot:      sendHarmonizerData(note, majGosRootHmz, true); break;
    case ETriadMajorGospelDominant:  sendHarmonizerData(note, majGosDomHmz, true);  break;
    case EMajorAddNine:              sendHarmonizerData(note, majAdd9Hmz, true);    break;
    case EMinorDorian:               sendHarmonizerData(note, minDorHmz, true);     break;
    case EMinorAeolian:              sendHarmonizerData(note, minAeoHmz, true);     break;
    case EMinorFourVoiceHip:         sendHarmonizerData(note, minHipHmz, true);     break;
    case EFourWayCloseHarmonizer:
    {
      int limit = (hmzLimit+fwcDrop2);
      if (!fwcDrop2 || (hmzLimit>(3+fwcLockH))) midiSendNoteOn(noteValueCheck(note+blockFWC[fwcType][(note-hmzKey)%12][0]-12*fwcDrop2), velocitySend);
      if (limit>2)  midiSendNoteOn(noteValueCheck(note+blockFWC[fwcType][(note-hmzKey)%12][1]), velocitySend);
      if (limit>3)  midiSendNoteOn(noteValueCheck(note+blockFWC[fwcType][(note-hmzKey)%12][2]), velocitySend);
      if ((limit>4) && (1 == fwcLockH)) midiSendNoteOn(noteValueCheck(note-12), velocitySend);
    }
    break;

    case ERotatorA: updateRotator(note, &rotations_a); break;
    case ERotatorB: updateRotator(note, &rotations_b); break;
    case ERotatorC: updateRotator(note, &rotations_c); break;

    default: break;
  }
}


//**************************************************************
// non linear mapping function (http://playground.arduino.cc/Main/MultiMap)
// note: the _in array should have increasing values
unsigned int multiMap(unsigned short val, const unsigned short * _in, const unsigned short * _out, uint8_t size) {
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size - 1]) return _out[size - 1];

  // search right interval
  uint8_t pos = 1; // _in[0] allready tested
  while (val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos - 1]) * (_out[pos] - _out[pos - 1]) / (_in[pos] - _in[pos - 1]) + _out[pos - 1];
}

//**************************************************************

// map breath values to selected curve
unsigned int breathCurve(unsigned int inputVal) {
  if(curve >= ARR_LEN(curves)) return inputVal;
  return multiMap(inputVal, curveIn, curves[curve], 17);
}

// MIDI note value check with out of range octave repeat
inline int noteValueCheck(int note) {
  if (note > 127) {
    note = 115 + (note - 127) % 12;
  } else if (note < 0) {
    note = 12 - abs(note) % 12;
  }
  return note;
}

//**************************************************************

int patchLimit(int value) {
  return constrain(value, 1, 128);
}

//**************************************************************

void breath() {
  int breathCCval, breathCCvalFine,breathCC2val;
  unsigned int breathCCvalHires;
  breathLevel = constrain(pressureSensor, breathThrVal, breathMaxVal);
  //breathLevel = breathLevel*0.6+pressureSensor*0.4; // smoothing of breathLevel value
  ////////breathCCval = map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,127);
  breathCCvalHires = breathCurve(map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, 0, 16383));
  breathCCval = (breathCCvalHires >> 7) & 0x007F;
  breathCCvalFine = breathCCvalHires & 0x007F;
  breathCC2val = constrain(breathCCval*breathCC2Rise,0,127);
  if (brHarmSetting) brHarmonics = map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, 0, brHarmSetting); else brHarmonics = 0;
  if (breathCCval != oldbreath) { // only send midi data if breath has changed from previous value
    if (breathCC) {
      // send midi cc
      midiSendControlChange(ccList[breathCC], breathCCval);
    }
    if (breathAT) {
      // send aftertouch
      midiSendAfterTouch(breathCCval);
    }
    oldbreath = breathCCval;
  }

  if (breathCCvalHires != oldbreathhires) {
    if ((breathCC > 4) && (breathCC < 9)) { // send high resolution midi
      midiSendControlChange(ccList[breathCC] + 32, breathCCvalFine);
    }
    oldbreathhires = breathCCvalHires;
  }
  if (breathCC2val != oldbreathcc2){
    if (breathCC2 && (breathCC2 != ccList[breathCC])){
      midiSendControlChange(breathCC2, breathCC2val);
    }
    oldbreathcc2 = breathCC2val;
  }
}

//**************************************************************

void pitch_bend() {
  // handle input from pitchbend touchpads and
  // on-pcb variable capacitor for vibrato.
  int vibMax;
  int vibMaxBite;
  int calculatedPBdepth;
  byte pbTouched = 0;
  int vibRead = 0;
  int vibReadBite = 0;
  pbUp = touchRead(pbUpPin); // PCB PIN "Pu"
  pbDn = touchRead(pbDnPin); // PCB PIN "Pd"
  halfPitchBendKey = (pinkySetting == PBD) && pinkyKey; // hold pinky key for 1/2 pitchbend value
  quarterToneTrigger = (pinkySetting == QTN) && pinkyKey; // pinky key for a quarter tone down using pitch bend (assuming PB range on synth is set to 2 semitones)

  calculatedPBdepth = pbDepthList[PBdepth];
  if (halfPitchBendKey) calculatedPBdepth = calculatedPBdepth * 0.5;


  vibMax = vibMaxList[vibSens - 1];
  vibMaxBite = vibMaxBiteList[vibSensBite - 1];

  if (1 == biteControl){ //bite vibrato
    if (biteJumper){ //PBITE (if pulled low with jumper, or NuRAD compile, use pressure sensor instead of capacitive bite sensor)
      vibReadBite = analogRead(bitePressurePin); // alternative kind bite sensor (air pressure tube and sensor)  PBITE
    } else {
      vibReadBite = touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right)
    }
    if (vibReadBite < vibThrBite) {
      if (UPWD == vibDirection) {
        vibSignal = (vibSignal + map(constrain(vibReadBite, (vibZeroBite - vibMaxBite), vibThrBite), vibThrBite, (vibZeroBite - vibMaxBite), 0, calculatedPBdepth * vibDepth[vibrato]))/2;
      } else {
        vibSignal = (vibSignal + map(constrain(vibReadBite, (vibZeroBite - vibMaxBite), vibThrBite), vibThrBite, (vibZeroBite - vibMaxBite), 0, (0 - calculatedPBdepth * vibDepth[vibrato])))/2;
      }
    } else if (vibReadBite > vibThrBiteLo) {
      if (UPWD == vibDirection) {
        vibSignal = (vibSignal + map(constrain(vibReadBite, vibThrBiteLo, (vibZeroBite + vibMaxBite)), vibThrBiteLo, (vibZeroBite + vibMaxBite), 0, (0 - calculatedPBdepth * vibDepth[vibrato])))/2;
      } else {
        vibSignal = (vibSignal + map(constrain(vibReadBite, vibThrBiteLo, (vibZeroBite + vibMaxBite)), vibThrBiteLo, (vibZeroBite + vibMaxBite), 0, calculatedPBdepth * vibDepth[vibrato]))/2;
      }
    } else {
      vibSignal = vibSignal / 2;
    }
  }
  if (1 == leverControl) { //lever vibrato
    vibRead = touchRead(vibratoPin); // SENSOR PIN 15 - built in var cap
    if (vibRead < vibThr) {
      if (UPWD == vibDirection) {
        vibSignal = (vibSignal + map(constrain(vibRead, (vibZero - vibMax), vibThr), vibThr, (vibZero - vibMax), 0, calculatedPBdepth * vibDepth[vibrato]))/2;
      } else {
        vibSignal = (vibSignal + map(constrain(vibRead, (vibZero - vibMax), vibThr), vibThr, (vibZero - vibMax), 0, (0 - calculatedPBdepth * vibDepth[vibrato])))/2;
      }
    } else if (vibRead > vibThrLo) {
      if (UPWD == vibDirection) {
        vibSignal = (vibSignal + map(constrain(vibRead, vibThrLo, (vibZero + vibMax)), vibThrLo, (vibZero + vibMax), 0, (0 - calculatedPBdepth * vibDepth[vibrato])))/2;
      } else {
        vibSignal = (vibSignal + map(constrain(vibRead, vibThrLo, (vibZero + vibMax)), vibThrLo, (vibZero + vibMax), 0, calculatedPBdepth * vibDepth[vibrato]))/2;
      }
    } else {
      vibSignal = vibSignal / 2;
    }
  }



  switch (vibRetn) { // moving baseline
  case 0:
    //keep vibZero value
    break;
  case 1:
    vibZero = vibZero * 0.95 + vibRead * 0.05;
    vibZeroBite = vibZeroBite * 0.95 + vibReadBite * 0.05;
    break;
  case 2:
    vibZero = vibZero * 0.9 + vibRead * 0.1;
    vibZeroBite = vibZeroBite * 0.9 + vibReadBite * 0.1;
    break;
  case 3:
    vibZero = vibZero * 0.8 + vibRead * 0.2;
    vibZeroBite = vibZeroBite * 0.8 + vibReadBite * 0.2;
    break;
  case 4:
    vibZero = vibZero * 0.6 + vibRead * 0.4;
    vibZeroBite = vibZeroBite * 0.6 + vibReadBite * 0.4;
  }
  vibThr = vibZero - vibSquelch;
  vibThrLo = vibZero + vibSquelch;
  vibThrBite = vibZeroBite - vibSquelchBite;
  vibThrBiteLo = vibZeroBite + vibSquelchBite;
  int pbPos = map(constrain(pbUp, pitchbThrVal, pitchbMaxVal), pitchbThrVal, pitchbMaxVal, 0, calculatedPBdepth);
  int pbNeg = map(constrain(pbDn, pitchbThrVal, pitchbMaxVal), pitchbThrVal, pitchbMaxVal, 0, calculatedPBdepth);
  int pbSum = 8193 + pbPos - pbNeg;
  int pbDif = abs(pbPos - pbNeg);

  if (((pbUp > pitchbThrVal) && PBdepth) || ((pbDn > pitchbThrVal) && PBdepth)) {
    if (pbDif < 10) {
      pitchBend = 8192;
    } else {
      pitchBend = pitchBend * 0.6 + 0.4 * pbSum;
    }
    pbTouched = 1;
  }
  if (!pbTouched) {
    pitchBend = pitchBend * 0.6 + 8192 * 0.4; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
  }

  pitchBend = pitchBend + vibSignal;

  pitchBend = constrain(pitchBend, 0, 16383);

  pbSend = pitchBend - quarterToneTrigger*calculatedPBdepth*0.25;

  pbSend = constrain(pbSend, 0, 16383);

  if (subVibSquelch && (8192 != pitchBend)) {
    statusLedOff();
    vibLedOff = 1;
  } else if (vibLedOff) {
    statusLedOn();
    vibLedOff = 0;
  }

  //Serial.print(pitchBend);
  //Serial.print(" - ");
  //Serial.println(oldpb);

  if (pbSend != oldpb) { // only send midi data if pitch bend has changed from previous value
    midiSendPitchBend(pbSend);
    oldpb = pbSend;
  }
}

//***********************************************************

void doorKnobCheck() {

  if (gateOpenEnable){
    #if defined(NURAD)
    if (R2 && R3 && R4 && R5) { // fold thumb in to cover R2 through R5
      if (!gateOpen && (pbUp > ((pitchbMaxVal + pitchbThrVal) / 2))) {
        gateOpen = 1;
        statusLedFlash(100);
      } else if (gateOpen && (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2))) {
        gateOpen = 0;
        midiPanic();
        statusLedFlash(100);
        statusLedFlash(100);
        statusLedFlash(100);
        delay(600);
      }
    }
    #else
    if (K4 && R1 && R2 && R3) { // doorknob grip on canister
      if (!gateOpen && (pbUp > ((pitchbMaxVal + pitchbThrVal) / 2))) {
        gateOpen = 1;
        statusLedFlash(100);
      } else if (gateOpen && (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2))) {
        gateOpen = 0;
        midiPanic();
        statusLedFlash(100);
        statusLedFlash(100);
        statusLedFlash(100);
        delay(600);
      }
    }
    #endif
  } else if (gateOpen) {
    gateOpen = 0;
    midiPanic();
  }
}

//***********************************************************

void battCheck(){
    battMeasured[battCheckPos] = analogRead(vMeterPin);
    battAvg = 0;
    for (int i=0; i<50; i++){
      battAvg += battMeasured[i];
    }
    battAvg /= 50;
    battCheckPos++;
    if (battCheckPos == 50) battCheckPos = 0;
}

void extraController() {
  bool CC2sw = false;
  bool CC1sw = false;
  int extracCC;
  // Extra Controller is the lip touch sensor (proportional) in front of the mouthpiece
  exSensor = exSensor * 0.6 + 0.4 * touchRead(extraPin); // get sensor data, do some smoothing - SENSOR PIN 16 - PCB PIN "EC" (marked K4 on some prototype boards)
  exSensorIndicator = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 0, 127);
  if (pinkySetting == EC2){
    CC1sw = true;
    //send 0 or 127 on extra controller CC2 depending on pinky key touch
    if (pinkyKey && extraCT2) {
      if (lastPinkyKey != pinkyKey){
        midiSendControlChange(extraCT2, 127);
        lastPinkyKey = pinkyKey;
      }
    } else {
      if (lastPinkyKey != pinkyKey){
        midiSendControlChange(extraCT2, 0);
        lastPinkyKey = pinkyKey;
      }
    }
  } else if (pinkySetting == ECSW){
    if (pinkyKey){
      //send extra controller CC2 only
      CC2sw = true;
      CC1sw = false;
    } else {
      //send extra controller primary CC only
      CC2sw = false;
      CC1sw = true;
    }
  } else if (pinkySetting == ECH){
    if (pinkyKey){
      //extra controller harmonics only
      CC2sw = false;
      CC1sw = false;
    } else {
      //send extra controller primary CC only
      CC2sw = false;
      CC1sw = true;
    }
  } else {
    //send both primary CC and CC2
    CC2sw = true;
    CC1sw = true;
  }

  if ((harmSetting && (pinkySetting != ECH)) || ((pinkySetting == ECH) && pinkyKey)){
    if (harmSelect < 4){
      harmonics = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 0, harmSetting);
    } else {
      harmonics = map(constrain(exSensor, extracThrVal, extracMaxVal), extracMaxVal, extracThrVal, 0, harmSetting);
    }
  } else if ((pinkySetting == ECH) && !pinkyKey) {
    harmonics = 0;
  }

  if ((extraCT || extraCT2) && (exSensor >= extracThrVal)) { // if we are enabled and over the threshold, send data
    if (!extracIsOn) {
      extracIsOn = 1;
      if ((extraCT == 4) && CC1sw) { //Sustain ON
        midiSendControlChange(64, 127);
      }
    }
    if ((extraCT == 1) && CC1sw) { //Send modulation
      extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac) {
        midiSendControlChange(1, extracCC);
      }
      oldextrac = extracCC;
    }
    if ((extraCT == 2) && CC1sw) { //Send foot pedal (CC#4)
      extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac) {
        midiSendControlChange(4, extracCC);
      }
      oldextrac = extracCC;
    }
    if ((extraCT == 3) && (breathCC != 9) && CC1sw) { //Send filter cutoff (CC#74)
      extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac) {
        midiSendControlChange(74, extracCC);
      }
      oldextrac = extracCC;
    }
    if ((extraCT2 ) && CC2sw){ //Send extra controller CC2
      extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac2) {
        midiSendControlChange(extraCT2, extracCC);
      }
      oldextrac2 = extracCC;
    }
  } else if (extracIsOn) { // we have just gone below threshold, so send zero value
    extracIsOn = 0;
    if ((extraCT == 1) && CC1sw) { //MW
      if (oldextrac != 0) {
        //send modulation 0
        midiSendControlChange(1, 0);
        oldextrac = 0;
      }
    } else if ((extraCT == 2) && CC1sw) { //FP
      if (oldextrac != 0) {
        //send foot pedal 0
        midiSendControlChange(4, 0);
        oldextrac = 0;
      }
    } else if ((extraCT == 3) && (breathCC != 9) && CC1sw) { //CF
      if (oldextrac != 0) {
        //send filter cutoff 0
        midiSendControlChange(74, 0);
        oldextrac = 0;
      }
    } else if ((extraCT == 4) && CC1sw) { //SP
      //send sustain off
      midiSendControlChange(64, 0);
    }
    if ((extraCT2 ) && CC2sw){ //CC2
      //send 0 for extra ctr CC2
      midiSendControlChange(extraCT2, 0);
      oldextrac2 = 0;
    }
  }
}

//***********************************************************

void portamento_() {
  int portSumCC = 0;
  if (pinkySetting == GLD){
    if (portamento && pinkyKey){
      portSumCC += portLimit;
    }
  }
  if (2 == biteControl) {
    // Portamento is controlled with the bite sensor in the mouthpiece
    if (biteJumper) { //PBITE (if pulled low with jumper or if on a NuRAD, use pressure sensor instead of capacitive bite sensor)
      biteSensor=analogRead(bitePressurePin); // alternative kind bite sensor (air pressure tube and sensor)  PBITE
    } else {
      biteSensor = touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right)
    }
    if (portamento && (biteSensor >= portamThrVal)) { // if we are enabled and over the threshold, send portamento
      portSumCC += map(constrain(biteSensor, portamThrVal, portamMaxVal), portamThrVal, portamMaxVal, 0, portLimit);
    }
  }
  if (2 == leverControl) {
    // Portamento is controlled with thumb lever
    leverPortRead = touchRead(vibratoPin);
#if defined(SEAMUS)
    if (portamento && ((leverPortRead) >= leverThrVal)) { // if we are enabled and over the threshold, send portamento
      portSumCC += map(constrain((leverPortRead), leverThrVal, leverMaxVal), leverThrVal, leverMaxVal, 0, portLimit);
    }
#else
    if (portamento && ((3000-leverPortRead) >= leverThrVal)) { // if we are enabled and over the threshold, send portamento
      portSumCC += map(constrain((3000-leverPortRead), leverThrVal, leverMaxVal), leverThrVal, leverMaxVal, 0, portLimit);
    }
#endif
  }
  portSumCC = constrain(portSumCC, 0, portLimit); // Total output glide rate limited to glide max setting
  if (portSumCC) { // there is a portamento level, so go for it
    if (!portIsOn) {
      portOn();
    }
    port(portSumCC);
  }else if (portIsOn) {
    portOff();
  }
}

//***********************************************************

void portOn() {
  if ((portamento == 2) || (portamento == 5)) { // if portamento midi switching is enabled
    midiSendControlChange(CCN_PortOnOff, 127);
  } else if (portamento == 3) { // if portamento midi switching is enabled - SE02 OFF/LIN
    midiSendControlChange(CCN_PortSE02, 64);
  } else if (portamento == 4) { // if portamento midi switching is enabled - SE02 OFF/EXP
    midiSendControlChange(CCN_PortSE02, 127);
  }
  portIsOn = 1;
}

//***********************************************************

void port(int portCC) {
  if ((portamento != 5) && (portCC != oldport)) { // portamento setting 5 is switch only, do not transmit glide rate
    midiSendControlChange(CCN_Port, portCC);
  }
  oldport = portCC;
}

//***********************************************************

void portOff() {
  if ((portamento != 5) && (oldport != 0)) { //did a zero get sent? if not, then send one (unless portamento is switch only)
    midiSendControlChange(CCN_Port, 0);
  }
  if ((portamento == 2) || (portamento == 5)) { // if portamento midi switching is enabled
    midiSendControlChange(CCN_PortOnOff, 0);
  } else if (portamento == 3) { // if portamento midi switching is enabled - SE02 OFF/LIN
    midiSendControlChange(CCN_PortSE02, 0);
  } else if (portamento == 4) { // if portamento midi switching is enabled - SE02 OFF/EXP
    midiSendControlChange(CCN_PortSE02, 0);
  }
  portIsOn = 0;
  oldport = 0;
}

//***********************************************************

void biteCC_() {
  int biteCClevel = 0;
  if (3 == biteControl){
    if (biteJumper) { //PBITE (if pulled low with jumper or if on a NuRAD, use pressure sensor instead of capacitive bite sensor)
      biteSensor=analogRead(bitePressurePin); // alternative kind bite sensor (air pressure tube and sensor)  PBITE
    } else {
      biteSensor = touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right)
    }
    if (biteSensor >= portamThrVal) { // we are over the threshold, calculate CC value
      biteCClevel = map(constrain(biteSensor, portamThrVal, portamMaxVal), portamThrVal, portamMaxVal, 0, 127);
    }
    if (biteCClevel) { // there is a bite CC level, so go for it
      if (!biteIsOn) {
        biteIsOn = 1;
      }
      if (biteCClevel != oldbitecc) {
        midiSendControlChange(biteCC, biteCClevel);
      }
      oldbitecc = biteCClevel;
    } else if (biteIsOn) {
      midiSendControlChange(biteCC, 0);
      biteIsOn = 0;
      oldbitecc = 0;
    }
  }
}

void leverCC_() {
  int leverCClevel = 0;
  if (3 == leverControl){
    leverPortRead = touchRead(vibratoPin);
    if (((3000-leverPortRead) >= leverThrVal)) { // we are over the threshold, calculate CC value
      leverCClevel = map(constrain((3000-leverPortRead), leverThrVal, leverMaxVal), leverThrVal, leverMaxVal, 0, 127);
    }
    if (leverCClevel) { // there is a lever CC level, so go for it
      if (!leverIsOn) {
        leverIsOn = 1;
      }
      if (leverCClevel != oldlevercc) {
        midiSendControlChange(leverCC, leverCClevel);
      }
      oldlevercc = leverCClevel;
    } else if (leverIsOn) {
      midiSendControlChange(leverCC, 0);
      leverIsOn = 0;
      oldlevercc = 0;
    }
  }
}

void autoCal() {
  int calRead;
  int calReadNext;
// NuRAD/NuEVI sensor calibration
  // Extra Controller
  calRead = touchRead(extraPin);
  extracThrVal = constrain(calRead+200, extracLoLimit, extracHiLimit);
  extracMaxVal = constrain(extracThrVal+600, extracLoLimit, extracHiLimit);
  writeSetting(EXTRAC_THR_ADDR, extracThrVal);
  writeSetting(EXTRAC_MAX_ADDR, extracMaxVal);
  // Breath sensor
  calRead = analogRead(breathSensorPin);
  breathThrVal = constrain(calRead+200, breathLoLimit, breathHiLimit);
  breathMaxVal = constrain(breathThrVal+1500, breathLoLimit, breathHiLimit);
  writeSetting(BREATH_THR_ADDR, breathThrVal);
  writeSetting(BREATH_MAX_ADDR, breathMaxVal);
  // Pitch Bend
  calRead = touchRead(pbUpPin);
  calReadNext = touchRead(pbDnPin);
  if (calReadNext > calRead) calRead = calReadNext; //use highest value
  pitchbThrVal = constrain(calRead+200, pitchbLoLimit, pitchbHiLimit);
  pitchbMaxVal = constrain(pitchbThrVal+800, pitchbLoLimit, pitchbHiLimit);
  writeSetting(PITCHB_THR_ADDR, pitchbThrVal);
  writeSetting(PITCHB_MAX_ADDR, pitchbMaxVal);
  // Lever
  calRead = 3000-touchRead(vibratoPin);
  leverThrVal = constrain(calRead+60, leverLoLimit, leverHiLimit);
  leverMaxVal = constrain(calRead+120, leverLoLimit, leverHiLimit);
  writeSetting(LEVER_THR_ADDR, leverThrVal);
  writeSetting(LEVER_MAX_ADDR, leverMaxVal);
#if defined(NURAD) // NuRAD sensor calibration
  // Bite Pressure sensor
  calRead = analogRead(bitePressurePin);
  portamThrVal = constrain(calRead+300, portamLoLimit, portamHiLimit);
  portamMaxVal = constrain(portamThrVal+600, portamLoLimit, portamHiLimit);
  writeSetting(PORTAM_THR_ADDR, portamThrVal);
  writeSetting(PORTAM_MAX_ADDR, portamMaxVal);
  // Touch sensors
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
#else // NuEVI sensor calibration
  // Bite sensor
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
  // Touch sensors
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
#endif
}


//***********************************************************

void readSwitches() {

#if defined(NURAD)

  switch (lap){
    case 0:
      // Octave rollers
      int touchValueRollers[12];
      for (byte i=0; i<6; i++){
        //touchValueRollers[i]=touchSensorRollers.filteredData(i) - calOffsetRollers[i];
        touchValueRollers[i]=touchSensorRollers.filteredData(i) * (300-calOffsetRollers[i])/300;
      }
 #if defined(SEAMUS)
        /*
        // 5-pin version
        octaveR = 0;
        if ((R5=(touchValueRollers[rPin5] < ctouchThrVal)) && ((touchValueRollers[rPin1] < ctouchThrVal))) octaveR = 6; //R6 = R5 && R1
        else if (R5=(touchValueRollers[rPin5] < ctouchThrVal)) octaveR = 5; //R5
        else if (R4=(touchValueRollers[rPin4] < ctouchThrVal)) octaveR = 4; //R4
        else if ((R3=(touchValueRollers[rPin3] < ctouchThrVal)) && lastOctaveR) octaveR = 3; //R3
        else if (R2=(touchValueRollers[rPin2] < ctouchThrVal)) octaveR = 2; //R2
        else if (touchValueRollers[rPin1] < ctouchThrVal) octaveR = 1; //R1
        else if (lastOctaveR > 1) {
          octaveR = lastOctaveR;
          if (otfKey && polySelect && (polySelect<RT1) && rotatorOn && (mainState == NOTE_OFF)) hmzKey = fingeredNote%12;
          if (mainState == NOTE_OFF) currentRotation = 3; //rotator reset by releasing rollers
        }
  //if rollers are released and we are not coming down from roller 1, stay at the higher octave
  //CV filter leak prevention when putting NuEVI aside
      */
      // 5-pin version, 1 & 6 common

      R1=(touchValueRollers[rPin1] < ctouchThrVal);
      R2=(touchValueRollers[rPin2] < ctouchThrVal);
      R3=(touchValueRollers[rPin3] < ctouchThrVal);
      R4=(touchValueRollers[rPin4] < ctouchThrVal);
      R5=(touchValueRollers[rPin5] < ctouchThrVal);
      rSum = R1+R2+R3+R4+R5;

      octaveR = 0;
      oneroll = (rollerMode < 2);
      if (R5 && R1) octaveR = 6; //R6 = R5 && R1
      else if (R5 && (R4 || oneroll)) octaveR = 5; //R5
      else if (R4 && (R3 || oneroll)) octaveR = 4; //R4
      else if (R3 && (R2 || oneroll)) octaveR = 3; //R3
      else if (R2 && (R1 || oneroll)) octaveR = 2; //R2
      else if (R1) octaveR = 1; //R1
      else if (lastOctaveR > 1) {
        if (rollerMode) octaveR = lastOctaveR; //if rollers are released and we are not coming down from roller 1, stay at the higher octave
        if (otfKey && !rSum && polySelect && (polySelect<PolySelect::ERotatorA) && rotatorOn && (mainState == NOTE_OFF)) hmzKey = fingeredNote%12;
        if (mainState == NOTE_OFF) currentRotation = 3; //rotator reset by releasing rollers
      }
      if ((3 == rollerMode) && R1 && !R5 && (6 == lastOctaveR)) octaveR = 7; // Bonus octave on top
      // Roller modes
      // 0: Highest touched roller, no release memory (legacy style), 1 in menu
      // 1: Highest touched roller, release memory, 2 in menu
      // 2: Touched roller pair, release memory, 3 in menu
      // 3: Touched roller pair, release memory, bonus octave on top, 4 in menu

  #else
     // 6-pin version, NuRAD

      R1=(touchValueRollers[rPin1] < ctouchThrVal);
      R2=(touchValueRollers[rPin2] < ctouchThrVal);
      R3=(touchValueRollers[rPin3] < ctouchThrVal);
      R4=(touchValueRollers[rPin4] < ctouchThrVal);
      R5=(touchValueRollers[rPin5] < ctouchThrVal);
      R6=(touchValueRollers[rPin6] < ctouchThrVal);
      rSum = R1+R2+R3+R4+R5+R6;

      octaveR = 0;
      oneroll = (rollerMode < 2);
      if      (R6 && (R5 || oneroll)) octaveR = 6;  //R6
      else if (R5 && (R4 || oneroll)) octaveR = 5;  //R5
      else if (R4 && (R3 || oneroll)) octaveR = 4;  //R4
      else if (R3 && (R2 || oneroll)) octaveR = 3;  //R3
      else if (R2 && (R1 || oneroll)) octaveR = 2;  //R2
      else if (R1) octaveR = 1;  //R1
      else if (lastOctaveR > 1) {
        if (rollerMode) octaveR = lastOctaveR; //if rollers are released and we are not coming down from roller 1, stay at the higher octave
        if (otfKey && !rSum && polySelect && (polySelect<RT1) && rotatorOn && (mainState == NOTE_OFF)) hmzKey = fingeredNote%12;
        if (mainState == NOTE_OFF) currentRotation = 3; //rotator reset by releasing rollers
      }
      if ((3 == rollerMode) && R6 && !R5 && (6 == lastOctaveR)) octaveR = 7; // Bonus octave on top
      // Roller modes
      // 0: Highest touched roller, no release memory (legacy style), 1 in menu
      // 1: Highest touched roller, release memory, 2 in menu
      // 2: Touched roller pair, release memory, 3 in menu
      // 3: Touched roller pair, release memory, bonus octave on top, 4 in menu
#endif
      lastOctaveR = octaveR;

      break;
    case 1:
      // RH keys
      int touchValueRH[12];
      for (byte i=0; i<12; i++){
        //touchValueRH[i]=touchSensorRH.filteredData(i) - calOffsetRH[i];
        touchValueRH[i]=touchSensorRH.filteredData(i) * (300-calOffsetRH[i])/300;
      }
      RHs=(touchValueRH[RHsPin] < ctouchThrVal);
      RH1=(touchValueRH[RH1Pin] < ctouchThrVal);
      RH2=(touchValueRH[RH2Pin] < ctouchThrVal);
      RH3=(touchValueRH[RH3Pin] < ctouchThrVal);
      RHp1=(touchValueRH[RHp1Pin] < ctouchThrVal);
      RHp2=(touchValueRH[RHp2Pin] < ctouchThrVal);
      RHp3=(touchValueRH[RHp3Pin] < ctouchThrVal);
#if defined(SEAMUS)
      specialKey=(touchValueRH[spec1Pin] < ctouchThrVal);
#else
      specialKey=(touchValueRH[spec1Pin] < ctouchThrVal) && (touchValueRH[spec2Pin] < ctouchThrVal);
#endif
      patchKey=(touchValueRH[patchPin] < ctouchThrVal);
      break;
    case 2:
      // LH keys
      int touchValueLH[12];
      for (byte i=0; i<12; i++){
        //touchValueLH[i]=touchSensorLH.filteredData(i) - calOffsetLH[i];
        touchValueLH[i]=touchSensorLH.filteredData(i) * (300-calOffsetLH[i])/300;
      }
      LHs=(touchValueLH[LHsPin] < ctouchThrVal);
      LHb=(touchValueLH[LHbPin] < ctouchThrVal);
      LH1=(touchValueLH[LH1Pin] < ctouchThrVal);
      LH2=(touchValueLH[LH2Pin] < ctouchThrVal);
      LH3=(touchValueLH[LH3Pin] < ctouchThrVal);
      LHp1=(touchValueLH[LHp1Pin] < ctouchThrVal);
      LHp2=(touchValueLH[LHp2Pin] < ctouchThrVal);
      LHp3=(touchValueLH[LHp3Pin] < ctouchThrVal);
  }
  if (lap<2) lap++; else lap=0;

#if defined(SEAMUS)
  K1=RH1;
  K2=RH2;
  K3=RH3;
  K4=LHp1;
  K5=RHs;
  K6=LHp2;
  K7=RHp3;
#else
  K1=RHp2;
  K2=LHp2;
  K3=LHp3;
  K4=LHp1;
  K5=RHp1;
  K6=RHp2;
  K7=RHp3;
#endif
  pinkyKey = LHs || ((lpinky3==MOD) && LHp3);

  int qTransp = ((pinkyKey && (pinkySetting < 25)) ? pinkySetting-12 : 0) + ((LHp3 && lpinky3) ? lpinky3-13 : 0);


  // Calculate midi note number from pressed keys

  if (0==fingering){ //EWI standard fingering
    //fingeredNote=startNote+1-2*LH1-(LHb && !(LH1 && LH2))-LH2-(LH2 && LH1)-2*LH3+LHp1-LHp2+(RHs && !LHp1)-RH1-(RH1 && LH3)-RH2-2*RH3+RHp1-RHp2-2*RHp3+octaveR*12+(octave-3)*12+transpose-12+qTransp;

    fingeredNoteUntransposed=startNote+1-2*LH1-(LHb && !(LH1 && LH2))-LH2-(LH2 && LH1)-2*LH3+LHp1-LHp2+(RHs && !LHp1)-RH1-(RH1 && LH3)-RH2-2*RH3+RHp1-RHp2-2*RHp3+octaveR*12;
  } else if (1==fingering) { //EWX extended EWI fingering - lift LH1 for extended range up, touch RHp3 for extended range down
    fingeredNoteUntransposed=startNote+1-2*LH1-(LHb && !(LH1 && LH2))-LH2-(LH2 && LH1)-2*LH3+LHp1-LHp2+(RHs && !LHp1)-RH1-(RH1 && LH3)-RH2-2*RH3+RHp1-RHp2-2*RHp3+9*(!LH1 && LH2 && LH3)-10*(!RH3 && RHp3)+octaveR*12;
  } else if (2==fingering) { //Sax fingering
    saxFinger[0] = LH1;
    saxFinger[1] = LHb;
    saxFinger[2] = LH2;
    saxFinger[3] = LH3;
    saxFinger[4] = LHp1;
    saxFinger[5] = RH1;
    saxFinger[6] = RH2;
    saxFinger[7] = RH3;
    saxFinger[8] = RHp1;
    saxFinger[9] = RHp3;

    byte matched = 0;
    byte combo = 0;

    while (matched<10 && combo<16)
    {
      combo++;
      matched = 0;
      for (byte finger=0; finger < 10; finger++)
      {
        if ((saxFinger[finger] == saxFingerMatch[combo-1][finger]) || (saxFingerMatch[combo-1][finger] == 2)) matched++;
      }
    }
    if (matched<11 && combo==17) fingeredNoteUntransposed=lastFingering; else fingeredNoteUntransposed = startNote+1+saxFingerResult[combo-1]-LHp2+RHs-(RHp2 && (1 == combo) && LHp2)+octaveR*12;
  } else if (3==fingering) { // EVI fingering

      fingeredNoteUntransposed = startNote
      - 2*RH1 - RH2 - 3*RH3  //"Trumpet valves"
      - 5*LH1              //Fifth key
      + 2*RHs + 4*RHp3  //Trill keys +2 and +4
      + (!LH2 || !LH3 || LHp2) // Trill +1 achieved by lifting finger from LH2 or LH3, or touching LHp2
      + octaveR*12;       //Octave rollers

      /*
      //Evan special fingering test
      fingeredNoteUntransposed = startNote
      - 2*RH1 - RH2 - 3*RH3  //"Trumpet valves"
      - 5*LH1              //Fifth key
      - 12*LH3 - 24*LH2  //Octaves on LH2 and LH3
      + octaveR*12;       //Octave rollers
      */
  } else { // EVI fingering with reversed octave rollers
      fingeredNoteUntransposed = startNote
      - 2*RH1 - RH2 - 3*RH3  //"Trumpet valves"
      - 5*LH1              //Fifth key
      + 2*RHs + 4*RHp3  //Trill keys +2 and +4
      + (!LH2 || !LH3 || LHp2) // Trill +1 achieved by lifting finger from LH2 or LH3, or touching LHp2
      + (6-octaveR)*12;       //Octave rollers, reversed
  }

  int fingeredNoteRead = fingeredNoteUntransposed + (octave - 3) * 12 + transpose - 12 + qTransp + harmonicResult[harmSelect][harmonics] + brHarmonicResult[brHarmSelect][brHarmonics]; //lip sensor and breath harmonics

  if (pinkyKey) pitchlatch = fingeredNoteUntransposed;  //use pitchlatch to make settings based on note fingered

#else //NuEVI

  // Read touch pads (MPR121), compare against threshold value
  bool touchKeys[12];
  for (byte i = 0; i < 12; i++) {
    touchKeys[i] = touchSensor.filteredData(i) < ctouchThrVal;
  }

  // Octave rollers

  R1 = touchKeys[R1Pin];
  R2 = touchKeys[R2Pin];
  R3 = touchKeys[R3Pin];
  R4 = touchKeys[R4Pin];
  R5 = touchKeys[R5Pin];
  rSum = R1+R2+R3+R4+R5;

  octaveR = 0;
  oneroll = (rollerMode < 2);
  if (R5 && R3) octaveR = 6; //R6 = R5 && R3
  else if (R5 && (R4 || oneroll)) octaveR = 5; //R5
  else if (R4 && (R3 || oneroll)) octaveR = 4; //R4
  else if (R3 && (R2 || (oneroll && lastOctaveR))) octaveR = 3; //R3
  else if (R2 && (R1 || oneroll)) octaveR = 2; //R2
  else if (R1) octaveR = 1; //R1
  else if (lastOctaveR > 1) {
    if (rollerMode) octaveR = lastOctaveR; //if rollers are released and we are not coming down from roller 1, stay at the higher octave (CV filter leak prevention when putting NuEVI aside)
    if (otfKey && !rSum && polySelect && (polySelect<PolySelect::ERotatorA) && rotatorOn && (mainState == NOTE_OFF)) hmzKey = fingeredNote%12;
    if (mainState == NOTE_OFF) currentRotation = 3; //rotator reset by releasing rollers
  }
  if ((3 == rollerMode) && R3 && !R5 && (6 == lastOctaveR)) octaveR = 7; // Bonus octave on top
  // Roller modes
  // 0: Highest touched roller, no release memory (legacy style), 1 in menu
  // 1: Highest touched roller, release memory, 2 in menu
  // 2: Touched roller pair, release memory, 3 in menu
  // 3: Touched roller pair, release memory, bonus octave on top, 4 in menu

  lastOctaveR = octaveR;

  // Valves and trill keys
  K1 = touchKeys[K1Pin];
  K2 = touchKeys[K2Pin];
  K3 = touchKeys[K3Pin];
  K4 = touchKeys[K4Pin];
  K5 = touchKeys[K5Pin];
  K6 = touchKeys[K6Pin];
  K7 = touchKeys[K7Pin];

  pinkyKey = (touchRead(halfPitchBendKeyPin) > touch_Thr); // SENSOR PIN 1  - PCB PIN "S1"


  int qTransp = (pinkyKey && (pinkySetting < 25)) ? pinkySetting-12 : 0;

  // Calculate midi note number from pressed keys
  if (0 == fingering){ //EVI fingering
    fingeredNoteUntransposed = startNote
      - 2*K1 - K2 - 3*K3  //"Trumpet valves"
      - 5*K4              //Fifth key
      + 2*K5 + K6 + trill3_interval*K7  //Trill keys. 3rd trill key interval controlled by setting
      + octaveR*12;       //Octave rollers
  } else if (1 == fingering){ //EVR fingering
    byte revOct = 6 - octaveR;
    fingeredNoteUntransposed = startNote
      - 2*K1 - K2 - 3*K3  //"Trumpet valves"
      - 5*K4              //Fifth key
      + 2*K5 + K6 + trill3_interval*K7  //Trill keys. 3rd trill key interval controlled by setting
      + revOct*12;       //Octave rollers
  } else if (2 == fingering){ //TPT fingering
        fingeredNoteUntransposed = startNote
      - 2*K1 - K2 - 3*K3  //"Trumpet valves"
      - 2                 //Trumpet in B flat
      + 2*K5 + K6 + trill3_interval*K7  //Trill keys. 3rd trill key interval controlled by setting
      + 24 + trumpetHarmonic[K4][octaveR]; // roller harmonics
  } else if (3 == fingering){ //HRN fingering
        fingeredNoteUntransposed = startNote
      - 2*K1 - K2 - 3*K3  //"Trumpet valves"
      + 5*K4              //Switch to Bb horn
      + 5                 //Horn in F
      + 2*K5 + K6 + trill3_interval*K7  //Trill keys. 3rd trill key interval controlled by setting
      + 12 + rollerHarmonic[K4][octaveR]; // roller harmonics
  }



  if (K3 && K7){
    if (4 == trill3_interval) fingeredNoteUntransposed+=2; else fingeredNoteUntransposed+=4;
  }

  int fingeredNoteRead = fingeredNoteUntransposed + (octave - 3) * 12 + transpose - 12 + qTransp + harmonicResult[harmSelect][harmonics] + brHarmonicResult[brHarmSelect][brHarmonics]; //lip sensor harmonics

  pcCombo1 = (K1 && K5 && !K2 && !K3);
  pcCombo2 = (K2 && K6 && !K1 && !K3);

  if (pinkyKey) pitchlatch = fingeredNoteUntransposed; //use pitchlatch to make settings based on note fingered

#endif

  if (fingeredNoteRead != lastFingering) { //
    // reset the debouncing timer
    lastDeglitchTime = millis();
  }
  if ((millis() - lastDeglitchTime) > deglitch) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state
    fingeredNote = fingeredNoteRead;
  }
  lastFingering = fingeredNoteRead;
}
