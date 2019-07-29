#include <cstdint>
#include <cstdio>


#include "Arduino.h"

/*************************************
 *	Stub simulation of Teensy usbMidi
 */


void SimUsbMidi::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::noteOff] note %03d vel %03d ch %02d\n", note, velocity, channel);
}

void SimUsbMidi::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::noteOn] note %03d vel %03d ch %02d\n", note, velocity, channel);
}

void SimUsbMidi::sendPolyPressure(uint8_t note, uint8_t pressure, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::polyPressure] note %03d p %03d ch %02d\n", note, pressure, channel);
}

void SimUsbMidi::sendAfterTouchPoly(uint8_t note, uint8_t pressure, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::afterTouchPoly] note %03d p %03d ch %02d\n", note, pressure, channel);
}

void SimUsbMidi::sendControlChange(uint8_t control, uint8_t value, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::controlChange] cc %03d val %03d ch %02d\n", control, value, channel);
}

void SimUsbMidi::sendProgramChange(uint8_t program, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::programChange] prg %03d ch %02d\n", program, channel);
}

void SimUsbMidi::sendAfterTouch(uint8_t pressure, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::afterTouch] p %03d ch %02d\n", pressure, channel);
}

void SimUsbMidi::sendPitchBend(int value, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::pitchBend] pb %05d ch %02d\n", value, channel);
}

void SimUsbMidi::sendSysEx(uint16_t length, const uint8_t __unused *data, bool __unused hasTerm, uint8_t __unused cable)
{
	printf( "[usbMIDI::sysEx] len %d\n", length);
}

bool SimUsbMidi::read(uint8_t __unused channel) {
	return false;
}

//Regular sysex handler
void SimUsbMidi::setHandleSystemExclusive(__unused void (*fptr) (const uint8_t *array, uint8_t size)) {

}

//"Chunked" sysex handler (teensy extension), for large messages
void SimUsbMidi::setHandleSystemExclusive(__unused void (*fptr) (const uint8_t *array, uint16_t size, bool last)) {

}
