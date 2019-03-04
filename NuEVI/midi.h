#ifndef __MIDI_H
#define __MIDI_H

//Enable use of USB and serial MIDI
#define USE_MIDI_USB
#define USE_MIDI_SERIAL

//Set / get current midi channel
void midiSetChannel(byte channel);
byte midiGetChannel();

void midiSendProgramChange(int patch);
void midiSendControlChange(int ccParam, int ccValue);
void midiSendNoteOn(byte note, int velocity);
void midiSendNoteOff(byte note);
void midiSendAfterTouch(byte value);
void midiSendPitchBend(int value);


void midiReset(); // reset controllers
void midiPanic(); // turn all notes off

void midiInitialize(byte channel=1);

#endif
