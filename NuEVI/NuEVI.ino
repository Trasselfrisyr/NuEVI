#include <Wire.h>

#include <Adafruit_MPR121.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Filters.h>  // for the breath signal LP filtering, https://github.com/edgar-bonet/Filters

#include "globals.h"
#include "hardware.h"
#include "midi.h"
#include "menu.h"
#include "config.h"
#include "settings.h"

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


// The three states of our main state machine

// No note is sounding
#define NOTE_OFF 1

// We've observed a transition from below to above the
// threshold value. We wait a while to see how fast the
// breath velocity is increasing
#define RISE_WAIT 2

// A note is sounding
#define NOTE_ON 3



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
unsigned short transpose;
unsigned short MIDIchannel;
unsigned short breathCC;  // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+:CF
unsigned short breathAT;
unsigned short velocity;
unsigned short portamento;// switching on cc65? just cc5 enabled? SW:ON:OFF
unsigned short PBdepth;   // OFF:1-12 divider
unsigned short extraCT;   // OFF:MW:FP:CF:SP
unsigned short vibrato;   // OFF:1-9
unsigned short deglitch;  // 0-70 ms in steps of 5
unsigned short patch;     // 1-128
unsigned short octave;
unsigned short curve;
unsigned short velSmpDl;  // 0-30 ms
unsigned short velBias;   // 0-9
unsigned short pinkySetting; // 0 - 11 (QuickTranspose -12 to -1), 12 (pb/2), 13 - 24 (QuickTranspose +1 to +12)
unsigned short dipSwBits; // virtual dip switch settings for special modes (work in progress)
unsigned short priority; // mono priority for rotator chords

unsigned short vibSens = 2; // vibrato sensitivity
unsigned short vibRetn = 2; // vibrato return speed
unsigned short vibSquelch = 15; //vibrato signal squelch
unsigned short vibDirection = DNWD; //direction of first vibrato wave UPWD or DNWD

unsigned short fastPatch[7] = {0,0,0,0,0,0,0};

byte rotatorOn = 0;
byte currentRotation = 0;
int rotations[4] = { -5, -10, -7, -14 }; // semitones { -5, -10, -7, -14 };
int parallel = 7; // semitones

byte gateOpen = 0; // setting for gate always open, note on sent for every time fingering changes, no matter the breath status

int breathLoLimit = 0;
int breathHiLimit = 4095;
int portamLoLimit = 700;
int portamHiLimit = 4700;
int pitchbLoLimit = 500;
int pitchbHiLimit = 4000;
int extracLoLimit = 500;
int extracHiLimit = 4000;
int ctouchLoLimit = 50;
int ctouchHiLimit = 350;
int ttouchLoLimit = 50;
int ttouchHiLimit = 1900;

int touch_Thr = 1300;


int breathStep;
int portamStep;
int pitchbStep;
int extracStep;
int ctouchStep;

byte ccList[11] = {0,1,2,7,11,1,2,7,11,74,20};  // OFF, Modulation, Breath, Volume, Expression (then same sent in hires), CC74 (cutoff/brightness), CC20

int pbDepthList[13] = {8192,8192,4096,2731,2048,1638,1365,1170,1024,910,819,744,683};


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

static unsigned long pixelUpdateTime = 0;
static const unsigned long pixelUpdateInterval = 80;

unsigned long lastDeglitchTime = 0;         // The last time the fingering was changed
unsigned long ccSendTime = 0L;              // The last time we sent CC values
unsigned long breath_on_time = 0L;          // Time when breath sensor value went over the ON threshold

int lastFingering = 0;             // Keep the last fingering value for debouncing

int mainState;                         // The state of the main state machine

int initial_breath_value;          // The breath value at the time we observed the transition

byte activeMIDIchannel;          // MIDI channel
byte activePatch=0;
byte doPatchUpdate=0;

byte legacy = 0;
byte legacyBrAct = 0;
byte halfTime = 0;
boolean programonce = false;
byte slowMidi = 0;

int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;
unsigned int oldbreathhires=0;
float filterFreq = 30.0;

float filterVal = 0.15;
float smoothedVal;
int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
int lastPressure;
byte velocitySend;   // remapped midi velocity from breath sensor (or set to static value if selected)
int breathCalZero;


int biteSensor=0;    // capacitance data from bite sensor, for midi cc and threshold checks
byte portIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
int oldport=0;
int lastBite=0;
byte biteJumper=0;

int cvPitch;
int targetPitch;

int exSensor=0;
byte extracIsOn=0;
int oldextrac=0;
int lastEx=0;

int pitchBend=8192;
int oldpb=8192;
int vibSignal=0;
int pbUp=0;
int pbDn=0;
byte vibLedOff = 0;
byte oldpkey = 0;

static const float vibDepth[10] = {0,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.40,0.45}; // max pitch bend values (+/-) for the vibrato settings
static const short vibMaxList[12] = {300,275,250,225,200,175,150,125,100,75,50,25};

static const unsigned short curveM4[] = {0,4300,7000,8700,9900,10950,11900,12600,13300,13900,14500,15000,15450,15700,16000,16250,16383};
static const unsigned short curveM3[] = {0,2900,5100,6650,8200,9500,10550,11500,12300,13100,13800,14450,14950,15350,15750,16150,16383};
static const unsigned short curveM2[] = {0,2000,3600,5000,6450,7850,9000,10100,11100,12100,12900,13700,14400,14950,15500,16000,16383};
static const unsigned short curveM1[] = {0,1400,2850,4100,5300,6450,7600,8700,9800,10750,11650,12600,13350,14150,14950,15650,16383};
static const unsigned short curveIn[] = {0,1023,2047,3071,4095,5119,6143,7167,8191,9215,10239,11263,12287,13311,14335,15359,16383};
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

int vibThr;          // this gets auto calibrated in setup
int vibThrLo;
int vibZero;


int fingeredNote;    // note calculated from fingering (switches), transpose and octave settings
int fingeredNoteUntransposed; // note calculated from fingering (switches), for on the fly settings
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
byte lastOctaveR = 0;

byte halfPitchBendKey;
byte specialKey;
byte pinkyKey;
byte lastSpecialKey = 0;
byte pitchlatch;
int reverb;

byte slurSustain = 0;
byte parallelChord = 0;
byte subOctaveDouble = 0;

const int breathLedBrightness = 500; // up to 4095, PWM
const int portamLedBrightness = 500; // up to 4095, PWM

Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor
FilterOnePole breathFilter;


//_______________________________________________________________________________________________ SETUP

void setup() {

  analogReadResolution(12);   // set resolution of ADCs to 12 bit
  analogWriteResolution(12);
  analogWriteFrequency(pwmDacPin,11718.75);

  pinMode(dPin, INPUT_PULLUP);
  pinMode(ePin, INPUT_PULLUP);
  pinMode(uPin, INPUT_PULLUP);
  pinMode(mPin, INPUT_PULLUP);

  pinMode(bLedPin, OUTPUT);        // breath indicator LED
  pinMode(pLedPin, OUTPUT);        // portam indicator LED
  pinMode(statusLedPin,OUTPUT);    // Teensy onboard LED
  pinMode(dacPin, OUTPUT);         //DAC output for analog signal
  pinMode(pwmDacPin, OUTPUT);      //PWMed DAC output for analog signal

  pinMode(biteJumperPin, INPUT_PULLUP); //PBITE
  pinMode(biteJumperGndPin, OUTPUT);    //PBITE
  digitalWrite(biteJumperGndPin, LOW);  //PBITE

  // if stored settings are not for current version, or Enter+Menu are pressed at startup, they are replaced by factory settings
  
  if (((readSetting(VERSION_ADDR) != VERSION) && (readSetting(VERSION_ADDR) < 24)) || (!digitalRead(ePin) && !digitalRead(mPin))) {
    writeSetting(VERSION_ADDR,VERSION);
    writeSetting(BREATH_THR_ADDR,BREATH_THR_FACTORY);
    writeSetting(BREATH_MAX_ADDR,BREATH_MAX_FACTORY);
      if (digitalRead(biteJumperPin)){ //PBITE (if pulled low with jumper, pressure sensor on A7 instead of capacitive bite sensing)
        writeSetting(PORTAM_THR_ADDR,PORTAM_THR_FACTORY);  
        writeSetting(PORTAM_MAX_ADDR,PORTAM_MAX_FACTORY); 
      } else {
        writeSetting(PORTAM_THR_ADDR,PORTPR_THR_FACTORY);  
        writeSetting(PORTAM_MAX_ADDR,PORTPR_MAX_FACTORY); 
      }
    writeSetting(PITCHB_THR_ADDR,PITCHB_THR_FACTORY);
    writeSetting(PITCHB_MAX_ADDR,PITCHB_MAX_FACTORY);
    writeSetting(EXTRAC_THR_ADDR,EXTRAC_THR_FACTORY);
    writeSetting(EXTRAC_MAX_ADDR,EXTRAC_MAX_FACTORY);
    writeSetting(CTOUCH_THR_ADDR,CTOUCH_THR_FACTORY);
  }
  
  if ((readSetting(VERSION_ADDR) != VERSION) || (!digitalRead(ePin) && !digitalRead(mPin))) {
    writeSetting(VERSION_ADDR,VERSION);
    
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
    writeSetting(PATCH_ADDR,PATCH_FACTORY);
    writeSetting(OCTAVE_ADDR,OCTAVE_FACTORY);
    writeSetting(BREATHCURVE_ADDR,BREATHCURVE_FACTORY);
    writeSetting(VEL_SMP_DL_ADDR,VEL_SMP_DL_FACTORY);
    writeSetting(VEL_BIAS_ADDR,VEL_BIAS_FACTORY);
    writeSetting(PINKY_KEY_ADDR,PINKY_KEY_FACTORY);
    writeSetting(FP1_ADDR,0);
    writeSetting(FP2_ADDR,0);
    writeSetting(FP3_ADDR,0);
    writeSetting(FP4_ADDR,0);
    writeSetting(FP5_ADDR,0);
    writeSetting(FP6_ADDR,0);
    writeSetting(FP7_ADDR,0);
    writeSetting(DIPSW_BITS_ADDR,DIPSW_BITS_FACTORY);
    writeSetting(PARAL_ADDR,PARAL_FACTORY);
    writeSetting(ROTN1_ADDR,ROTN1_FACTORY);
    writeSetting(ROTN2_ADDR,ROTN2_FACTORY);
    writeSetting(ROTN3_ADDR,ROTN3_FACTORY);
    writeSetting(ROTN4_ADDR,ROTN4_FACTORY);
    writeSetting(PRIO_ADDR,PRIO_FACTORY);
    writeSetting(VIB_SENS_ADDR,VIB_SENS_FACTORY);
    writeSetting(VIB_RETN_ADDR,VIB_RETN_FACTORY);
    writeSetting(VIB_SQUELCH_ADDR,VIB_SQUELCH_FACTORY);
    writeSetting(VIB_DIRECTION_ADDR,VIB_DIRECTION_FACTORY);
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
  velSmpDl     = readSetting(VEL_SMP_DL_ADDR);
  velBias      = readSetting(VEL_BIAS_ADDR);
  pinkySetting = readSetting(PINKY_KEY_ADDR);
  fastPatch[0] = readSetting(FP1_ADDR);
  fastPatch[1] = readSetting(FP2_ADDR);
  fastPatch[2] = readSetting(FP3_ADDR);
  fastPatch[3] = readSetting(FP4_ADDR);
  fastPatch[4] = readSetting(FP5_ADDR);
  fastPatch[5] = readSetting(FP6_ADDR);
  fastPatch[6] = readSetting(FP7_ADDR);
  dipSwBits    = readSetting(DIPSW_BITS_ADDR);
  parallel     = readSetting(PARAL_ADDR)-24;
  rotations[0] = readSetting(ROTN1_ADDR)-24;
  rotations[1] = readSetting(ROTN2_ADDR)-24;
  rotations[2] = readSetting(ROTN3_ADDR)-24;
  rotations[3] = readSetting(ROTN4_ADDR)-24;
  priority     = readSetting(PRIO_ADDR);
  vibSens      = readSetting(VIB_SENS_ADDR);
  vibRetn      = readSetting(VIB_RETN_ADDR);
  vibSquelch   = readSetting(VIB_SQUELCH_ADDR);
  vibDirection = readSetting(VIB_DIRECTION_ADDR);

  legacy = dipSwBits & (1<<1);
  legacyBrAct = dipSwBits & (1<<2);
  slowMidi = dipSwBits & (1<<3);
  activePatch = patch;

  breathStep = (breathHiLimit - breathLoLimit)/92; // 92 is the number of pixels in the settings bar
  portamStep = (portamHiLimit - portamLoLimit)/92;
  pitchbStep = (pitchbHiLimit - pitchbLoLimit)/92;
  extracStep = (extracHiLimit - extracLoLimit)/92;
  ctouchStep = (ctouchHiLimit - ctouchLoLimit)/92;

  touch_Thr = map(ctouchThrVal,ctouchHiLimit,ctouchLoLimit,ttouchLoLimit,ttouchHiLimit);

  if (!touchSensor.begin(0x5A)) {
    while (1);  // Touch sensor initialization failed - stop doing stuff
  }

  breathFilter.setFilter(LOWPASS, filterFreq, 0.0);   // create a one pole (RC) lowpass filter
  
  initDisplay(); //Start up display and show logo

  biteJumper = !digitalRead(biteJumperPin);
  if (biteJumper){
    pinMode(bitePin, INPUT);
  }

  //auto-calibrate the vibrato threshold while showing splash screen
  vibZero = breathCalZero = 0;
  const int sampleCount = 4;
  for(int i = 1 ; i <= sampleCount; ++i) {
    vibZero += touchRead(vibratoPin);
    breathCalZero += analogRead(breathSensorPin);
    digitalWrite( statusLedPin, i&1 );
    delay(250);
  }
  vibZero /= sampleCount;
  breathCalZero /= sampleCount;

  vibThr = vibZero - vibSquelch;
  vibThrLo = vibZero + vibSquelch;

  digitalWrite(statusLedPin, LOW);
  delay(250);
  digitalWrite(statusLedPin,HIGH);
  delay(250);
  digitalWrite(statusLedPin,LOW);

  showVersion();

  delay(1500);

  mainState = NOTE_OFF;       // initialize main state machine

  if (!digitalRead(ePin)) {
    activePatch=0;
    doPatchUpdate=1;
  }

  activeMIDIchannel = MIDIchannel;
  midiInitialize(MIDIchannel);

  //Serial.begin(9600); // debug

  digitalWrite(statusLedPin,HIGH); // Switch on the onboard LED to indicate power on/ready

}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {
  breathFilter.input(analogRead(breathSensorPin));
  pressureSensor = constrain((int) breathFilter.output(), 0, 4095); // Get the filtered pressure sensor reading from analog pin A0, input from sensor MP3V5004GP 
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
      #if defined(CASSIDY)
      if (((pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) && (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2)) && legacy) ||
          ((analogRead(0) < breathCalZero - 900) && legacyBrAct)) { // both pb pads touched or br suck
      #else
      if (((pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) && (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2)) && legacy) ||
          ((analogRead(0) < breathCalZero - 800) && legacyBrAct && (pbUp > (pitchbMaxVal + pitchbThrVal) / 2) && (pbDn < (pitchbMaxVal + pitchbThrVal) / 2))) { // both pb pads touched or br suck
      #endif  
        readSwitches();
        fingeredNoteUntransposed = patchLimit(fingeredNoteUntransposed + 1);
        if (exSensor >= ((extracThrVal + extracMaxVal) / 2)) { // instant midi setting     
          if ((fingeredNoteUntransposed >= 73) && (fingeredNoteUntransposed <= 88)) {
            MIDIchannel = fingeredNoteUntransposed - 72; // Mid C and up 
            #if !defined(CASSIDY)
            digitalWrite(statusLedPin, LOW);
            delay(150);
            digitalWrite(statusLedPin, HIGH);
            #endif
          }
        } else {
          if (!pinkyKey) { // note number to patch number
            if (patch != fingeredNoteUntransposed) {
              patch = fingeredNoteUntransposed;
              doPatchUpdate = 1;
              #if !defined(CASSIDY)
              digitalWrite(statusLedPin, LOW);
              delay(150);
              digitalWrite(statusLedPin, HIGH);
              #endif
            }
          } else { // hi and lo patch numbers
            if (fingeredNoteUntransposed > 75) {
              if (patch != patchLimit(fingeredNoteUntransposed + 24)) {
                patch = patchLimit(fingeredNoteUntransposed + 24); // add 24 to get high numbers 108 to 127
                doPatchUpdate = 1;
                #if !defined(CASSIDY)
                digitalWrite(statusLedPin, LOW);
                delay(150);
                digitalWrite(statusLedPin, HIGH);
                #endif
              }
            } else {
              if (patch != patchLimit(fingeredNoteUntransposed - 36)) {
                patch = patchLimit(fingeredNoteUntransposed - 36); // subtract 36 to get low numbers 0 to 36
                doPatchUpdate = 1;
                #if !defined(CASSIDY)
                digitalWrite(statusLedPin, LOW);
                delay(150);
                digitalWrite(statusLedPin, HIGH);
                #endif
              }
            }
          }
        }
      } else {
        if (pbDn > (pitchbMaxVal + pitchbThrVal) / 2 && (analogRead(0) < (breathCalZero - 800)) && programonce == false) { // down bend for suck programming button
          programonce = true;
          readSwitches();

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

    if (analogRead(0) > (breathCalZero - 800)) programonce = false;

    specialKey = (touchRead(specialKeyPin) > touch_Thr); //S2 on pcb
    if (lastSpecialKey != specialKey) {
      if (specialKey) {
        // special key just pressed, check other keys
        readSwitches();
        if (K4) {
          if (!slurSustain) {
            slurSustain = 1;
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
        if (!K1 && !K4 && !K5) {
          slurSustain = 0;
          parallelChord = 0;
          subOctaveDouble = 0;
          rotatorOn = 0;
        }
        if (pinkyKey) {
          if (!rotatorOn) {
            rotatorOn = 1;
            slurSustain = 0;
            parallelChord = 0;
            subOctaveDouble = 0;
          } else rotatorOn = 0;
        }
      }
    }
    lastSpecialKey = specialKey;
  } else if (mainState == RISE_WAIT) {
    if ((pressureSensor > breathThrVal) || gateOpen) {
      // Has enough time passed for us to collect our second
      // sample?
      if ((millis() - breath_on_time > velSmpDl) || (0 == velSmpDl)) {
        // Yes, so calculate MIDI note and velocity, then send a note on event
        readSwitches();
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
          midiSendNoteOn(noteValueCheck(fingeredNote + parallel), velocitySend); // send Note On message for new note
          if (currentRotation < 3) currentRotation++;
          else currentRotation = 0;
          midiSendNoteOn(noteValueCheck(fingeredNote + rotations[currentRotation]), velocitySend); // send Note On message for new note
        }
        if (!priority) { // mono prio to base note
          midiSendNoteOn(fingeredNote, velocitySend); // send Note On message for new note
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
        midiSendNoteOff(noteValueCheck(activeNote + parallel)); // send Note Off message for old note
        midiSendNoteOff(noteValueCheck(activeNote + rotations[currentRotation])); // send Note Off message for old note
      }
      if (!priority) {
        midiSendNoteOff(activeNote); //  send Note Off message
      }
      if (slurSustain) {
        midiSendControlChange(64, 0);
      }
      breathLevel = 0;
      mainState = NOTE_OFF;
    } else {
      readSwitches();
      if (fingeredNote != lastFingering) { //
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
            midiSendNoteOff(noteValueCheck(activeNote + parallel)); // send Note Off message for old note
            midiSendNoteOff(noteValueCheck(activeNote + rotations[currentRotation])); // send Note Off message for old note
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
            midiSendNoteOn(noteValueCheck(fingeredNote + parallel), velocitySend); // send Note On message for new note
            if (currentRotation < 3) currentRotation++;
            else currentRotation = 0;
            midiSendNoteOn(noteValueCheck(fingeredNote + rotations[currentRotation]), velocitySend); // send Note On message for new note
          }

          if (!priority) {
            midiSendNoteOn(fingeredNote, velocitySend); // send Note On message for new note
          }

          if (!parallelChord && !subOctaveDouble && !rotatorOn) { // mono playing, send old note off after new note on
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
    }
    if (pressureSensor > breathThrVal) cursorBlinkTime = millis(); // keep display from updating with cursor blinking if breath is over thr
  }
  // Is it time to send more CC data?
  if (millis() - ccSendTime > CC_INTERVAL) {
    // deal with Breath, Pitch Bend, Modulation, etc.
    if (!slowMidi) breath();
    halfTime = !halfTime;
    if (halfTime) {
      pitch_bend();
      portamento_();
    } else {
      if (slowMidi) breath();
      extraController();
      statusLEDs();
      doorKnobCheck();
    }
    ccSendTime = millis();
  }
  if (millis() - pixelUpdateTime > pixelUpdateInterval) {
    // even if we just alter a pixel, the whole display is redrawn (35ms of MPU lockup) and we can't do that all the time
    // this is one of the big reasons the display is for setup use only
    drawSensorPixels(); // live sensor monitoring for the setup screens
    if (rotatorOn || slurSustain || parallelChord || subOctaveDouble || gateOpen) {
      digitalWrite(statusLedPin, !digitalRead(statusLedPin));
    } else if (!digitalRead(statusLedPin)) {
      digitalWrite(statusLedPin, HIGH);
    }
    pixelUpdateTime = millis();
  }
  lastFingering = fingeredNote;
  #if defined(CVSCALEBOARD) // pitch CV from DAC and breath CV from PWM on pin 6, for filtering and scaling on separate board
    targetPitch = (fingeredNote-24)*42;
    if (portIsOn){
      if (targetPitch > cvPitch){
        cvPitch += 1+(127-oldport)/4;
        if (cvPitch > targetPitch) cvPitch = targetPitch;
      } else if (targetPitch < cvPitch){
        cvPitch -= 1+(127-oldport)/4;
        if (cvPitch < targetPitch) cvPitch = targetPitch;
      } else {
        cvPitch = targetPitch;
      }
    } else {
      cvPitch = targetPitch;
    }
    analogWrite(dacPin,constrain(cvPitch+map(pitchBend,0,16383,-84,84),0,4095));
    analogWrite(pwmDacPin,breathCurve(map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,500,4095))); //starting at 0.6V to match use of cv from sensor, so recalibration of cv offset/scaler is not needed
  #else // else breath CV on DAC pin, directly to unused pin of MIDI DIN jack
    analogWrite(dacPin,breathCurve(map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,4095)));
  #endif

  midiDiscardInput();

  //do menu stuff
  menu();
}

//_______________________________________________________________________________________________ FUNCTIONS

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
  // 0 to 16383, moving mid value up or down
  switch (curve) {
  case 0:
    // -4
    return multiMap(inputVal, curveIn, curveM4, 17);
    break;
  case 1:
    // -3
    return multiMap(inputVal, curveIn, curveM3, 17);
    break;
  case 2:
    // -2
    return multiMap(inputVal, curveIn, curveM2, 17);
    break;
  case 3:
    // -1
    return multiMap(inputVal, curveIn, curveM1, 17);
    break;
  case 4:
    // 0, linear
    return inputVal;
    break;
  case 5:
    // +1
    return multiMap(inputVal, curveIn, curveP1, 17);
    break;
  case 6:
    // +2
    return multiMap(inputVal, curveIn, curveP2, 17);
    break;
  case 7:
    // +3
    return multiMap(inputVal, curveIn, curveP3, 17);
    break;
  case 8:
    // +4
    return multiMap(inputVal, curveIn, curveP4, 17);
    break;
  case 9:
    // S1
    return multiMap(inputVal, curveIn, curveS1, 17);
    break;
  case 10:
    // S2
    return multiMap(inputVal, curveIn, curveS2, 17);
    break;
  case 11:
    // Z1
    return multiMap(inputVal, curveIn, curveZ1, 17);
    break;
  case 12:
    // Z2
    return multiMap(inputVal, curveIn, curveZ2, 17);
    break;
  default: //Fallback option that should never be reached, use linear
    return inputVal;
    break;
  }
}

//**************************************************************
/*
int smooth(int data, float filterVal, float smoothedVal){


  if (filterVal > 1){      // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}

*/
//**************************************************************

// MIDI note value check with out of range octave repeat
int noteValueCheck(int note) {
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

void statusLEDs() {
  if (breathLevel > breathThrVal) { // breath indicator LED, labeled "B" on PCB
    //analogWrite(bLedPin, map(breathLevel,0,4096,5,breathLedBrightness));
    analogWrite(bLedPin, map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, 5, breathLedBrightness));
  } else {
    analogWrite(bLedPin, 0);
  }
  if (biteSensor > portamThrVal) { // portamento indicator LED, labeled "P" on PCB
    //analogWrite(pLedPin, map(biteSensor,0,4096,5,portamLedBrightness));
    analogWrite(pLedPin, map(constrain(biteSensor, portamThrVal, portamMaxVal), portamThrVal, portamMaxVal, 5, portamLedBrightness));
  } else {
    analogWrite(pLedPin, 0);
  }
}

//**************************************************************

void breath() {
  int breathCCval, breathCCvalFine;
  unsigned int breathCCvalHires;
  breathLevel = constrain(pressureSensor, breathThrVal, breathMaxVal);
  //breathLevel = breathLevel*0.6+pressureSensor*0.4; // smoothing of breathLevel value
  ////////breathCCval = map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,127);
  breathCCvalHires = breathCurve(map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, 0, 16383));
  breathCCval = (breathCCvalHires >> 7) & 0x007F;
  breathCCvalFine = breathCCvalHires & 0x007F;

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
}

//**************************************************************

void pitch_bend() {
  // handle input from pitchbend touchpads and
  // on-pcb variable capacitor for vibrato.
  int vibMax;
  int calculatedPBdepth;
  byte pbTouched = 0;
  pbUp = touchRead(pbUpPin); // SENSOR PIN 23 - PCB PIN "Pu"
  pbDn = touchRead(pbDnPin); // SENSOR PIN 22 - PCB PIN "Pd"
  halfPitchBendKey = (pinkySetting == PBD) && (touchRead(halfPitchBendKeyPin) > touch_Thr); // SENSOR PIN 1  - PCB PIN "S1" - hold for 1/2 pitchbend value
  int vibRead = touchRead(vibratoPin); // SENSOR PIN 15 - built in var cap
  calculatedPBdepth = pbDepthList[PBdepth];
  if (halfPitchBendKey) calculatedPBdepth = calculatedPBdepth * 0.5;

  vibMax = vibMaxList[vibSens - 1];

  if (vibRead < vibThr) {
    if (UPWD == vibDirection) {
      vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibRead, (vibZero - vibMax), vibThr), vibThr, (vibZero - vibMax), 0, calculatedPBdepth * vibDepth[vibrato]);
    } else {
      vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibRead, (vibZero - vibMax), vibThr), vibThr, (vibZero - vibMax), 0, (0 - calculatedPBdepth * vibDepth[vibrato]));
    }
  } else if (vibRead > vibThrLo) {
    if (UPWD == vibDirection) {
      vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibRead, vibThrLo, (vibZero + vibMax)), vibThrLo, (vibZero + vibMax), 0, (0 - calculatedPBdepth * vibDepth[vibrato]));
    } else {
      vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibRead, vibThrLo, (vibZero + vibMax)), vibThrLo, (vibZero + vibMax), 0, calculatedPBdepth * vibDepth[vibrato]);
    }
  } else {
    vibSignal = vibSignal * 0.5;
  }

  switch (vibRetn) { // moving baseline
  case 0:
    //keep vibZero value
    break;
  case 1:
    vibZero = vibZero * 0.95 + vibRead * 0.05;
    break;
  case 2:
    vibZero = vibZero * 0.9 + vibRead * 0.1;
    break;
  case 3:
    vibZero = vibZero * 0.8 + vibRead * 0.2;
    break;
  case 4:
    vibZero = vibZero * 0.6 + vibRead * 0.4;
  }
  vibThr = vibZero - vibSquelch;
  vibThrLo = vibZero + vibSquelch;
  int pbPos = map(constrain(pbUp, pitchbThrVal, pitchbMaxVal), pitchbThrVal, pitchbMaxVal, 0, calculatedPBdepth);
  int pbNeg = map(constrain(pbDn, pitchbThrVal, pitchbMaxVal), pitchbThrVal, pitchbMaxVal, 0, calculatedPBdepth);
  int pbSum = 8193 + pbPos - pbNeg;
  int pbDif = abs(pbPos - pbNeg);
  /*
  if ((pbUp > pitchbThrVal) && PBdepth){
    pitchBend=pitchBend*0.6+0.4*map(constrain(pbUp,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,(8193 + calculatedPBdepth));
    pbTouched++;
  }
  if ((pbDn > pitchbThrVal) && PBdepth){
    pitchBend=pitchBend*0.6+0.4*map(constrain(pbDn,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,(8192 - calculatedPBdepth));
    pbTouched++;
  }
  */
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

  if (subVibSquelch && (8192 != pitchBend)) {
    digitalWrite(statusLedPin, LOW);
    vibLedOff = 1;
  } else if (vibLedOff) {
    digitalWrite(statusLedPin, HIGH);
    vibLedOff = 0;
  }

  //Serial.print(pitchBend);
  //Serial.print(" - ");
  //Serial.println(oldpb);

  if (pitchBend != oldpb) { // only send midi data if pitch bend has changed from previous value
    midiSendPitchBend(pitchBend);
    oldpb = pitchBend;
  }
}

//***********************************************************

void doorKnobCheck() {
  int touchValue[12];
  for (byte i = 0; i < 12; i++) {
    touchValue[i] = touchSensor.filteredData(i);
  }
  if ((touchValue[K4Pin] < ctouchThrVal) && (touchValue[R1Pin] < ctouchThrVal) && (touchValue[R2Pin] < ctouchThrVal) && (touchValue[R3Pin] < ctouchThrVal)) { // doorknob grip on canister
    if (pbUp > ((pitchbMaxVal + pitchbThrVal) / 2)) {
      gateOpen = 1;
      digitalWrite(statusLedPin, LOW);
      delay(50);
      digitalWrite(statusLedPin, HIGH);
      delay(50);
    } else if (pbDn > ((pitchbMaxVal + pitchbThrVal) / 2)) {
      gateOpen = 0;
      midiPanic();
      digitalWrite(statusLedPin, LOW);
      delay(50);
      digitalWrite(statusLedPin, HIGH);
      delay(50);
      digitalWrite(statusLedPin, LOW);
      delay(50);
      digitalWrite(statusLedPin, HIGH);
      delay(50);
      digitalWrite(statusLedPin, LOW);
      delay(50);
      digitalWrite(statusLedPin, HIGH);
      delay(700);
    }
  }
}

//***********************************************************

void extraController() {
  // Extra Controller is the lip touch sensor (proportional) in front of the mouthpiece
  exSensor = exSensor * 0.6 + 0.4 * touchRead(extraPin); // get sensor data, do some smoothing - SENSOR PIN 16 - PCB PIN "EC" (marked K4 on some prototype boards)
  if (extraCT && (exSensor >= extracThrVal)) { // if we are enabled and over the threshold, send data
    if (!extracIsOn) {
      extracIsOn = 1;
      if (extraCT == 4) { //Sustain ON
        midiSendControlChange(64, 127);
      }
    }
    if (extraCT == 1) { //Send modulation
      int extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac) {
        midiSendControlChange(1, extracCC);
      }
      oldextrac = extracCC;
    }
    if (extraCT == 2) { //Send foot pedal (CC#4)
      int extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac) {
        midiSendControlChange(4, extracCC);
      }
      oldextrac = extracCC;
    }
    if ((extraCT == 3) && (breathCC != 9)) { //Send filter cutoff (CC#74)
      int extracCC = map(constrain(exSensor, extracThrVal, extracMaxVal), extracThrVal, extracMaxVal, 1, 127);
      if (extracCC != oldextrac) {
        midiSendControlChange(74, extracCC);
      }
      oldextrac = extracCC;
    }
  } else if (extracIsOn) { // we have just gone below threshold, so send zero value
    extracIsOn = 0;
    if (extraCT == 1) { //MW
      if (oldextrac != 0) {
        //send modulation 0
        midiSendControlChange(1, 0);
        oldextrac = 0;
      }
    } else if (extraCT == 2) { //FP
      if (oldextrac != 0) {
        //send foot pedal 0
        midiSendControlChange(4, 0);
        oldextrac = 0;
      }
    } else if ((extraCT == 3) && (breathCC != 9)) { //CF
      if (oldextrac != 0) {
        //send filter cutoff 0
        midiSendControlChange(74, 0);
        oldextrac = 0;
      }
    } else if (extraCT == 4) { //SP
      //send sustain off
      midiSendControlChange(64, 0);
    }
  }
}

//***********************************************************

void portamento_() {
  // Portamento is controlled with the bite sensor (variable capacitor) in the mouthpiece
  if (biteJumper){ //PBITE (if pulled low with jumper, use pressure sensor on A7)
    biteSensor=analogRead(A7); // alternative kind bite sensor (air pressure tube and sensor)  PBITE 
   } else { 
    biteSensor=touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right) 
   }
  if (portamento && (biteSensor >= portamThrVal)) { // if we are enabled and over the threshold, send portamento
    if (!portIsOn) {
      portOn();
    }
    port();
  } else if (portIsOn) { // we have just gone below threshold, so send zero value
    portOff();
  }
}

//***********************************************************

void portOn() {
  if (portamento == 2) { // if portamento midi switching is enabled
    midiSendControlChange(CCN_PortOnOff, 127);
  }
  portIsOn = 1;
}

//***********************************************************

void port() {
  int portCC;
  portCC = map(constrain(biteSensor, portamThrVal, portamMaxVal), portamThrVal, portamMaxVal, 0, 127);
  if (portCC != oldport) {
    midiSendControlChange(CCN_Port, portCC);
  }
  oldport = portCC;
}

//***********************************************************

void portOff() {
  if (oldport != 0) { //did a zero get sent? if not, then send one
    midiSendControlChange(CCN_Port, 0);
  }
  if (portamento == 2) { // if portamento midi switching is enabled
    midiSendControlChange(CCN_PortOnOff, 0);
  }
  portIsOn = 0;
  oldport = 0;
}

//***********************************************************

void readSwitches() {
  int qTransp;
  // Read touch pads (MPR121) and put value in variables
  int touchValue[12];
  for (byte i = 0; i < 12; i++) {
    touchValue[i] = touchSensor.filteredData(i);
  }

  // Octave rollers
  octaveR = 0;
  if ((touchValue[R5Pin] < ctouchThrVal) && (touchValue[R3Pin] < ctouchThrVal)) octaveR = 6; //R6 = R5 && R3
  else if (touchValue[R5Pin] < ctouchThrVal) octaveR = 5; //R5
  else if (touchValue[R4Pin] < ctouchThrVal) octaveR = 4; //R4
  else if ((touchValue[R3Pin] < ctouchThrVal) && lastOctaveR) octaveR = 3; //R3
  else if (touchValue[R2Pin] < ctouchThrVal) octaveR = 2; //R2
  else if (touchValue[R1Pin] < ctouchThrVal) octaveR = 1; //R1

  lastOctaveR = octaveR;

  // Valves and trill keys
  K4 = (touchValue[K4Pin] < ctouchThrVal);
  K1 = (touchValue[K1Pin] < ctouchThrVal);
  K2 = (touchValue[K2Pin] < ctouchThrVal);
  K3 = (touchValue[K3Pin] < ctouchThrVal);
  K5 = (touchValue[K5Pin] < ctouchThrVal);
  K6 = (touchValue[K6Pin] < ctouchThrVal);
  K7 = (touchValue[K7Pin] < ctouchThrVal);

  pinkyKey = (touchRead(halfPitchBendKeyPin) > touch_Thr); // SENSOR PIN 1  - PCB PIN "S1" 

  if ((pinkySetting < 12) && pinkyKey) {
    qTransp = pinkySetting - 12;
  } else if ((pinkySetting > 12) && pinkyKey) {
    qTransp = pinkySetting - 12;
  } else {
    qTransp = 0;
  }

  // Calculate midi note number from pressed keys  
  #if defined(CASSIDY)
  fingeredNote = startNote - 2*K1 - K2 - 3*K3 - 5*K4 + 2*K5 + K6 + 3*K7 + octaveR*12 + (octave - 3)*12 + transpose - 12 + qTransp;
  fingeredNoteUntransposed = startNote - 2*K1 - K2 - 3*K3 - 5*K4 + 2*K5 + K6 + 3*K7 + octaveR*12;
  #else
  fingeredNote = startNote - 2*K1 - K2 - 3*K3 - 5*K4 + 2*K5 + K6 + 4*K7 + octaveR*12 + (octave - 3)*12 + transpose - 12 + qTransp;
  fingeredNoteUntransposed = startNote - 2*K1 - K2 - 3*K3 - 5*K4 + 2*K5 + K6 + 4*K7 + octaveR*12;
  #endif

  if (pinkyKey) pitchlatch = fingeredNoteUntransposed; //use pitchlatch to make settings based on note fingered
}
