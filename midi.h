#ifndef __MIDI_H
#define __MIDI_H

//Enable use of USB and serial MIDI
#define USE_MIDI_USB
#define USE_MIDI_SERIAL

//Define which serial device to use
#define MidiSerial Serial3

//Set / get current midi channel
void midiSetChannel(int channel);
int midiGetChannel();

void midiSendProgramChange(int patch);
void midiSendControlChange(int ccParam, int ccValue);
void midiSendNoteOn(byte note, int velocity);
void midiSendNoteOff(byte note);
void midiSendAfterTouch(byte value);
void midiSendPitchBend(int value);


void midiReset(); // reset controllers
void midiPanic(); // turn all notes off

void dinMIDIsendPitchBend(int pb, byte ch); //  Send din pitchbend
void dinMIDIsendAfterTouch(byte value, byte ch); //  Send din aftertouch

#endif