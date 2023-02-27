
#ifndef __CONFIG_H
#define __CONFIG_H


// Compile options, comment/uncomment to change

#define FIRMWARE_VERSION "1.6.0"    // FIRMWARE VERSION NUMBER HERE <<<<<<<<<<<<<<<<<<<<<<<

#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define CCN_Port 5      // Controller number for portamento level
#define CCN_PortOnOff 65// Controller number for portamento on/off
#define CCN_PortSE02 9  // Controller number for portamento type on Roland SE-02

// Send breath CC data no more than every CC_BREATH_INTERVAL 
// milliseconds
#define CC_BREATH_INTERVAL 5
#define SLOW_MIDI_ADD 7
#define CC_INTERVAL 9
#define CC_INTERVAL2 13
#define CC_INTERVAL3 37
#define LVL_TIMER_INTERVAL 15
#define CVPORTATUNE 2

#define maxSamplesNum 120

#define breathLoLimit   0
#define breathHiLimit   4095
#define portamLoLimit   700
#define portamHiLimit   4700
#define pitchbLoLimit   500
#define pitchbHiLimit   4000
#define extracLoLimit   500
#define extracHiLimit   4000
#define ctouchLoLimit   50
#define ctouchHiLimit   350
#define ttouchLoLimit   50
#define ttouchHiLimit   1900
#define leverLoLimit    1400
#define leverHiLimit    2000


#define MIN_LED_BRIGHTNESS    5   // lowest PWM value that still is visible
#define BREATH_LED_BRIGHTNESS 600 // up to 4095, PWM
#define PORTAM_LED_BRIGHTNESS 300 // up to 4095, PWM
#define EXTCON_LED_BRIGHTNESS 300 // up to 4095, PWM
#define SPCKEY_LED_BRIGHTNESS 700 // up to 4095, PWM

#define ALK_BAT_FULL 2800 // about 4.6V
#define NMH_BAT_FULL 2380 // about 3.9V
#define LIP_BAT_FULL 2540 // about 4.2V
#define ALK_BAT_LOW 2300  // about 3.8V
#define NMH_BAT_LOW 2200  // about 3.6V
#define LIP_BAT_LOW 2250  // about 3.7V


#endif
