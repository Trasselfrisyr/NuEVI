#ifndef __HARDWARE_H
#define __HARDWARE_H

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


#define biteJumperPin 11  //PBITE
#define biteJumperGndPin 12 //PBITE

#define breathSensorPin A0

#define dPin 3
#define ePin 4
#define uPin 5
#define mPin 6

#define bLedPin 10
#define pLedPin 9
#define statusLedPin 13

#define vMeterPin A11

#define dacPin A14
#define pwmDacPin 20

#define PBD 12
#define UPWD 1
#define DNWD 0

//Which serial port to use for MIDI
#define MIDI_SERIAL Serial3

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

#endif //REVB


#endif
