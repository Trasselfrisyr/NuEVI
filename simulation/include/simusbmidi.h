#ifndef __SIMUSBMIDI_H__
#define __SIMUSBMIDI_H__

#include <string>

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
	void setHandleSystemExclusive(void (*fptr) (const uint8_t *array, unsigned int size));
	void setHandleSystemExclusive(void (*fptr) (const uint8_t *data, uint16_t length, bool complete));

//Things not part of Teensy USBMidi, but used to simulate sending data to it
	void receiveMidiData(const uint8_t *data, const uint16_t length); //Send midi data "into simulator"
	void setMidiFile(std::string filename); //MIDI data to send to device
	void triggerMidi(); //"Arm" so data is sent to device next time it tries to read anything
private:
	//Handlers registered to receive MIDI
	void (*usb_midi_handleSysExPartial)(const uint8_t *data, uint16_t length, uint8_t complete);
	void (*usb_midi_handleSysExComplete)(const uint8_t *data, unsigned int size);
	std::string midiFile;
	bool sendMidi;
};


#endif