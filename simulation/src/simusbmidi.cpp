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

//Set a low chunk size on purpose just to let the receiver work for it
#define MIDI_SYSEX_CHUNK_SIZE 32

/* Test data for config mode

//Carefully crafted config command chunk to send via midi
static const uint8_t midimessage[] = {
 0xf0,								//Sysex start
 0x00, 0x3e, 0x7f,					//Vendor
 'N', 'u', 'E', 'V', 'I',           //header
 'c', '0', '2',                     //message code
 0, 102,                            //length

 //Payload
 0x00, 0x20, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,  //00
 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x06, 0x07,  //08
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //10
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //18
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //20
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //28
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //30
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //38
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //40
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //48
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //50
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  //58
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,              //60
 0x2a, 0x11, 0x32, 0x5a,                          //crc32
 0xf7								//sysex end marker
};

static bool midisent = false;

//On first midi read, send a message
bool SimUsbMidi::read(uint8_t __unused channel) {
	if(!midisent) {
		this->receiveMidiData(midimessage, sizeof(midimessage));
		midisent=true;
	}
	return false;
}

*/

bool SimUsbMidi::read(uint8_t __unused channel) {
	return false;
}


//Provide midi data for simulation to receive
void SimUsbMidi::receiveMidiData(const uint8_t *data, const uint16_t length) {
	if(this->usb_midi_handleSysExPartial) {
		//Chunked sysex receiver set, use that.
		if(length<=MIDI_SYSEX_CHUNK_SIZE) {
			//Send all in one go
			printf( "[SimUsbMidi::receiveMidiData] usb_midi_handleSysExPartial(complete) %d B\n", length);
			(this->usb_midi_handleSysExPartial)(data, length, true);
		} else {
			uint8_t* buf = (uint8_t*)malloc(MIDI_SYSEX_CHUNK_SIZE);
			int pos=0;
			while(pos<length) {
				int remaining = length-pos;
				int bytesToSend = min(remaining, MIDI_SYSEX_CHUNK_SIZE);
				bool complete = (bytesToSend == remaining);

				memcpy(buf, data+pos, bytesToSend);
				printf( "[SimUsbMidi::receiveMidiData] usb_midi_handleSysExPartial(complete: %d) %d B\n", complete, bytesToSend);
				(this->usb_midi_handleSysExPartial)(buf, bytesToSend, complete);
				pos=pos+bytesToSend;
			}
			free(buf);
		}

	} else if(this->usb_midi_handleSysExComplete) {
		printf( "[SimUsbMidi::receiveMidiData] usb_midi_handleSysExComplete() %d B\n", length);
		(this->usb_midi_handleSysExComplete)(data, length);
	} else {
		//Nobody listening
	}
}

//Regular sysex handler. For some reason the data pointer is not const, but we'll set it as such to not be dumb.
void SimUsbMidi::setHandleSystemExclusive(void (*fptr) (const uint8_t *array, unsigned int size)) {
	this->usb_midi_handleSysExComplete = fptr;
}

//"Chunked" sysex handler (teensy extension), for large messages
void SimUsbMidi::setHandleSystemExclusive(void (*fptr) (const uint8_t *array, uint16_t size, uint8_t last)) {
	this->usb_midi_handleSysExPartial = fptr;
}
