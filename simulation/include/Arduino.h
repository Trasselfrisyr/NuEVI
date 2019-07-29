#ifndef __ARDUINO_H__
#define __ARDUINO_H__

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "Wiring.h"

#include "core_pins.h"

#define PROGMEM

#ifndef _BV
#define _BV(x) (1u<<(x))
#endif

// SPI CTRL REG BITS
#define SPIE 	7
#define SPE 	6
#define DORD 	5
#define MSTR 	4
#define CPOL 	3
#define CPHA 	2
#define SPR1 	1
#define SPR0 	0


#define A11 (0xa1)

/*
SPCR
| 7    | 6    | 5    | 4    | 3    | 2    | 1    | 0    |
| SPIE | SPE  | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 |

SPIE - Enables the SPI interrupt when 1
SPE - Enables the SPI when 1
DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
MSTR - Sets the Arduino in master mode when 1, slave mode when 0
CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
CPHA - Samples data on the falling edge of the data clock when 1, rising edge when 0
SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)
*/



#define LSBFIRST 0
#define MSBFIRST 1

typedef bool boolean;
typedef uint8_t byte;

// class Print
// {
// public:
// };


class SimSerial
{
public:
	void begin(uint32_t speed);
	void println(uint32_t value);
	void println();
	void println(const char* str);
	void print(const char* str);
	void print(uint32_t intValue);
	void write(const uint8_t str);
	void flush();

};

class SimUsbMidi
{
public:
	void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t cable=0);
	void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t cable=0);
	void sendPolyPressure(uint8_t note, uint8_t pressure, uint8_t channel, uint8_t cable=0);
	void sendAfterTouchPoly(uint8_t note, uint8_t pressure, uint8_t channel, uint8_t cable=0);
	void sendControlChange(uint8_t control, uint8_t value, uint8_t channel, uint8_t cable=0);
	void sendProgramChange(uint8_t program, uint8_t channel, uint8_t cable=0);
	void sendAfterTouch(uint8_t pressure, uint8_t channel, uint8_t cable=0);
	void sendPitchBend(int value, uint8_t channel, uint8_t cable=0);
	void sendSysEx(uint16_t length, const uint8_t *data, bool hasTerm=false, uint8_t cable=0);
	bool read(uint8_t channel=0);
	void setHandleSystemExclusive(__unused void (*fptr) (const uint8_t *array, uint8_t size));
	void setHandleSystemExclusive(__unused void (*fptr) (const uint8_t *data, uint16_t length, bool complete));
};

extern SimSerial Serial;
extern SimSerial Serial3; //Used for MIDI serial putput with default hardware
extern SimUsbMidi usbMIDI;

//extern void putString(int row, int col, int color, const char* msg, const FONT_INFO fontInfo);

uint32_t micros();
uint32_t millis();
void delay(uint32_t millis);
void delayMicroseconds(uint32_t micros);


void pinMode(uint8_t, uint8_t);

#ifdef __cplusplus
extern "C" {
#endif
//void digitalWrite(uint8_t, uint8_t);
//int digitalRead(uint8_t);

#ifdef __cplusplus
}
#endif

int analogRead(uint8_t);
void analogReference(uint8_t mode);
void analogWrite(uint8_t, int);

#endif
