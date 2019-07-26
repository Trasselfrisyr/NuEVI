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
unsigned short transpose;
unsigned short MIDIchannel;
unsigned short breathCC;  // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+:CF
unsigned short breathCC2;  // OFF:1-127
unsigned short breathCC2Rise;  // 1X:2X:3X:4X:5X
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
unsigned short vibSquelch = 12; //vibrato signal squelch
unsigned short vibDirection = DNWD; //direction of first vibrato wave UPWD or DNWD
unsigned short vibSensBite = 2; // vibrato sensitivity (bite)
unsigned short vibSquelchBite = 12; //vibrato signal squelch (bite)
unsigned short vibControl = 0;

unsigned short fastPatch[7] = {0,0,0,0,0,0,0};

uint16_t bcasMode; //Legacy CASSIDY compile flag
uint16_t trill3_interval;
uint16_t fastBoot;
uint16_t dacMode;

byte rotatorOn = 0;
byte currentRotation = 0;
uint16_t rotations[4]; // semitones { -5, -10, -7, -14 };
uint16_t parallel; // = 7; // semitones

byte gateOpen = 0; // setting for gate always open, note on sent for every time fingering changes, no matter the breath status
uint16_t gateOpenEnable = 0;

uint16_t specialKeyEnable = 0;

int touch_Thr = 1300;

byte ccList[11] = {0,1,2,7,11,1,2,7,11,74,20};  // OFF, Modulation, Breath, Volume, Expression (then same sent in hires), CC74 (cutoff/brightness), CC20 (UNO Cutoff)

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

uint16_t legacy = 0;
uint16_t legacyBrAct = 0;
byte halfTime = 0;
boolean programonce = false;
byte slowMidi = 0;

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
int leverPortThr = 50;
int leverPortRead;


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


  //Read eeprom data into global vars
  readEEPROM();

  activePatch = patch;

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
  vibZero = vibZeroBite = breathCalZero = 0;
  const int sampleCount = 4;
  for(int i = 1 ; i <= sampleCount; ++i) {
    vibZero += touchRead(vibratoPin);
    breathCalZero += analogRead(breathSensorPin);
    if (biteJumper) vibZeroBite += analogRead(A7); else vibZeroBite += touchRead(bitePin);
    digitalWrite( statusLedPin, i&1 );
    delay(fastBoot?75:250); //Shorter delay for fastboot
  }

  vibZero /= sampleCount;
  breathCalZero /= sampleCount;
  vibZeroBite /= sampleCount;
  leverPortZero = vibZero;
  vibThr = vibZero - vibSquelch;
  vibThrLo = vibZero + vibSquelch;
  vibThrBite = vibZeroBite - vibSquelchBite;
  vibThrBiteLo = vibZeroBite + vibSquelchBite;

  if(!fastBoot) {
    statusLedFlash(500);
    statusLedOff();

    showVersion();

    delay(1500);
  }

  mainState = NOTE_OFF;       // initialize main state machine

  if (!digitalRead(ePin)) {
    activePatch=0;
    doPatchUpdate=1;
  }

  activeMIDIchannel = MIDIchannel;
  midiInitialize(MIDIchannel);

  //Serial.begin(9600); // debug

  statusLedOn();    // Switch on the onboard LED to indicate power on/ready

}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {
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
      bool brSuck = analogRead(breathSensorPin) < (breathCalZero - (bcasMode?900:800));
      
      if (
          (bothPB && legacy) ||
          (brSuck && legacyBrAct && (bothPB || bcasMode))
          ) { // both pb pads touched or br suck


        fingeredNoteUntransposed = patchLimit(fingeredNoteUntransposed + 1);
        if (exSensor >= ((extracThrVal + extracMaxVal) / 2)) { // instant midi setting     
          if ((fingeredNoteUntransposed >= 73) && (fingeredNoteUntransposed <= 88)) {
            MIDIchannel = fingeredNoteUntransposed - 72; // Mid C and up 
          }
        } else {
          if (!pinkyKey) { // note number to patch number
            if (patch != fingeredNoteUntransposed) {
              patch = fingeredNoteUntransposed;
              doPatchUpdate = 1;
            }
          } else { // hi and lo patch numbers
            if (fingeredNoteUntransposed > 75) {
              if (patch != patchLimit(fingeredNoteUntransposed + 24)) {
                patch = patchLimit(fingeredNoteUntransposed + 24); // add 24 to get high numbers 108 to 127
                doPatchUpdate = 1;
              }
            } else {
              if (patch != patchLimit(fingeredNoteUntransposed - 36)) {
                patch = patchLimit(fingeredNoteUntransposed - 36); // subtract 36 to get low numbers 0 to 36
                doPatchUpdate = 1;
              }
            }
          }
        }
      } else {
        if (pbDn > (pitchbMaxVal + pitchbThrVal) / 2 && (analogRead(breathSensorPin) < (breathCalZero - 800)) && programonce == false) { // down bend for suck programming button
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

    if (analogRead(breathSensorPin) > (breathCalZero - 800)) programonce = false;

    if (specialKeyEnable) {
      specialKey = (touchRead(specialKeyPin) > touch_Thr); //S2 on pcb
      if (lastSpecialKey != specialKey) {
        if (specialKey) {
          // special key just pressed, check other keys
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
    } else {
      rotatorOn = 0;
      slurSustain = 0;
      parallelChord = 0;
      subOctaveDouble = 0;
    }
  } else if (mainState == RISE_WAIT) {
    if ((pressureSensor > breathThrVal) || gateOpen) {
      // Has enough time passed for us to collect our second
      // sample?
      if ((millis() - breath_on_time > velSmpDl) || (0 == velSmpDl)) {
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
          midiSendNoteOn(noteValueCheck(fingeredNote + parallel-24), velocitySend); // send Note On message for new note
          if (currentRotation < 3) currentRotation++;
          else currentRotation = 0;
          midiSendNoteOn(noteValueCheck(fingeredNote + rotations[currentRotation]-24), velocitySend); // send Note On message for new note
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
        midiSendNoteOff(noteValueCheck(activeNote + parallel-24 )); // send Note Off message for old note
        midiSendNoteOff(noteValueCheck(activeNote + rotations[currentRotation]-24)); // send Note Off message for old note
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
          midiSendNoteOff(noteValueCheck(activeNote + parallel-24)); // send Note Off message for old note
          midiSendNoteOff(noteValueCheck(activeNote + rotations[currentRotation]-24)); // send Note Off message for old note
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
          midiSendNoteOn(noteValueCheck(fingeredNote + parallel-24), velocitySend); // send Note On message for new note
          if (currentRotation < 3) currentRotation++;
          else currentRotation = 0;
          midiSendNoteOn(noteValueCheck(fingeredNote + rotations[currentRotation]-24), velocitySend); // send Note On message for new note
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
      statusLedFlip();
    } else {
      statusLedOn();
    }
    pixelUpdateTime = millis();
  }

  if(dacMode == DAC_MODE_PITCH) { // pitch CV from DAC and breath CV from PWM on pin 6, for filtering and scaling on separate board
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
  } else if(dacMode == DAC_MODE_BREATH) { // else breath CV on DAC pin, directly to unused pin of MIDI DIN jack
    analogWrite(dacPin,breathCurve(map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,4095)));
  }

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
  if(curve >= ARR_LEN(curves)) return inputVal;
  return multiMap(inputVal, curveIn, curves[curve], 17);
}

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
  if (portIsOn) { // portamento indicator LED, labeled "P" on PCB
    //analogWrite(pLedPin, map(biteSensor,0,4096,5,portamLedBrightness));
    analogWrite(pLedPin, map(constrain(oldport, 0, 127), 0, 127, 5, portamLedBrightness));
  } else {
    analogWrite(pLedPin, 0);
  }
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
  int calculatedPBdepth;
  byte pbTouched = 0;
  int vibRead;
  int vibReadBite;
  pbUp = touchRead(pbUpPin); // SENSOR PIN 23 - PCB PIN "Pu"
  pbDn = touchRead(pbDnPin); // SENSOR PIN 22 - PCB PIN "Pd"
  halfPitchBendKey = (pinkySetting == PBD) && (touchRead(halfPitchBendKeyPin) > touch_Thr); // SENSOR PIN 1  - PCB PIN "S1" - hold for 1/2 pitchbend value

  calculatedPBdepth = pbDepthList[PBdepth];
  if (halfPitchBendKey) calculatedPBdepth = calculatedPBdepth * 0.5;


  vibMax = vibMaxList[vibSens - 1];

  if (vibControl){ //bite vibrato
    if (biteJumper){ //PBITE (if pulled low with jumper, use pressure sensor instead of capacitive bite sensor)
      vibReadBite = analogRead(bitePressurePin); // alternative kind bite sensor (air pressure tube and sensor)  PBITE
    } else { 
      vibReadBite = touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right) 
    }
        if (vibReadBite < vibThrBite) {
      if (UPWD == vibDirection) {
        vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibReadBite, (vibZeroBite - vibMax), vibThrBite), vibThrBite, (vibZeroBite - vibMax), 0, calculatedPBdepth * vibDepth[vibrato]);
      } else {
        vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibReadBite, (vibZeroBite - vibMax), vibThrBite), vibThrBite, (vibZeroBite - vibMax), 0, (0 - calculatedPBdepth * vibDepth[vibrato]));
      }
    } else if (vibReadBite > vibThrBiteLo) {
      if (UPWD == vibDirection) {
        vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibReadBite, vibThrBiteLo, (vibZeroBite + vibMax)), vibThrBiteLo, (vibZeroBite + vibMax), 0, (0 - calculatedPBdepth * vibDepth[vibrato]));
      } else {
        vibSignal = vibSignal * 0.5 + 0.5 * map(constrain(vibReadBite, vibThrBiteLo, (vibZeroBite + vibMax)), vibThrBiteLo, (vibZeroBite + vibMax), 0, calculatedPBdepth * vibDepth[vibrato]);
      }
    } else {
      vibSignal = vibSignal * 0.5;
    }
  } else { //lever vibrato
    vibRead = touchRead(vibratoPin); // SENSOR PIN 15 - built in var cap  
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
  if (gateOpenEnable){
    if ((touchValue[K4Pin] < ctouchThrVal) && (touchValue[R1Pin] < ctouchThrVal) && (touchValue[R2Pin] < ctouchThrVal) && (touchValue[R3Pin] < ctouchThrVal)) { // doorknob grip on canister
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
  } else if (gateOpen) {
    gateOpen = 0;
    midiPanic();
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

  if (biteJumper){ //PBITE (if pulled low with jumper, use pressure sensor instead of capacitive bite sensor)
    biteSensor=analogRead(bitePressurePin); // alternative kind bite sensor (air pressure tube and sensor)  PBITE
   } else { 
    biteSensor = touchRead(bitePin);     // get sensor data, do some smoothing - SENSOR PIN 17 - PCB PINS LABELED "BITE" (GND left, sensor pin right) 
   }
  if (!vibControl){
    // Portamento is controlled with the bite sensor (variable capacitor) in the mouthpiece
    if (portamento && (biteSensor >= portamThrVal)) { // if we are enabled and over the threshold, send portamento
      if (!portIsOn) {
        portOn();
      }
      port();
    } else if (portIsOn) { // we have just gone below threshold, so send zero value
      portOff();
    }
  } else {
    leverPortRead = touchRead(vibratoPin);
    if (portamento && (leverPortRead <= (leverPortZero-leverPortThr))) { // if we are enabled and over the threshold, send portamento
      if (!portIsOn) {
       portOn();
      }
      port();
    } else if (portIsOn) { // we have just gone below threshold, so send zero value
    portOff();
  }
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
  if (!vibControl)
    portCC = map(constrain(biteSensor, portamThrVal, portamMaxVal), portamThrVal, portamMaxVal, 0, 127);
  else
    portCC = constrain((leverPortZero-leverPortThr-leverPortRead),0,127);
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
  
  // Read touch pads (MPR121), compare against threshold value
  bool touchKeys[12];
  for (byte i = 0; i < 12; i++) {
    touchKeys[i] = touchSensor.filteredData(i) < ctouchThrVal;
  }

  // Octave rollers
  octaveR = 0;
  if (touchKeys[R5Pin] && touchKeys[R3Pin]) octaveR = 6; //R6 = R5 && R3
  else if (touchKeys[R5Pin]) octaveR = 5; //R5
  else if (touchKeys[R4Pin]) octaveR = 4; //R4
  else if (touchKeys[R3Pin] && lastOctaveR) octaveR = 3; //R3
  else if (touchKeys[R2Pin]) octaveR = 2; //R2
  else if (touchKeys[R1Pin]) octaveR = 1; //R1

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

  int qTransp = pinkyKey ? pinkySetting-12 : 0;

  // Calculate midi note number from pressed keys  

  fingeredNoteUntransposed = startNote
    - 2*K1 - K2 - 3*K3  //"Trumpet valves"
    - 5*K4              //Fifth key
    + 2*K5 + K6 + trill3_interval*K7  //Trill keys. 3rd trill key interval controlled by setting
    + octaveR*12;       //Octave rollers

  int fingeredNoteRead = fingeredNoteUntransposed + transpose - 12 + qTransp;

  if (pinkyKey) pitchlatch = fingeredNoteUntransposed; //use pitchlatch to make settings based on note fingered

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



void statusLedOn() {
  digitalWrite(statusLedPin, HIGH);
}

void statusLedOff() {
  digitalWrite(statusLedPin, LOW);
}

void statusLedFlip() {
  digitalWrite(statusLedPin, !digitalRead(statusLedPin));
}

void statusLedFlash(uint16_t delayTime) {
  statusLedOff();
  delay(delayTime/2);
  statusLedOn();
  delay(delayTime/2);
}
