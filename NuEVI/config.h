
#ifndef __CONFIG_H
#define __CONFIG_H


// Compile options, comment/uncomment to change

#define FIRMWARE_VERSION "1.3.8"    // FIRMWARE VERSION NUMBER HERE <<<<<<<<<<<<<<<<<<<<<<<


//#define CASSIDY
#define CVSCALEBOARD

#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
//#define touch_Thr 1200  // sensitivity for Teensy touch sensors
#define CCN_Port 5      // Controller number for portamento level
#define CCN_PortOnOff 65// Controller number for portamento on/off

// Send breath CC data no more than every CC_INTERVAL (other CC is sent with double interval)
// milliseconds (due to timing errors, the value should be about half the actual wanted value)
#define CC_INTERVAL 2


// MAybe move these to config.h (as defines?)
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


#endif
