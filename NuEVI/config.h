
#ifndef __CONFIG_H
#define __CONFIG_H


// Compile options, comment/uncomment to change

#define FIRMWARE_VERSION "1.4.4"    // FIRMWARE VERSION NUMBER HERE <<<<<<<<<<<<<<<<<<<<<<<

#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define CCN_Port 5      // Controller number for portamento level
#define CCN_PortOnOff 65// Controller number for portamento on/off

// Send breath CC data no more than every CC_BREATH_INTERVAL 
// milliseconds (due to timing errors, the value should be about half the actual wanted value)
#define CC_BREATH_INTERVAL 1
#define SLOW_MIDI_ADD 7
#define CC_INTERVAL 13
#define CC_INTERVAL2 19
#define CC_INTERVAL3 37


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


#define MIN_LED_BRIGHTNESS    5   // lowest PWM value that still is visible
#define BREATH_LED_BRIGHTNESS 500 // up to 4095, PWM
#define PORTAM_LED_BRIGHTNESS 500 // up to 4095, PWM


#endif
