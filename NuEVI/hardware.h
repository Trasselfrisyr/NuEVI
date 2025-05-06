#ifndef __HARDWARE_H
#define __HARDWARE_H
#endif

#define REVB
//#define NURAD
//#define SEAMUS
//#define LITE
//#define EVIR2

//#define I2CSCANNER

#if defined(NURAD) //NuRAD <<<<<<<<<<<<<<<<<<<<<<<
#if defined(LITE)  //NuRAD lite

// Pin definitions

// Teensy pins

//Analog pressure sensors. Breath and optional bite
#define breathSensorPin A0
#define bitePressurePin A7

//Digital pins for menu buttons
#define dPin 3
#define ePin 4
#define uPin 5
#define mPin 6

//Output pins for LEDs (breath, power, status)
#define bLedPin 10
#define pLedPin 9
#define eLedPin 22
#define sLedPin 23
#define statusLedPin 13

//DAC outputs for analog and pwm
#define dacPin 1 // PWM on pin 1 for pitch CV

//Pins for WIDI board management
#define widiJumperPin 30
#define widiJumperGndPin 31
#define widiPowerPin 33

//Analog input for measuring voltage
#define vMeterPin A6

//Which serial port to use for MIDI
#define MIDI_SERIAL Serial2
#define WIDI_SERIAL Serial7

// MPR121 Rollers 0x5D

#define rPin1 0
#define rPin2 1
#define rPin3 2
#define rPin4 3
#define rPin5 4
#define rPin6 5

// Lite version replacement pins from roller board
#define extraPin 9
#define pbUpPin 10
#define pbDnPin 8
#define vibratoPin 7 // pad
#define extraPin2 11

#define bitePin 17

// MPR121 RH 0x5C

#define RHsPin 3
#define RH1Pin 4
#define RH2Pin 2
#define RH3Pin 1
#define RHp1Pin 0
#define RHp2Pin 8
#define RHp3Pin 7
#define spec1Pin 10
#define spec2Pin 9
#define patchPin 5

// MPR121 LH 0x5B

#define LHsPin 8
#define LH1Pin 7
#define LHbPin 1
#define LH2Pin 9
#define LH3Pin 10
#define LHp1Pin 11
#define LHp2Pin 3
#define LHp3Pin 4


#else //Regular NuRAD
// Pin definitions

// Teensy pins

//Capacitive sensor pins (on-board teensy)
#define bitePin 17
#define extraPin 16
#define pbUpPin 1
#define pbDnPin 0
#define vibratoPin 15

//Analog pressure sensors. Breath and optional bite
#define breathSensorPin A0
#define bitePressurePin A7

//Digital pins for menu buttons
#define dPin 3
#define ePin 4
#define uPin 5
#define mPin 6

//Output pins for LEDs (breath, power, status)
#define bLedPin 10
#define pLedPin 9
#define eLedPin 22
#define sLedPin 23
#define statusLedPin 13

//Pins for WIDI board management
#define widiJumperPin 28
#define widiJumperGndPin 27
#define widiPowerPin 33

//Hacky solder in patch change pin
#define patchPinEVI 32
#define lockGlidePin 25

//Analog input for measuring voltage
#define vMeterPin A11

//DAC outputs for analog and pwm
#define dacPin A14
#define pwmDacPin A6

//Which serial port to use for MIDI
#define MIDI_SERIAL Serial3
#define WIDI_SERIAL Serial2

// MPR121 Rollers 0x5D

#define rPin1 0
#define rPin2 1
#define rPin3 2
#define rPin4 3
#define rPin5 4
#define rPin6 5

// MPR121 RH 0x5C

#define RHsPin 3
#define RH1Pin 4
#define RH2Pin 2
#define RH3Pin 1
#define RHp1Pin 0
#define RHp2Pin 8
#define RHp3Pin 7
#define spec1Pin 10
#define spec2Pin 9
#define patchPin 5

// MPR121 LH 0x5B

#define LHsPin 8
#define LH1Pin 7
#define LHbPin 1
#define LH2Pin 9
#define LH3Pin 10
#define LHp1Pin 11
#define LHp2Pin 3
#define LHp3Pin 4

#endif
#else //NuEVI <<<<<<<<<<<<<<<<<<<<<<<
#if defined(EVIR2)  //NuEVI R2

//Analog pressure sensors. Breath and optional bite
#define breathSensorPin A0
#define bitePressurePin A7

//Digital pins for menu buttons
#define dPin 3
#define ePin 4
#define uPin 5
#define mPin 6

//Pins jumpered to enable bite pressure sensor
#define biteJumperPin 11
#define biteJumperGndPin 12

//Output pins for LEDs (breath, power, status)
#define bLedPin 10
#define pLedPin 9
#define statusLedPin 13

//DAC outputs for analog and pwm
#define dacPin 1 // PWM on pin 1 for pitch CV

//Which serial port to use for MIDI
#define MIDI_SERIAL Serial2
#define WIDI_SERIAL Serial7

//Pins for WIDI board management
#define widiJumperPin 30
#define widiJumperGndPin 31
#define widiPowerPin 33

//Analog input for measuring voltage
#define vMeterPin A6

//Analog input for piezo vibrato
#define piezoPin A9

//MPR121 board main/roller
#define R1Pin 6
#define R2Pin 7
#define R3Pin 8
#define R4Pin 9
#define R5Pin 10
#define K4Pin 11
#define pbUpPin 4
#define pbDnPin 3
#define vibratoPin 5
#define extraPin 1
#define extraPin2 2

//MPR121 board key
#define K1Pin 11//0
#define K2Pin 10//1
#define K3Pin 9//2
#define K5Pin 2//9
#define K6Pin 1//10
#define K7Pin 0//11
#define specialKeyPin 4       // SK  7
#define halfPitchBendKeyPin 3 // PD  8
#define patchPinEVI 6
#define lockGlidePin 7

#else
// Pin definitions

// Teensy pins

#define specialKeyPin 0       // SK or S2
#define halfPitchBendKeyPin 1 // PD or S1


//Capacitive sensor pins (on-board teensy)
#define bitePin 17
#define extraPin 16
#define pbUpPin 23
#define pbDnPin 22
#define vibratoPin 15

//Pins jumpered to enable bite pressure sensor
#define biteJumperPin 11
#define biteJumperGndPin 12

//Pins for WIDI board management
#define widiJumperPin 28
#define widiJumperGndPin 27
#define widiPowerPin 33

//Hacky solder in patch change pin
#define patchPinEVI 32
#define lockGlidePin 25

//Analog pressure sensors. Breath and optional bite
#define breathSensorPin A0
#define bitePressurePin A7

//Digital pins for menu buttons
#define dPin 3
#define ePin 4
#define uPin 5
#define mPin 6

//Output pins for LEDs (breath, power, status)
#define bLedPin 10
#define pLedPin 9
#define statusLedPin 13

//Analog input for measuring voltage
#define vMeterPin A11

//DAC outputs for analog and pwm
#define dacPin A14
#define pwmDacPin 20

//Which serial port to use for MIDI
#define MIDI_SERIAL Serial3
#define WIDI_SERIAL Serial2

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

# else //REV A

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
#endif //EVIR2
#endif //NURAD

