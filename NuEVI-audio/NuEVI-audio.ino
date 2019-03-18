#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Filters.h>  // for the breath signal LP filtering, https://github.com/edgar-bonet/Filters
#include <Audio.h>
#include <SD.h>
#include <SerialFlash.h>

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

//_______________________________________________________________________________________________ DECLARATIONS

// Compile options, comment/uncomment to change

#define REVB


// Pin definitions

// Teensy pins

#define specialKeyPin 0       // SK or S2
#define halfPitchBendKeyPin 1 // PD or S1

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

#define vMeterPin A11

#define PBD 12

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
#define ROTATOR_MENU 100
#define SYNTH_MENU 110

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
#define VEL_SMP_DL_ADDR 46
#define VEL_BIAS_ADDR 48
#define PINKY_KEY_ADDR 50
#define FP1_ADDR 52
#define FP2_ADDR 54
#define FP3_ADDR 56
#define FP4_ADDR 58
#define FP5_ADDR 60
#define FP6_ADDR 62
#define FP7_ADDR 64
#define DIPSW_BITS_ADDR 66
#define PARAL_ADDR 68
#define ROTN1_ADDR 70
#define ROTN2_ADDR 72
#define ROTN3_ADDR 74
#define ROTN4_ADDR 76
#define PRIO_ADDR 78

//"factory" values for settings
#define VERSION 28
#define BREATH_THR_FACTORY 1400
#define BREATH_MAX_FACTORY 4000
#define PORTAM_THR_FACTORY 2600
#define PORTAM_MAX_FACTORY 3300
#define PITCHB_THR_FACTORY 1400
#define PITCHB_MAX_FACTORY 2300
#define EXTRAC_THR_FACTORY 1200
#define EXTRAC_MAX_FACTORY 2400
#define TRANSP_FACTORY 12   // 12 is 0 transpose
#define MIDI_FACTORY 1      // 1-16
#define BREATH_CC_FACTORY 2 //thats CC#2, see ccList
#define BREATH_AT_FACTORY 0 //aftertouch default off
#define VELOCITY_FACTORY 0  // 0 is dynamic/breath controlled velocity
#define PORTAM_FACTORY 2    // 0 - OFF, 1 - ON, 2 - SW 
#define PB_FACTORY 1        // 0 - OFF, 1 - 12
#define EXTRA_FACTORY 2     // 0 - OFF, 1 - Modulation wheel, 2 - Foot pedal, 3 - Filter Cutoff, 4 - Sustain pedal
#define VIBRATO_FACTORY 4   // 0 - OFF, 1 - 9 depth
#define DEGLITCH_FACTORY 20 // 0 - OFF, 5 to 70 ms in steps of 5
#define PATCH_FACTORY 1     // MIDI program change 1-128
#define OCTAVE_FACTORY 3    // 3 is 0 octave change
#define CTOUCH_THR_FACTORY 125  // MPR121 touch threshold
#define BREATHCURVE_FACTORY 4 // 0 to 12 (-4 to +4, S1 to S4)
#define VEL_SMP_DL_FACTORY 20 // 0 to 30 ms in steps of 5
#define VEL_BIAS_FACTORY 0  // 0 to 9
#define PINKY_KEY_FACTORY 12 // 0 - 11 (QuickTranspose -12 to -1), 12 (pb/2), 13 - 22 (QuickTranspose +1 to +12)
#define DIPSW_BITS_FACTORY 0 // virtual dip switch settings for special modes (work in progress)
#define PARAL_FACTORY 31 // 7 (+ 24) Rotator parallel
#define ROTN1_FACTORY 19 // -5 (+24) Rotation 1
#define ROTN2_FACTORY 14 // -10 (+24) Rotation 2
#define ROTN3_FACTORY 17 // -7 (+24) Rotation 3
#define ROTN4_FACTORY 10 // -14 (+24) Rotation 4
#define PRIO_FACTORY 0 // Mono priority 0 - BAS(e note), 1 - ROT(ating note)

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

const int16_t AKWF_vgame_0006[257] = {
     1,     2,     2,     2,     4,     5,     6,     7,     9,
     7,    10,    10,    11,    11,    13,    12,    15,    12,    16,
    14,    16,    14,    18,    16,    18,    15,    20,    16,    21,
    18,    20,    21,    20,    21,    19,    24,    19,    25,    16,
    27,    15,    29,    13,    32,    13,    34,    11,    34,    12,
    34,    13,    33,    17,    30,    20,    25,    25,    19,    32,
    12,    41,     3,    50,    -7,    63,   -21,    78,   -38,    98,
   -63,   128,  -105,   189,  -211,   370,  -473, 11424, 28860, 30309,
 32767, 20949,  3562,   277, -2542, -2886, -3011, -2718, -2830, -2678,
 -2711, -2583, -2584, -2495, -2470, -2404, -2359, -2315, -2257, -2226,
 -2161, -2140, -2068, -2054, -1980, -1970, -1897, -1888, -1818, -1808,
 -1743, -1731, -1670, -1654, -1601, -1579, -1535, -1510, -1469, -1442,
 -1408, -1376, -1348, -1315, -1289, -1255, -1233, -1199, -1178, -1145,
 -1123, -1092, -1073, -1044, -1024,  -995,  -975,  -950,  -929,  -906,
  -885,  -864,  -842,  -823,  -803,  -783,  -764,  -746,  -727,  -710,
  -691,  -675,  -656,  -641,  -625,  -608,  -594,  -579,  -563,  -549,
  -535,  -521,  -507,  -494,  -480,  -468,  -456,  -442,  -430,  -418,
  -407,  -397,  -386,  -375,  -364,  -353,  -343,  -334,  -324,  -314,
  -306,  -296,  -288,  -279,  -271,  -262,  -255,  -247,  -240,  -232,
  -225,  -218,  -211,  -204,  -198,  -191,  -184,  -178,  -172,  -167,
  -162,  -155,  -150,  -144,  -140,  -134,  -129,  -124,  -120,  -116,
  -112,  -109,  -103,  -100,   -95,   -92,   -87,   -85,   -81,   -77,
   -74,   -70,   -67,   -63,   -60,   -58,   -56,   -52,   -49,   -48,
   -44,   -42,   -40,   -37,   -35,   -33,   -31,   -29,   -26,   -24,
   -23,   -21,   -20,   -17,   -15,   -14,   -12,   -11,    -9,    -8,
    -7,    -6,    -5,    -3,    -1,    -1,     0,     1
};

const int16_t AKWF_vgtri_0008[257] = {
    63,   609,   287,   611,   129,  1825,  2747,  2663,  2408,
  3562,  5515,  5160,  5230,  5504,  7586,  7340,  7533,  7305,  7687,
  9613,  9631,  9637,  9524, 11857, 12359, 12250, 12081, 13593, 14672,
 14355, 14361, 15180, 16991, 16376, 16863, 16293, 17459, 18916, 18848,
 18683, 19150, 21474, 21446, 21417, 21419, 23298, 23784, 23498, 23745,
 23423, 25488, 25834, 25908, 25526, 27395, 28652, 28406, 28302, 29161,
 30890, 30482, 30739, 30475, 31022, 32065, 31842, 32069, 31644, 29346,
 29267, 29344, 29327, 27546, 26910, 27287, 26952, 27382, 25289, 24917,
 24820, 25244, 23430, 22091, 22333, 22448, 21624, 19895, 20195, 20142,
 19911, 17841, 17994, 17872, 18024, 17770, 15792, 15732, 15724, 15878,
 13625, 12958, 13165, 13239, 11947, 10581, 11185, 10683, 11303,  9673,
  8649,  8681,  8968,  7981,  5921,  6172,  6155,  5952,  3918,  3941,
  3920,  3973,  3846,  1818,  1744,  1706,  1893,    33,  -562,  -425,
  -292, -1562, -2952, -2401, -2849, -2282, -3780, -4860, -6817, -7657,
 -7320, -7501, -7404, -7421, -7549, -9619, -9605, -9669, -9484,-11431,
-14014,-13961,-14049,-13933,-14078,-13894,-14153,-13685,-15369,-16886,
-16534,-16605,-16969,-19998,-21140,-20949,-21022,-20982,-21006,-21004,
-20963,-21098,-23516,-23658,-23634,-23504,-25279,-28009,-27935,-28085,
-27892,-28127,-27839,-28221,-27607,-29387,-30757,-30699,-30366,-31414,
-32079,-30342,-30740,-30564,-30652,-30617,-30606,-30669,-30547,-28194,
-27905,-28032,-28080,-26491,-23630,-23689,-23533,-23720,-23513,-23745,
-23459,-23902,-22617,-21243,-21480,-21483,-21109,-17813,-16475,-16670,
-16600,-16636,-16612,-16626,-16626,-16623,-14639,-14373,-14424,-14589,
-12705, -9668, -9671, -9554, -9699, -9536, -9725, -9480, -9883, -8706,
 -7231, -7497, -7458, -7173, -3925, -2474, -2670, -2602, -2637, -2612,
 -2630, -2620, -2658,  -712,  -321,  -490,  -442,    63
};

const int16_t AKWF_violin_0010[257] = {
    27,   303,  1129,  2595,  4250,  5804,  7288,  8741, 10134,
 11437, 12646, 13753, 14783, 15739, 16600, 17358, 17995, 18484, 18819,
 19029, 19146, 19172, 19102, 18942, 18690, 18357, 17953, 17508, 17045,
 16549, 15998, 15414, 14847, 14338, 13865, 13406, 12962, 12521, 12076,
 11672, 11380, 11203, 11075, 10944, 10836, 10785, 10802, 10885, 11040,
 11260, 11529, 11844, 12213, 12636, 13125, 13679, 14304, 14993, 15730,
 16512, 17335, 18205, 19111, 20049, 21018, 22026, 23082, 24153, 25186,
 26174, 27109, 27982, 28814, 29631, 30426, 31136, 31727, 32189, 32513,
 32694, 32764, 32754, 32643, 32391, 31986, 31429, 30703, 29836, 28882,
 27852, 26719, 25456, 24081, 22672, 21248, 19761, 18182, 16530, 14891,
 13302, 11750, 10238,  8774,  7337,  5936,  4571,  3254,  1999,   838,
  -206, -1154, -2020, -2805, -3462, -3967, -4341, -4611, -4787, -4881,
 -4906, -4858, -4722, -4495, -4199, -3848, -3448, -2979, -2446, -1905,
 -1410,  -956,  -492,    -1,   487,   952,  1385,  1766,  2086,  2364,
  2632,  2871,  3014,  3016,  2915,  2754,  2536,  2256,  1914,  1498,
   981,   364,  -325, -1065, -1872, -2795, -3839, -4945, -6079, -7240,
 -8461, -9748,-11061,-12369,-13671,-15008,-16373,-17730,-19056,-20330,
-21545,-22694,-23794,-24888,-25950,-26909,-27739,-28460,-29112,-29690,
-30154,-30490,-30711,-30828,-30849,-30782,-30640,-30412,-30088,-29668,
-29163,-28585,-27940,-27231,-26473,-25692,-24920,-24149,-23323,-22440,
-21568,-20770,-20039,-19334,-18639,-17973,-17337,-16749,-16256,-15873,
-15569,-15310,-15090,-14918,-14789,-14721,-14736,-14834,-14992,-15204,
-15451,-15702,-15952,-16211,-16480,-16760,-17041,-17318,-17591,-17852,
-18087,-18267,-18377,-18423,-18407,-18336,-18233,-18101,-17901,-17587,
-17175,-16711,-16203,-15622,-14930,-14131,-13237,-12246,-11172,-10039,
 -8840, -7580, -6262, -4870, -3317, -1690,  -473,    27
};

const int16_t AKWF_oboe_0006[257] = {
     8,   267,   522,   775,  1026,  1276,  1525,  1768,  2015,
  2254,  2496,  2736,  2980,  3228,  3484,  3750,  4029,  4321,  4627,
  4957,  5314,  5705,  6136,  6607,  7119,  7679,  8282,  8931,  9626,
 10366, 11151, 11982, 12854, 13771, 14724, 15718, 16754, 17818, 18909,
 20012, 21117, 22221, 23311, 24383, 25434, 26446, 27412, 28331, 29186,
 29972, 30688, 31317, 31868, 32317, 32670, 32767, 32761, 32767, 32764,
 32762, 32523, 32105, 31593, 30966, 30239, 29403, 28475, 27447, 26323,
 25111, 23820, 22460, 21052, 19591, 18097, 16571, 15021, 13464, 11900,
 10330,  8772,  7225,  5698,  4210,  2752,  1341,   -13, -1307, -2533,
 -3692, -4791, -5819, -6782, -7679, -8501, -9261, -9956,-10585,-11155,
-11667,-12121,-12524,-12878,-13190,-13465,-13708,-13925,-14116,-14289,
-14453,-14616,-14786,-14974,-15179,-15404,-15651,-15919,-16208,-16519,
-16850,-17200,-17571,-17961,-18365,-18781,-19209,-19639,-20077,-20515,
-20949,-21376,-21787,-22182,-22560,-22912,-23235,-23527,-23779,-23989,
-24156,-24277,-24347,-24369,-24337,-24254,-24116,-23925,-23674,-23367,
-22996,-22558,-22057,-21493,-20871,-20195,-19465,-18689,-17871,-17011,
-16116,-15192,-14250,-13299,-12353,-11419,-10490, -9569, -8654, -7737,
 -6822, -5901, -4975, -4056, -3147, -2262, -1407,  -581,   209,   964,
  1679,  2346,  2960,  3523,  4026,  4470,  4857,  5193,  5490,  5755,
  5990,  6199,  6375,  6520,  6630,  6702,  6733,  6724,  6678,  6594,
  6470,  6305,  6096,  5846,  5553,  5222,  4859,  4472,  4068,  3656,
  3236,  2808,  2374,  1937,  1491,  1036,   579,   116,  -343,  -794,
 -1236, -1658, -2057, -2433, -2781, -3105, -3408, -3684, -3939, -4171,
 -4376, -4555, -4705, -4819, -4899, -4941, -4948, -4920, -4856, -4756,
 -4621, -4453, -4251, -4015, -3755, -3475, -3184, -2888, -2587, -2286,
 -1983, -1683, -1387, -1094,  -806,  -528,  -256,     8
};

const int16_t AKWF_altosax_0013[257] = {
     8,   934,  1757,  2638,  3598,  4607,  5581,  6586,  7689,
  8646,  9725, 10834, 12000, 13106, 14181, 15300, 16377, 17445, 18545,
 19676, 20849, 21955, 23006, 23981, 24859, 25715, 26548, 27465, 28299,
 29019, 29731, 30311, 30822, 31392, 31861, 32242, 32505, 32679, 32767,
 32759, 32734, 32668, 32605, 32508, 32224, 31908, 31595, 31137, 30655,
 30172, 29538, 28750, 27959, 27072, 26057, 24937, 23664, 22228, 20739,
 19332, 17922, 16535, 15097, 13764, 12691, 11815, 11097, 10334,  9541,
  8660,  7781,  6784,  5645,  4402,  3061,  1763,   414,  -967, -2355,
 -3633, -4711, -5737, -6695, -7510, -8359, -9124, -9794,-10386,-10944,
-11430,-11840,-12295,-12690,-13049,-13436,-13791,-14104,-14336,-14452,
-14495,-14466,-14485,-14466,-14357,-14255,-14105,-13952,-13820,-13584,
-13384,-13189,-12951,-12723,-12493,-12211,-11873,-11578,-11242,-10853,
-10496,-10149, -9693, -9160, -8531, -7916, -7337, -6652, -5955, -5338,
 -4709, -4165, -3686, -3221, -2916, -2549, -2221, -1909, -1621, -1312,
 -1023,  -879,  -752,  -703,  -638,  -631,  -657,  -678,  -783,  -920,
 -1049, -1247, -1493, -1745, -1996, -2203, -2414, -2671, -2936, -3229,
 -3543, -3908, -4205, -4536, -4896, -5202, -5584, -5959, -6371, -6794,
 -7162, -7496, -7841, -8162, -8495, -8845, -9179, -9510, -9784,-10049,
-10332,-10596,-10844,-11051,-11225,-11349,-11471,-11550,-11581,-11579,
-11589,-11672,-11690,-11688,-11679,-11666,-11603,-11594,-11632,-11563,
-11467,-11350,-11294,-11267,-11210,-11105,-10995,-10919,-10846,-10764,
-10713,-10727,-10755,-10765,-10764,-10728,-10627,-10543,-10549,-10573,
-10531,-10489,-10433,-10295,-10183,-10164,-10141,-10110,-10039, -9999,
 -9982, -9883, -9737, -9605, -9494, -9428, -9332, -9216, -9024, -8735,
 -8516, -8309, -8102, -7768, -7410, -6969, -6519, -6075, -5624, -5218,
 -4672, -4132, -3663, -3046, -2306, -1555,  -774,     8
};

const int16_t AKWF_sinharm_0011[257] = {
     0,  3519,  6994, 10388, 13660, 16769, 19684, 22368, 24792,
 26928, 28755, 30254, 31411, 32218, 32668, 32763, 32508, 31910, 30987,
 29754, 28237, 26457, 24447, 22238, 19860, 17355, 14758, 12104,  9435,
  6784,  4192,  1691,  -685, -2907, -4946, -6777, -8379, -9739,-10843,
-11681,-12248,-12549,-12585,-12365,-11903,-11215,-10322, -9246, -8013,
 -6653, -5196, -3670, -2110,  -550,   980,  2449,  3825,  5082,  6192,
  7130,  7879,  8419,  8737,  8827,  8678,  8291,  7669,  6819,  5752,
  4482,  3027,  1409,  -345, -2212, -4158, -6155, -8168,-10164,-12111,
-13974,-15722,-17324,-18749,-19972,-20969,-21716,-22201,-22404,-22322,
-21945,-21276,-20315,-19074,-17561,-15793,-13795,-11587, -9198, -6657,
 -4000, -1258,  1528,  4324,  7090,  9789, 12382, 14831, 17105, 19172,
 20998, 22559, 23833, 24798, 25441, 25752, 25722, 25355, 24650, 23615,
 22266, 20620, 18696, 16520, 14121, 11534,  8788,  5926,  2983,     0,
 -2982, -5926, -8789,-11534,-14122,-16520,-18695,-20620,-22266,-23615,
-24649,-25355,-25722,-25753,-25441,-24798,-23833,-22560,-20998,-19172,
-17105,-14831,-12382, -9789, -7091, -4323, -1529,  1258,  3999,  6656,
  9198, 11587, 13796, 15794, 17560, 19073, 20315, 21276, 21945, 22322,
 22405, 22200, 21716, 20968, 19973, 18749, 17324, 15722, 13975, 12112,
 10165,  8168,  6154,  4158,  2212,   346, -1409, -3027, -4482, -5752,
 -6819, -7670, -8291, -8678, -8827, -8737, -8418, -7879, -7130, -6192,
 -5082, -3825, -2449,  -980,   550,  2110,  3669,  5195,  6653,  8013,
  9246, 10321, 11215, 11903, 12366, 12585, 12550, 12249, 11680, 10844,
  9739,  8380,  6777,  4946,  2907,   686, -1691, -4192, -6783, -9434,
-12103,-14757,-17356,-19860,-22238,-24446,-26458,-28237,-29755,-30987,
-31911,-32508,-32763,-32668,-32217,-31411,-30254,-28755,-26928,-24792,
-22369,-19684,-16770,-13660,-10388, -6994, -3519,     0
};

const int16_t AKWF_raw_0004[257] = {
     0,   512,  1024,  1534,  2047,  2558,  3072,  3582,  4097,
  4607,  5119,  5632,  6143,  6656,  7168,  7681,  8191,  8703,  9215,
  9728, 10239, 10752, 11264, 11776, 12288, 12799, 13313, 13823, 14337,
 14847, 15360, 15872, 16385, 16895, 17408, 17920, 18432, 18943, 19456,
 19968, 20480, 20993, 21503, 22016, 22527, 23041, 23550, 24065, 24574,
 25089, 25597, 26114, 26621, 27139, 27644, 28163, 28667, 29188, 29690,
 30214, 30713, 31240, 31733, 32275, 32662, 32244, 31699, 31193, 30659,
 30149, 29618, 29108, 28579, 28065, 27539, 27024, 26498, 25984, 25460,
 24946, 24422, 23907, 23384, 22868, 22347, 21830, 21311, 20795, 20275,
 19759, 19241, 18724, 18206, 17687, 17173, 16655, 16140, 15622, 15107,
 14590, 14075, 13558, 13044, 12528, 12014, 11497, 10983, 10468,  9954,
  9438,  8924,  8411,  7896,  7383,  6870,  6356,  5842,  5330,  4817,
  4304,  3792,  3279,  2767,  2254,  1742,  1230,   720,   208,  -302,
  -813, -1325, -1836, -2346, -2857, -3367, -3876, -4387, -4896, -5406,
 -5915, -6426, -6933, -7444, -7952, -8461, -8968, -9479, -9984,-10495,
-11000,-11510,-12014,-12526,-13029,-13539,-14044,-14552,-15057,-15566,
-16070,-16577,-17082,-17588,-18094,-18598,-19106,-19605,-20116,-20614,
-21126,-21622,-22137,-22629,-23146,-23634,-24154,-24641,-25162,-25647,
-26167,-26653,-27171,-27662,-28160,-28628,-29038,-29389,-29762,-30127,
-30471,-30871,-31135,-32329,-32371,-31676,-31279,-30682,-30239,-29670,
-29205,-28652,-28174,-27635,-27146,-26615,-26118,-25595,-25091,-24574,
-24065,-23552,-23038,-22529,-22013,-21506,-20989,-20483,-19965,-19459,
-18941,-18434,-17916,-17411,-16892,-16386,-15870,-15362,-14845,-14338,
-13822,-13313,-12798,-12288,-11775,-11263,-10753,-10239, -9729, -9215,
 -8705, -8191, -7681, -7167, -6656, -6142, -5632, -5119, -4607, -4095,
 -3583, -3071, -2559, -2048, -1535, -1024,  -512,     0
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
unsigned short breathCC;  // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+
unsigned short breathAT;
unsigned short velocity;
unsigned short portamento;// switching on cc65? just cc5 enabled? SW:ON:OFF
unsigned short PBdepth;   // OFF:1-12 divider
unsigned short extraCT;   // OFF:MW:FP:FC:SP
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
unsigned short wave = 2; // SAW, SQR, TRI
unsigned short cutoff = 7; // 0-9
unsigned short reso = 2; //0-9
unsigned short filter = 0; // 0 INT, 1 EXT
unsigned short vol = 7; // 0-9

unsigned short fastPatch[7] = {0,0,0,0,0,0,0};

byte rotatorOn = 0;
byte currentRotation = 0;
int rotations[4] = { -5, -10, -7, -14 }; // semitones { -5, -10, -7, -14 };
int parallel = 7; // semitones

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
byte rotatorMenuCursor = 1;
byte synthMenuCursor = 1;

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
byte subPinky = 0;
byte subVelSmpDl = 0;
byte subVelBias = 0;
byte subParallel = 0;
byte subRotator = 0;
byte subPriority = 0;
byte subWave = 0;
byte subCutoff = 0;
byte subReso = 0;
byte subFilter = 0;
byte subVolume = 0;

byte ccList[9] = {0,1,2,7,11,1,2,7,11};  // OFF, Modulation, Breath, Volume, Expression (then same sent in hires)

int pbDepthList[13] = {0,8192,4096,2731,2048,1638,1365,1170,1024,910,819,744,683};

byte cursorNow;
byte forcePix = 0;
byte forceRedraw = 0;

int pos1;
int pos2;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;         // the last time the output pin was toggled
unsigned long debounceDelay = 30;           // the debounce time; increase if the output flickers
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
byte doPatchUpdate=0;

byte FPD = 0;

int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;
unsigned int oldbreathhires=0;
unsigned int breathCV=0;
float filterFreq = 30.0;

float filterVal = 0.15;
float smoothedVal;
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

byte oldpkey = 0;

float vibDepth[10] = {0,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.40,0.45}; // max pitch bend values (+/-) for the vibrato settings 

unsigned int curveM4[] = {0,4300,7000,8700,9900,10950,11900,12600,13300,13900,14500,15000,15450,15700,16000,16250,16383};
unsigned int curveM3[] = {0,2900,5100,6650,8200,9500,10550,11500,12300,13100,13800,14450,14950,15350,15750,16150,16383};
unsigned int curveM2[] = {0,2000,3600,5000,6450,7850,9000,10100,11100,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveM1[] = {0,1400,2850,4100,5300,6450,7600,8700,9800,10750,11650,12600,13350,14150,14950,15650,16383};
unsigned int curveIn[] = {0,1023,2047,3071,4095,5119,6143,7167,8191,9215,10239,11263,12287,13311,14335,15359,16383};
unsigned int curveP1[] = {0,600,1350,2150,2900,3800,4700,5600,6650,7700,8800,9900,11100,12300,13500,14850,16383};
unsigned int curveP2[] = {0,400,800,1300,2000,2650,3500,4300,5300,6250,7400,8500,9600,11050,12400,14100,16383};
unsigned int curveP3[] = {0,200,500,900,1300,1800,2350,3100,3800,4600,5550,6550,8000,9500,11250,13400,16383};
unsigned int curveP4[] = {0,100,200,400,700,1050,1500,1950,2550,3200,4000,4900,6050,7500,9300,12100,16383};
unsigned int curveS1[] = {0,600,1350,2150,2900,3800,4700,6000,8700,11000,12400,13400,14300,14950,15500,16000,16383};
unsigned int curveS2[] = {0,600,1350,2150,2900,4000,6100,9000,11000,12100,12900,13700,14400,14950,15500,16000,16383};
//unsigned int curveS3[] = {0,600,1350,2300,3800,6200,8700,10200,11100,12100,12900,13700,14400,14950,15500,16000,16383};
//unsigned int curveS4[] = {0,600,1700,4000,6600,8550,9700,10550,11400,12200,12900,13700,14400,14950,15500,16000,16383};

unsigned int curveZ1[] = {0,1400,2100,2900,3200,3900,4700,5600,6650,7700,8800,9900,11100,12300,13500,14850,16383};
unsigned int curveZ2[] = {0,2000,3200,3800,4096,4800,5100,5900,6650,7700,8800,9900,11100,12300,13500,14850,16383};

int vibThr;          // this gets auto calibrated in setup
int vibThrLo;
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
byte pinkyKey;
byte lastSpecialKey = 0;

byte slurSustain = 0;
byte parallelChord = 0;
byte subOctaveDouble = 0;

unsigned int breathLedBrightness = 7000; // up to 16383, PWM
unsigned int portamLedBrightness = 7000; // up to 16383, PWM

int internalNote = 0;
int internalPitchbend = 0;
//int ipb=8192;
//int oldipb=8192;
float cents = 0;
float cent = 1.000577789;
float internalVolume = 0.50;
float playVolume = 0;
byte intPatch = 1; // 0 = sine, 1 = saw

float midiToFreq[128]; // for storing pre calculated frequencies for note numbers


// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=167.77777862548828,86.66667079925537
AudioFilterStateVariable filter1;        //xy=285.4444389343262,245.8888816833496
AudioMixer4              mixer2;         //xy=408.88888931274414,128.77777862548828
AudioOutputAnalog        dac1;           //xy=588.444450378418,216.99999618530273
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(waveform1, 0, mixer2, 0);
AudioConnection          patchCord3(filter1, 0, mixer2, 1);
AudioConnection          patchCord4(mixer2, dac1);
// GUItool: end automatically generated code



Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor


//_______________________________________________________________________________________________ SETUP

void setup() {
  
  analogReadResolution(12);   // set resolution of ADCs to 12 bit
  analogWriteFrequency(A6,2929.687); // set PWM freq for CV out
  analogWriteResolution(14);  // set resolution of PWMs to 14 bit (match hi-res midi)
     
  pinMode(dPin, INPUT_PULLUP);
  pinMode(ePin, INPUT_PULLUP);
  pinMode(uPin, INPUT_PULLUP);
  pinMode(mPin, INPUT_PULLUP);

  pinMode(bLedPin, OUTPUT);   // breath indicator LED
  pinMode(pLedPin, OUTPUT);   // portam indicator LED
  pinMode(13,OUTPUT);         // Teensy onboard LED  

  
  // if stored settings are not for current version, or Enter+Menu are pressed at startup, they are replaced by factory settings
  
  if ((readSetting(VERSION_ADDR) != VERSION) && (readSetting(VERSION_ADDR) < 24) || (!digitalRead(ePin) && !digitalRead(mPin))){ 
    writeSetting(VERSION_ADDR,VERSION);
    writeSetting(BREATH_THR_ADDR,BREATH_THR_FACTORY);
    writeSetting(BREATH_MAX_ADDR,BREATH_MAX_FACTORY);
    writeSetting(PORTAM_THR_ADDR,PORTAM_THR_FACTORY);  
    writeSetting(PORTAM_MAX_ADDR,PORTAM_MAX_FACTORY); 
    writeSetting(PITCHB_THR_ADDR,PITCHB_THR_FACTORY);
    writeSetting(PITCHB_MAX_ADDR,PITCHB_MAX_FACTORY);
    writeSetting(EXTRAC_THR_ADDR,EXTRAC_THR_FACTORY);
    writeSetting(EXTRAC_MAX_ADDR,EXTRAC_MAX_FACTORY);
    writeSetting(CTOUCH_THR_ADDR,CTOUCH_THR_FACTORY);
  }
  
  if ((readSetting(VERSION_ADDR) != VERSION) || (!digitalRead(ePin) && !digitalRead(mPin))){
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

  activePatch = patch;
 
  breathStep = (breathHiLimit - breathLoLimit)/92; // 92 is the number of pixels in the settings bar
  portamStep = (portamHiLimit - portamLoLimit)/92;
  pitchbStep = (pitchbHiLimit - pitchbLoLimit)/92;   
  extracStep = (extracHiLimit - extracLoLimit)/92;
  ctouchStep = (ctouchHiLimit - ctouchLoLimit)/92;
  
  for(int i=0;i<128;i++) {  // set up table, midi note number to frequency
        midiToFreq[i] = numToFreq(i);
  }

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


  AudioMemory(50);
  
  dac1.analogReference(INTERNAL); // normal volume
  //dac1.analogReference(EXTERNAL); // louder

  //waveform1.arbitraryWaveform(AKWF_vgame_0006,172.00);
  //waveform1.arbitraryWaveform(AKWF_vgtri_0008,172.00);
  //waveform1.arbitraryWaveform(AKWF_violin_0010,172.00);
  //waveform1.arbitraryWaveform(AKWF_oboe_0006,172.00);
  //waveform1.arbitraryWaveform(AKWF_altosax_0013,172.00);
  //waveform1.arbitraryWaveform(AKWF_sinharm_0011,172.00);
  waveform1.arbitraryWaveform(AKWF_raw_0004,172.00);
  
  updateSynth();

  
  //auto-calibrate the vibrato "dead zone" while showing splash screen
  int cv1=touchRead(15);
  digitalWrite(13,HIGH);
  delay(250);
  int cv2=touchRead(15);
  digitalWrite(13,LOW);
  delay(250);
  int cv3=touchRead(15);
  digitalWrite(13,HIGH);
  delay(250);
  digitalWrite(13,LOW);
  int cv4=touchRead(15);
  vibThr=(cv1+cv2+cv3+cv4)/4-30;
  vibThrLo=(cv1+cv2+cv3+cv4)/4+30;
  delay(250);
  digitalWrite(13,HIGH);
  delay(250);
  digitalWrite(13,LOW);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(85,52);
  display.println("v.1.1.5");       // FIRMWARE VERSION NUMBER HERE <<<<<<<<<<<<<<<<<<<<<<<
  display.display();
  
  delay(2000); 
  
  state = DISPLAYOFF_IDL;
  mainState = NOTE_OFF;       // initialize main state machine

  if (!digitalRead(ePin)) {
    activePatch=0;                
    doPatchUpdate=1;
  }
  
  Serial3.begin(31250);   // start serial with midi baudrate 31250
  Serial3.flush();

  digitalWrite(13,HIGH); // Switch on the onboard LED to indicate power on/ready

  midiReset();
  
}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {
  mainLoop();
}

void mainLoop() {
  FilterOnePole breathFilter( LOWPASS, filterFreq );   // create a one pole (RC) lowpass filter
  while (1){
    breathFilter.input(analogRead(A0));
    pressureSensor = constrain((int)breathFilter.output(),0,4095); // Get the filtered pressure sensor reading from analog pin A0, input from sensor MP3V5004GP
    breathCV = breathCurve(map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,16383));
    analogWrite(A6,breathCV);
    //pressureSensor = analogRead(A0);
    //pressureSensor =  smooth(analogRead(0), filterVal, smoothedVal);   // second parameter determines smoothness  - 0 is off,  .9999 is max smooth 
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
      
      readSwitches();
      fingeredNote=noteValueCheck(fingeredNote);
      if (internalNote != fingeredNote) {
        internalNote = fingeredNote;
        internalFreqChange();
      }
      
      //gate always on - put code for note on here?
      
      if (pressureSensor > breathThrVal) {
        // Value has risen above threshold. Move to the RISE_WAIT
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
          if (!K1 && !K4 && !K5){
            slurSustain = 0;
            parallelChord = 0;
            subOctaveDouble = 0;
            rotatorOn = 0;
          }
          if (pinkyKey){
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
      if (pressureSensor > breathThrVal) {
        // Has enough time passed for us to collect our second
        // sample?
        if ((millis() - breath_on_time > velSmpDl) || (0 == velSmpDl)) {
          // Yes, so calculate MIDI note and velocity, then send a note on event
          readSwitches();
          // We should be at tonguing peak, so set velocity based on current pressureSensor value unless fixed velocity is set     
          breathLevel=constrain(max(pressureSensor,initial_breath_value),breathThrVal,breathMaxVal); 
          if (!velocity) {
            unsigned int breathValHires = breathCurve(map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,16383));
            velocitySend = (breathValHires >>7) & 0x007F;
            velocitySend = constrain(velocitySend+velocitySend*.1*velBias,1,127);
            //velocitySend = map(constrain(max(pressureSensor,initial_breath_value),breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,1,127);
          } else velocitySend = velocity;          
          breath(); // send breath data
          internalBreath();
          fingeredNote=noteValueCheck(fingeredNote);
          internalNote = fingeredNote;
          internalFreqChange();
          if (priority){ // mono prio to last chord note
            usbMIDI.sendNoteOn(fingeredNote, velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOn(fingeredNote, velocitySend, activeMIDIchannel - 1);
          }
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
          if (rotatorOn){
            usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+parallel), velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOn(noteValueCheck(fingeredNote+parallel), velocitySend, activeMIDIchannel - 1);
            if (currentRotation < 3) currentRotation++; else currentRotation = 0;
            usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+rotations[currentRotation]), velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOn(noteValueCheck(fingeredNote+rotations[currentRotation]), velocitySend, activeMIDIchannel - 1);
          }
          if (!priority){ // mono prio to base note
            usbMIDI.sendNoteOn(fingeredNote, velocitySend, activeMIDIchannel); // send Note On message for new note 
            dinMIDIsendNoteOn(fingeredNote, velocitySend, activeMIDIchannel - 1);
          }
          activeNote=fingeredNote;
          mainState = NOTE_ON;
        }
      } else {
        // Value fell below threshold before velocity sample delay time passed. Return to
        // NOTE_OFF state (e.g. we're ignoring a short blip of breath)
        mainState = NOTE_OFF;
      }
    } else if (mainState == NOTE_ON) {
      cursorBlinkTime = millis(); // keep display from updating with cursor blinking if note on
      if (pressureSensor < breathThrVal) {
        // Value has fallen below threshold - turn the note off
        activeNote=noteValueCheck(activeNote);
        if (priority){
          usbMIDI.sendNoteOff(activeNote, velocitySend, activeMIDIchannel); //  send Note Off message 
          dinMIDIsendNoteOff(activeNote, velocitySend, activeMIDIchannel - 1);
        }
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
        if (rotatorOn){
          usbMIDI.sendNoteOff(noteValueCheck(activeNote+parallel), velocitySend, activeMIDIchannel); // send Note Off message for old note
          dinMIDIsendNoteOff(noteValueCheck(activeNote+parallel), velocitySend, activeMIDIchannel - 1);
          usbMIDI.sendNoteOff(noteValueCheck(activeNote+rotations[currentRotation]), velocitySend, activeMIDIchannel); // send Note Off message for old note
          dinMIDIsendNoteOff(noteValueCheck(activeNote+rotations[currentRotation]), velocitySend, activeMIDIchannel - 1);
        }     
        if (!priority){
          usbMIDI.sendNoteOff(activeNote, velocitySend, activeMIDIchannel); //  send Note Off message 
          dinMIDIsendNoteOff(activeNote, velocitySend, activeMIDIchannel - 1);
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
              velocitySend = constrain(velocitySend+velocitySend*.1*velBias,1,127);
              //velocitySend = map(constrain(pressureSensor,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,7,127); // set new velocity value based on current pressure sensor level
            }
            activeNote=noteValueCheck(activeNote);
            if ((parallelChord || subOctaveDouble || rotatorOn) && priority){ // poly playing, send old note off before new note on
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
            if (rotatorOn){
              usbMIDI.sendNoteOff(noteValueCheck(activeNote+parallel), velocitySend, activeMIDIchannel); // send Note Off message for old note
              dinMIDIsendNoteOff(noteValueCheck(activeNote+parallel), velocitySend, activeMIDIchannel - 1);
              usbMIDI.sendNoteOff(noteValueCheck(activeNote+rotations[currentRotation]), velocitySend, activeMIDIchannel); // send Note Off message for old note
              dinMIDIsendNoteOff(noteValueCheck(activeNote+rotations[currentRotation]), velocitySend, activeMIDIchannel - 1);
            }
            if ((parallelChord || subOctaveDouble || rotatorOn) && !priority){ // poly playing, send old note off before new note on
              usbMIDI.sendNoteOff(activeNote, velocitySend, activeMIDIchannel); // send Note Off message for old note
              dinMIDIsendNoteOff(activeNote, velocitySend, activeMIDIchannel - 1);
            }
            
            
            fingeredNote=noteValueCheck(fingeredNote);
            internalNote = fingeredNote;
            internalFreqChange();
            if (priority){
              usbMIDI.sendNoteOn(fingeredNote, velocitySend, activeMIDIchannel); // send Note On message for new note 
              dinMIDIsendNoteOn(fingeredNote, velocitySend, activeMIDIchannel - 1);
            }
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
            if (rotatorOn){
              usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+parallel), velocitySend, activeMIDIchannel); // send Note On message for new note 
              dinMIDIsendNoteOn(noteValueCheck(fingeredNote+parallel), velocitySend, activeMIDIchannel - 1);
              if (currentRotation < 3) currentRotation++; else currentRotation = 0;
              usbMIDI.sendNoteOn(noteValueCheck(fingeredNote+rotations[currentRotation]), velocitySend, activeMIDIchannel); // send Note On message for new note 
              dinMIDIsendNoteOn(noteValueCheck(fingeredNote+rotations[currentRotation]), velocitySend, activeMIDIchannel - 1);
            }

            if (!priority){
              usbMIDI.sendNoteOn(fingeredNote, velocitySend, activeMIDIchannel); // send Note On message for new note 
              dinMIDIsendNoteOn(fingeredNote, velocitySend, activeMIDIchannel - 1);
            }
            
            if (!parallelChord && !subOctaveDouble && !rotatorOn){ // mono playing, send old note off after new note on
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
      internalBreath();
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
      // -4
      return multiMap(inputVal,curveIn,curveM4,17);
      break;
    case 1:
      // -3
      return multiMap(inputVal,curveIn,curveM3,17);
      break;
    case 2:
      // -2
      return multiMap(inputVal,curveIn,curveM2,17);
      break;
    case 3:
      // -1
      return multiMap(inputVal,curveIn,curveM1,17);
      break;
    case 4:
      // 0, linear
      return inputVal;
      break;
    case 5:
      // +1
      return multiMap(inputVal,curveIn,curveP1,17);
      break;
    case 6:
      // +2
      return multiMap(inputVal,curveIn,curveP2,17);
      break;
    case 7:
      // +3
      return multiMap(inputVal,curveIn,curveP3,17);
      break;
    case 8:
      // +4
      return multiMap(inputVal,curveIn,curveP4,17);
      break;
    case 9:
      // S1
      return multiMap(inputVal,curveIn,curveS1,17);
      break;
    case 10:
      // S2
      return multiMap(inputVal,curveIn,curveS2,17);
      break;
    case 11:
      // Z1
      return multiMap(inputVal,curveIn,curveZ1,17);
      break;
    case 12:
      // Z2
      return multiMap(inputVal,curveIn,curveZ2,17);
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

// MIDI note number to frequency calculation
float numToFreq(int input) {
    int number = input - 21; // set to midi note numbers = start with 21 at A0 
    number = number - 60; // A0 is 48 steps below A4 = 440hz
    return 440*(pow (1.059463094359,number));
}

//**************************************************************

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

void midiPanic(){ // all notes off
  usbMIDI.sendControlChange(123, 0, activeMIDIchannel);
  dinMIDIsendControlChange(123, 0, activeMIDIchannel - 1);
}

//**************************************************************

void midiReset(){ // reset all controllers
  usbMIDI.sendControlChange(121, 0, activeMIDIchannel);
  dinMIDIsendControlChange(121, 0, activeMIDIchannel - 1);
  usbMIDI.sendControlChange(7, 100, activeMIDIchannel);
  dinMIDIsendControlChange(7, 100, activeMIDIchannel - 1);
  usbMIDI.sendControlChange(11, 127, activeMIDIchannel);
  dinMIDIsendControlChange(11, 127, activeMIDIchannel - 1);
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

void internalFreqChange(){
  if (midiToFreq[internalNote] > 20.0){
    waveform1.frequency(midiToFreq[internalNote]+internalPitchbend);
  }
}

//**************************************************************

void internalBreath(){
  int intBreath;
  breathLevel = constrain(pressureSensor,breathThrVal,breathMaxVal);
  intBreath = breathCurve(map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,1000*(cutoff+1)));
  if (0 == intPatch){
    playVolume = constrain(internalVolume * intBreath/10000, 0, internalVolume);
    //mixer2.gain(0, playVolume);
    //mixer2.gain(1, 0);
  } else {
    //playVolume = constrain(internalVolume * intBreath/1000, 0.5, internalVolume);
    //mixer2.gain(0, 0);
    //mixer2.gain(1, playVolume);
    filter1.frequency(intBreath);
  }
}

//**************************************************************

void updateSynth(){
  if (vol > 9) vol = 9; // 
  internalVolume = 0.05 * vol;
  if (0 == filter){ // internal
    mixer2.gain(0, 0.00);
    mixer2.gain(1, internalVolume);
  } else { //external
    mixer2.gain(0, internalVolume);
    mixer2.gain(1, 0.00);
  }
  mixer2.gain(2, 0.00);
  mixer2.gain(3, 0.00);
  delay(100);
  filter1.frequency(0); 
  filter1.resonance(0.7+0.1*reso);
  switch (wave){
    case 0:
      waveform1.begin(WAVEFORM_SAWTOOTH);
      waveform1.amplitude(0.3);
      break;
    case 1:
      waveform1.begin(WAVEFORM_SQUARE);
      waveform1.amplitude(0.3);
      break;
    case 2:
      waveform1.begin(WAVEFORM_ARBITRARY);
      waveform1.amplitude(0.6);
      break;
  }
  
}

//**************************************************************

void statusLEDs() {
  if (breathLevel > breathThrVal){ // breath indicator LED, labeled "B" on PCB
    analogWrite(bLedPin, map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,500,breathLedBrightness));
  } else {
    analogWrite(bLedPin, 0);
  }
  if (biteSensor > portamThrVal){ // portamento indicator LED, labeled "P" on PCB
    analogWrite(pLedPin, map(constrain(biteSensor,portamThrVal,portamMaxVal),portamThrVal,portamMaxVal,500,portamLedBrightness));
  } else {
    analogWrite(pLedPin, 0);
  }
}

//**************************************************************

void breath(){
  int breathCCval,breathCCvalFine;
  unsigned int breathCCvalHires;
  breathLevel = constrain(pressureSensor,breathThrVal,breathMaxVal);
  //breathLevel = breathLevel*0.6+pressureSensor*0.4; // smoothing of breathLevel value      
  ////////breathCCval = map(constrain(breathLevel,breathThrVal,breathMaxVal),breathThrVal,breathMaxVal,0,127);
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
  pbUp = touchRead(pbUpPin);                                                                 // SENSOR PIN 23 - PCB PIN "Pu"
  pbDn = touchRead(pbDnPin);                                                                 // SENSOR PIN 22 - PCB PIN "Pd"
  halfPitchBendKey = (pinkySetting == PBD) && (touchRead(halfPitchBendKeyPin) > touch_Thr);  // SENSOR PIN 1  - PCB PIN "S1" - hold for 1/2 pitchbend value
  int vibRead = touchRead(vibratoPin);                                                       // SENSOR PIN 15 - built in var cap
  if (PBdepth){
    calculatedPBdepth = pbDepthList[PBdepth];
    if (halfPitchBendKey) calculatedPBdepth = calculatedPBdepth*0.5;
  }
  if (((vibRead < vibThr) || (vibRead > vibThrLo))&&(vibRead > oldvibRead)){
    nudge = 0.01*constrain(abs(vibRead - oldvibRead),0,100);
    if (!dirUp){
      pitchBend=oldpb*(1-nudge)+nudge*(8192 + calculatedPBdepth*vibDepth[vibrato]);
      //ipb=oldipb*(1-nudge)+nudge*(8192 + 8192*vibDepth[vibrato]);
      vibratoMoved = 1;
    } else {
      pitchBend=oldpb*(1-nudge)+nudge*(8191 - calculatedPBdepth*vibDepth[vibrato]);
      //ipb=oldipb*(1-nudge)+nudge*(8192 - 8192*vibDepth[vibrato]);
      vibratoMoved = 1;
    }
  } else if (((vibRead < vibThr) || (vibRead > vibThrLo))&&(vibRead < oldvibRead)){
    nudge = 0.01*constrain(abs(vibRead - oldvibRead),0,100);
    if (!dirUp ){
      pitchBend=oldpb*(1-nudge)+nudge*(8191 - calculatedPBdepth*vibDepth[vibrato]);
      //ipb=oldipb*(1-nudge)+nudge*(8192 - 8192*vibDepth[vibrato]);
      vibratoMoved = 1;
    } else {
      pitchBend=oldpb*(1-nudge)+nudge*(8192 + calculatedPBdepth*vibDepth[vibrato]);
      //ipb=oldipb*(1-nudge)+nudge*(8192 + 8192*vibDepth[vibrato]);
      vibratoMoved = 1;
    }
  } else {
    vibratoMoved = 0;
  }

  oldvibRead = vibRead;

  if ((pbUp > pitchbThrVal) && PBdepth){
    pitchBend=pitchBend*0.6+0.4*map(constrain(pbUp,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,(8193 + calculatedPBdepth));
    //ipb=ipb*0.6+0.4*map(constrain(pbUp,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,16383);
  } else if ((pbDn > pitchbThrVal) && PBdepth){
    pitchBend=pitchBend*0.6+0.4*map(constrain(pbDn,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,(8192 - calculatedPBdepth));
    //ipb=ipb*0.6+0.4*map(constrain(pbDn,pitchbThrVal,pitchbMaxVal),pitchbThrVal,pitchbMaxVal,8192,0);
  } else if (oldvibRead > vibThr){
    pitchBend = pitchBend*0.6+8192*0.4; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) {
      pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
      //ipb = 8192;
    }
  } else if (!vibratoMoved){
    pitchBend = oldpb*0.8+8192*0.2; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) {
      pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
      //ipb = 8192;
    }
  }
  pitchBend=constrain(pitchBend, 0, 16383);
  //ipb=constrain(ipb, 0, 16383);
  
  if (pitchBend != oldpb){// only send midi data if pitch bend has changed from previous value 
    usbMIDI.sendPitchBend(pitchBend-8192, activeMIDIchannel); // newer teensyduino "pitchBend-8192" older just "pitchBend"... strange thing to change
    dinMIDIsendPitchBend(pitchBend, activeMIDIchannel - 1);
    cents = (pitchBend - 8192)/40;
    internalPitchbend = (midiToFreq[internalNote]*pow(cent,cents)) - midiToFreq[internalNote];
    internalFreqChange();
    
    oldpb=pitchBend;
  }
  /*if (ipb != oldipb){ // only alter signal data if pitch bend has changed from previous value 
    cents = (ipb - 8192)/40;
    internalPitchbend = (midiToFreq[internalNote]*pow(cent,cents)) - midiToFreq[internalNote];
    internalFreqChange();
    oldipb=ipb;
  }*/
}

//***********************************************************

void extraController(){
 // Extra Controller is the lip touch sensor (proportional) in front of the mouthpiece
 exSensor=exSensor*0.6+0.4*touchRead(extraPin);     // get sensor data, do some smoothing - SENSOR PIN 16 - PCB PIN "EC" (marked K4 on some prototype boards)
 if (extraCT && (exSensor >= extracThrVal)) {    // if we are enabled and over the threshold, send data
   if (!extracIsOn) {
     extracIsOn=1;
     if (extraCT == 4){ //Sustain ON
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
    if (extraCT == 3){ //Send filter cutoff (CC#74)
      int extracCC = map(constrain(exSensor,extracThrVal,extracMaxVal),extracThrVal,extracMaxVal,1,127); 
      if (extracCC != oldextrac){
        usbMIDI.sendControlChange(74,extracCC, activeMIDIchannel);
        dinMIDIsendControlChange(74,extracCC, activeMIDIchannel - 1);      
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
    } else if (extraCT == 3){ //FC
      //send foot pedal 0
      usbMIDI.sendControlChange(74,0, activeMIDIchannel);
      dinMIDIsendControlChange(74,0, activeMIDIchannel - 1);
      oldextrac = 0;
    } else if (extraCT == 4){ //SP
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
  int qTransp;
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

  pinkyKey = (touchRead(halfPitchBendKeyPin) > touch_Thr); // SENSOR PIN 1  - PCB PIN "S1" 

 if ((pinkySetting < 12) && pinkyKey){
  qTransp = pinkySetting - 12;
 } else if ((pinkySetting > 12) && pinkyKey){
  qTransp = pinkySetting - 12;
 } else {
  qTransp = 0;
 }


  // Calculate midi note number from pressed keys  
  fingeredNote=startNote-2*K1-K2-3*K3-5*K4+2*K5+K6+4*K7+octaveR*12+(octave-3)*12+transpose-12+qTransp;
}

//***********************************************************

int readTrills(){
  readSwitches();
  return K5+2*K6+4*K7;
}

//***********************************************************

void setFPS(int trills){
  fastPatch[trills-1] = patch;
  writeSetting(FP1_ADDR+2*(trills-1),patch);
  FPD = 2;
}

//***********************************************************

void clearFPS(int trills){
  fastPatch[trills-1] = 0;
  writeSetting(FP1_ADDR+2*(trills-1),0);
  FPD = 3;
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
    subPinky = 0;
    subVelSmpDl = 0;
    subVelBias = 0;
    subParallel = 0;
    subRotator = 0;
    subPriority = 0;
    subWave = 0;
    subCutoff = 0;
    subReso = 0;
    subFilter = 0;
    subVolume = 0;
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
          display.ssd1306_command(SSD1306_DISPLAYON);
          if (pinkyKey){
            state = ROTATOR_MENU;
          } else if (specialKey){
            state = SYNTH_MENU;
          } else {
            state = MAIN_MENU;
          }
          stateFirstRun = 1;
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
          } else if (!trills){
            if (patch > 1){
              patch--;
            } else patch = 128;
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
          } else if (!trills){
            if (patch < 128){
              patch++;
            } else patch = 1;
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
          FPD = 0;
          break;
        case 10:
          // enter + menu
            midiPanic();
            midiReset();
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
  } else if (state == SYNTH_MENU){    // SYNTH MENU HERE <<<<<<<<<<<<<<<
    if (stateFirstRun) {
      drawSynthMenuScreen();
      stateFirstRun = 0;
    }
    if (subWave){
      if (((millis() - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE; 
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotWave(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (wave > 0){
              plotWave(BLACK);
              wave--;
              plotWave(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotWave(WHITE);
            cursorNow = BLACK;
            display.display();
            subWave = 0;
            // if (readSetting(PARAL_ADDR) != (parallel + 24)) writeSetting(PARAL_ADDR,(parallel + 24));
            break;
          case 4:
            // up
            if (wave < 2){
              plotWave(BLACK);
              wave++;
              plotWave(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotWave(WHITE);
            cursorNow = BLACK;
            display.display();
            subWave = 0;
            //if (readSetting(PARAL_ADDR) != (parallel + 24)) writeSetting(PARAL_ADDR,(parallel + 24));
            break;
        }
        updateSynth();
      }  
    } else if (subCutoff){
      if (((millis() - cursorBlinkTime) > cursorBlinkInterval) || forceRedraw) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE; 
        if (forceRedraw){
          forceRedraw = 0;
          cursorNow = WHITE;
        }
        plotCutoff(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (cutoff > 0){
              plotCutoff(BLACK);
              cutoff--;
              plotCutoff(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotCutoff(WHITE);
            cursorNow = BLACK;
            display.display();
            //if (readSetting(ROTN1_ADDR+2*(subRotator-1)) != rotations[subRotator-1]) writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subCutoff = 0;
            break;
          case 4:
            // up
            if (cutoff < 9){
              plotCutoff(BLACK);
              cutoff++;
              plotCutoff(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotCutoff(WHITE);
            cursorNow = BLACK;
            display.display();
            //if (readSetting(ROTN1_ADDR+2*(subRotator-1)) != (rotations[subRotator-1]+24)) writeSetting(ROTN1_ADDR+2*(subRotator-1),(rotations[subRotator-1]+24));
            subCutoff = 0;
            break;
        }
        updateSynth();
      }
    } else if (subReso){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE; 
        plotReso(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (reso > 0){
              plotReso(BLACK);
              reso--;
              plotReso(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotReso(WHITE);
            cursorNow = BLACK;
            display.display();
            subReso = 0;
            // if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
          case 4:
            // up
            if (reso < 9){
              plotReso(BLACK);
              reso++;
              plotReso(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotReso(WHITE);
            cursorNow = BLACK;
            display.display();
            subReso = 0;
            // if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
        }
        updateSynth();
      }        
    } else if (subVolume){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE; 
        plotVol(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            if (vol > 0){
              plotVol(BLACK);
              vol--;
              plotVol(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 2:
            // enter
            plotVol(WHITE);
            cursorNow = BLACK;
            display.display();
            subVolume = 0;
            // if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
          case 4:
            // up
            if (vol < 9){
              plotVol(BLACK);
              vol++;
              plotVol(WHITE);
              cursorNow = BLACK;
              display.display();
              cursorBlinkTime = millis();
            }
            break;
          case 8:
            // menu
            plotVol(WHITE);
            cursorNow = BLACK;
            display.display();
            subVolume = 0;
            // if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
        }
        updateSynth();
      }
    } else if (subFilter){
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE; 
        plotFilter(cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        switch (deumButtonState){
          case 1:
            // down
            plotFilter(BLACK);
            filter = !filter;
            plotFilter(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 2:
            // enter
            plotFilter(WHITE);
            cursorNow = BLACK;
            display.display();
            subFilter = 0;
            //if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
          case 4:
            // up
            plotFilter(BLACK);
            filter = !filter;
            plotFilter(WHITE);
            cursorNow = BLACK;
            display.display();
            cursorBlinkTime = millis();
            break;
          case 8:
            // menu
            plotFilter(WHITE);
            cursorNow = BLACK;
            display.display();
            subFilter = 0;
            //if (readSetting(PRIO_ADDR) != priority) writeSetting(PRIO_ADDR,priority);
            break;
        }
        updateSynth();
      }          
    } else {
      if ((millis() - cursorBlinkTime) > cursorBlinkInterval) {
        if (cursorNow == WHITE) cursorNow = BLACK; else cursorNow = WHITE; 
        drawMenuCursor(synthMenuCursor, cursorNow);
        display.display();
        cursorBlinkTime = millis();
      }
      if (buttonPressedAndNotUsed){
        buttonPressedAndNotUsed = 0;
        int trills = readTrills();
        switch (deumButtonState){
          case 1:
            // down
            if (synthMenuCursor < 5){
              drawMenuCursor(synthMenuCursor, BLACK);
              synthMenuCursor++;
              drawMenuCursor(synthMenuCursor, WHITE);
              cursorNow = BLACK;
              clearSub();
              display.display();
            }
            break;
          case 2:
            // enter
            selectSynthMenu();
            break;
          case 4:
            // up
            if (synthMenuCursor > 1){
              drawMenuCursor(synthMenuCursor, BLACK);
              synthMenuCursor--;
              drawMenuCursor(synthMenuCursor, WHITE);
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
  // end synth menu  
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
            if (readSetting(BREATH_CC_ADDR) != breathCC) {
              writeSetting(BREATH_CC_ADDR,breathCC);
              midiReset();
            }
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
              velSmpDl-=5;
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
              velSmpDl+=5;
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

void selectSynthMenu(){
  switch (synthMenuCursor){
    case 1:
      subWave = 1;
      drawMenuCursor(synthMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubWave();
      break;
    case 2:
      subCutoff = 1;
      drawMenuCursor(synthMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubCutoff();
      break;
    case 3:
      subReso = 1;
      drawMenuCursor(synthMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubReso();
      break;
    case 4:
      subVolume = 1;
      drawMenuCursor(synthMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubVol();
      break;
    case 5:
      subFilter = 1;
      drawMenuCursor(synthMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubFilter();
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
      break;
    case 6:
      subPinky = 1;
      drawMenuCursor(setupCtMenuCursor, WHITE);
      display.display();
      cursorBlinkTime = millis();
      drawSubPinkyKey();
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
  display.print("MENU         ");
  int vMeterReading = analogRead(vMeterPin);
  if (vMeterReading > 3000) display.print("USB "); else display.print("BAT ");
  if (vMeterReading < 2294) display.print("LOW"); else {
    display.print(map(vMeterReading,0,3030,0,50)*0.1,1);
    display.print("V");
  }
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

void drawRotatorMenuScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("ROTATOR SETUP");
  display.drawLine(0,9,127,9,WHITE);
  display.setCursor(0,12);
  display.println("INTERVAL"); 
  display.setCursor(0,21);
  display.println("ROTATE 1");
  display.setCursor(0,30);
  display.println("ROTATE 2"); 
  display.setCursor(0,39);
  display.println("ROTATE 3"); 
  display.setCursor(0,48);
  display.println("ROTATE 4"); 
  display.setCursor(0,57);
  display.println("PRIORITY"); 
  drawMenuCursor(rotatorMenuCursor, WHITE);
  display.display();
}

void drawSynthMenuScreen(){
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("SYNTH SETUP");
  display.drawLine(0,9,127,9,WHITE);
  display.setCursor(0,12);
  display.println("WAVE"); 
  display.setCursor(0,21);
  display.println("CUTOFF");
  display.setCursor(0,30);
  display.println("RESO"); 
  display.setCursor(0,39);
  display.println("VOLUME"); 
  display.setCursor(0,48);
  display.println("FILTER"); 
  display.setCursor(0,57);
  display.println(""); 
  drawMenuCursor(synthMenuCursor, WHITE);
  display.display();
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
  
  if (priority){
    display.setCursor(83,33);
    //display.setCursor(79,33);
    display.println("LO"); 
  } else {
    display.setCursor(83,33);
    //display.setCursor(79,33);
    display.println("HI"); 
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

void drawSubWave(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(80,15);
  display.println("SHAPE");
  plotWave(WHITE);
  display.display();
}

void plotWave(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  switch (wave){
    case 0:
      display.setCursor(79,33);
      display.println("SAW");
      break;
    case 1:
      display.setCursor(79,33);
      display.println("SQR");
      break;
    case 2:
      display.setCursor(79,33);
      display.println("TRI");
      break;    
  } 
}

void drawSubCutoff(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(80,15);
  display.println("LEVEL");
  plotCutoff(WHITE);
  display.display();
}

void plotCutoff(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(cutoff); 
}

void drawSubReso(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(80,15);
  display.println("LEVEL");
  plotReso(WHITE);
  display.display();
}

void plotReso(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  display.setCursor(90,33);
  display.println(reso); 
}

void drawSubVol(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(80,15);
  display.println("LEVEL");
  plotVol(WHITE);
  display.display();
}

void plotVol(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  if (vol){
    display.setCursor(90,33);
    display.println(vol); 
  } else {
    display.setCursor(79,33);
    display.println("OFF"); 
  }
}

void drawSubFilter(){
  display.fillRect(63,11,64,52,BLACK);
  display.drawRect(63,11,64,52,WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(75,15);
  display.println("SELECT");
  plotFilter(WHITE);
  display.display();
}

void plotFilter(int color){
  display.setTextColor(color);
  display.setTextSize(2);
  if (!filter){
    display.setCursor(79,33);
    display.println("INT"); 
  } else {
    display.setCursor(79,33);
    display.println("EXT"); 
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
    display.println("FC");
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
  display.println("VEL DELAY"); 
  display.setCursor(0,57);
  display.println("VEL BIAS"); 

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
  display.println("PINKY KEY"); 

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


