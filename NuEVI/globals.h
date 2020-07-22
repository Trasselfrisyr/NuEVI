#ifndef __GLOBALS_H
#define __GLOBALS_H

#include "wiring.h"


// The three states of our main state machine

// No note is sounding
#define NOTE_OFF 1

// We've observed a transition from below to above the
// threshold value. We wait a while to see how fast the
// breath velocity is increasing
#define RISE_WAIT 2

// A note is sounding
#define NOTE_ON 3


//Magic value where pinky button means "pitch bend"
#define PBD 12
#define EC2 25
#define ECSW 26
#define LVL 27
#define LVLP 28
#define GLD 29
#define ECH 30

#define HOF 0
#define MGR 1
#define MGD 2
#define MA9 3
#define MND 4
#define MNA 5
#define MNH 6
#define FWC 7
#define RT1 8
#define RT2 9
#define RT3 10

#define MOD 13


//Vibrato direction
#define UPWD 1
#define DNWD 0


extern const unsigned short* const curves[];
extern const unsigned short curveIn[];

#if defined(NURAD)
extern int calOffsetRollers[6];
extern int calOffsetRH[12];
extern int calOffsetLH[12];
#endif

extern unsigned short breathThrVal;
extern unsigned short breathMaxVal;
extern unsigned short portamThrVal;
extern unsigned short portamMaxVal;
extern unsigned short pitchbThrVal;
extern unsigned short pitchbMaxVal;
extern unsigned short extracThrVal;
extern unsigned short extracMaxVal;
extern unsigned short ctouchThrVal;
extern unsigned short transpose;
extern unsigned short MIDIchannel;
extern unsigned short breathCC;  // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+:CF:UNO
extern unsigned short breathCC2;  // OFF:1-127
extern unsigned short breathCC2Rise;  // 1X:2X:3X:4X:5X
extern unsigned short breathAT;
extern unsigned short velocity;
extern unsigned short portamento;// switching on cc65? just cc5 enabled? SW:ON:OFF
extern unsigned short PBdepth;   // OFF:1-12 divider
extern unsigned short extraCT;   // OFF:MW:FP:CF:SP
extern unsigned short vibrato;   // OFF:1-9
extern unsigned short deglitch;  // 0-70 ms in steps of 5
extern unsigned short patch;     // 1-128
extern unsigned short octave;
extern unsigned short curve;
extern unsigned short velSmpDl;  // 0-30 ms
extern unsigned short velBias;   // 0-9
extern unsigned short pinkySetting; // 0 - 11 (QuickTranspose -12 to -1), 12 (pb/2), 13 - 24 (QuickTranspose +1 to +12), 25 (EC2), 26 (ECSW), 27 (LVL), 28 (LVLP)
extern unsigned short dipSwBits; // virtual dip switch settings for special modes (work in progress)
extern unsigned short priority; // mono priority for rotator chords
extern unsigned short vibSens; // vibrato sensitivity
extern unsigned short vibRetn; // vibrato return speed
extern unsigned short vibSquelch; //vibrato signal squelch
extern unsigned short vibDirection; //direction of first vibrato wave UPWD or DNWD
extern unsigned short vibSensBite; // vibrato sensitivity (bite)
extern unsigned short vibSquelchBite; //vibrato signal squelch (bite)
extern unsigned short vibControl;
extern unsigned short fastPatch[7];
extern unsigned short extraCT2; // OFF:1-127
extern unsigned short levelCC; // 0-127
extern unsigned short levelVal; // 0-127
extern unsigned short fingering; // 0-4 EWI,EWX,SAX,EVI,EVR
extern unsigned short lpinky3; // 0-25 (OFF, -12 - MOD - +12)
extern unsigned short batteryType; // 0-2 ALK,NIM,LIP
extern unsigned short harmSetting; // 0-7
extern unsigned short harmSelect; // 0-4
extern unsigned short polySelect;  // OFF, MGR, MGD, MND, MNH, FWC, RTA, RTB or RTC
extern unsigned short fwcType; // 6, m6, 7, m7
extern unsigned short fwcLockH; // OFF:ON
extern unsigned short fwcDrop2; // OFF:ON
extern unsigned short hmzKey; // 0-11 (0 is C) 
extern unsigned short hmzLimit; // 2-5
extern unsigned short otfKey; //OFF:ON
extern unsigned short breathInterval; // 3-15
extern uint16_t gateOpenEnable;
extern uint16_t specialKeyEnable;
extern byte rotatorOn;
extern byte currentRotation;
extern uint16_t rotations[4];
extern uint16_t parallel; // semitones
extern uint16_t rotationsb[4];
extern uint16_t parallelb; // semitones
extern uint16_t rotationsc[4];
extern uint16_t parallelc; // semitones

extern uint16_t bcasMode; //Legacy CASSIDY compile flag
extern uint16_t trill3_interval;
extern uint16_t fastBoot;
extern uint16_t dacMode;


extern int touch_Thr;

extern unsigned long cursorBlinkTime;          // the last time the cursor was toggled

extern byte activePatch;
extern byte doPatchUpdate;

extern uint16_t legacy;
extern uint16_t legacyBrAct;

extern byte slowMidi;

extern int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
extern int lastPressure;

extern int biteSensor;    // capacitance data from bite sensor, for midi cc and threshold checks
extern int lastBite;
extern byte biteJumper;

extern int exSensor;
extern int exSensorIndicator;

extern int pitchBend;

extern int pbUp;
extern int pbDn;

extern byte vibLedOff;
extern byte oldpkey;

extern int vibThr;          // this gets auto calibrated in setup
extern int vibThrLo;
extern int vibZero;
extern int vibZeroBite;
extern int vibThrBite;
extern int vibThrBiteLo;

extern int battAvg;

extern int breathLevel;
extern byte portIsOn;
extern int oldport;

#if defined(NURAD)
            // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
extern byte LHs;            
extern byte LH1;   // Left Hand key 1 (pitch change -2)
extern byte LHb;   // Left Hand bis key (pitch change -1 unless both LH1 and LH2 are pressed)
extern byte LH2;   // Left Hand key 2  (with LH1 also pressed pitch change is -2, otherwise -1)
extern byte LH3;   // Left Hand key 3 (pitch change -2)
extern byte LHp1;  // Left Hand pinky key 1 (pitch change +1)
extern byte LHp2;  // Left Hand pinky key 2 (pitch change -1)
extern byte LHp3;
extern byte RHs;   // Right Hand side key  (pitch change -2 unless LHp1 is pressed)
extern byte RH1;   // Right Hand key 1 (with LH3 also pressed pitch change is -2, otherwise -1)
extern byte RH2;   // Right Hand key 2 (pitch change -1)
extern byte RH3;   // Right Hand key 3 (pitch change -2)
extern byte RHp1;  // Right Hand pinky key 1 (pitch change +1)
extern byte RHp2;  // Right Hand pinky key 2 (pitch change -1)
extern byte RHp3;  // Right Hand pinky key 3 (pitch change -2)
extern byte Tr1;  // Trill key 1 (pitch change +2) (EVI fingering)
extern byte Tr2;  // Trill key 2 (pitch change +1)
extern byte Tr3;  // Trill key 3 (pitch change +4)
#endif

// Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
extern byte K1;   // Valve 1 (pitch change -2)
extern byte K2;   // Valve 2 (pitch change -1)
extern byte K3;   // Valve 3 (pitch change -3)
extern byte K4;   // Left Hand index finger (pitch change -5)
extern byte K5;   // Trill key 1 (pitch change +2)
extern byte K6;   // Trill key 2 (pitch change +1)
extern byte K7;   // Trill key 3 (pitch change +4)

extern byte halfPitchBendKey;
extern byte specialKey;
extern byte pinkyKey;

extern unsigned int multiMap(unsigned short val, const unsigned short * _in, const unsigned short * _out, uint8_t size);

#endif
