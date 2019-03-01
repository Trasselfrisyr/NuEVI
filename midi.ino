#include "midi.h"
#include "hardware.h"

int midiChannel = 1;


void midiSetChannel(int channel) {
  midiChannel = constrain(channel, 1, 16);
}

int midiGetChannel() {
  return midiChannel;
}


void midiSendProgramChange(int patch) {
	usbMIDI.sendProgramChange(patch-1, midiChannel);
	dinMIDIsendProgramChange(patch-1, midiChannel-1);
}

void midiSendControlChange(int ccParam, int ccValue) {
      usbMIDI.sendControlChange(ccParam, ccValue, midiChannel);
      dinMIDIsendControlChange(ccParam, ccValue, midiChannel - 1);
}

void midiSendNoteOn(byte note, int velocity) {
  usbMIDI.sendNoteOn(note, velocity, midiChannel);
  dinMIDIsendNoteOn(note, velocity, midiChannel - 1);
}

void midiSendNoteOff(byte note) {
  //Always send velocity 0 on note off to avoid confusing some synthesizers
  usbMIDI.sendNoteOn(note, 0, midiChannel);
  dinMIDIsendNoteOn(note, 0, midiChannel - 1);
}


void midiSendAfterTouch(byte value) {
  usbMIDI.sendAfterTouch(value, midiChannel);
  dinMIDIsendAfterTouch(value, midiChannel - 1);
}


void midiSendPitchBend(int value) {
    #if defined(NEWTEENSYDUINO)
    usbMIDI.sendPitchBend(value-8192, midiChannel); // newer teensyduino "pitchBend-8192" older just "pitchBend"... strange thing to change
    #else
    usbMIDI.sendPitchBend(value, midiChannel);
    #endif
    dinMIDIsendPitchBend(value, midiChannel - 1);
}

void midiReset() { // reset controllers
  midiSendControlChange(7, 100);
  midiSendControlChange(11, 127);
}

void midiPanic() { // all notes off
  midiSendControlChange(123, 0);
  for (int i = 0; i < 128; i++){
    midiSendNoteOff(i);
    delay(2);
  }
}



//Serial midi functions

//  Send a three byte din midi message
void midiSend3B(byte midistatus, byte data1, byte data2) {
  MIDI_SERIAL.write(midistatus);
  MIDI_SERIAL.write(data1);
  MIDI_SERIAL.write(data2);
}

//**************************************************************

//  Send a two byte din midi message
void midiSend2B(byte midistatus, byte data) {
  MIDI_SERIAL.write(midistatus);
  MIDI_SERIAL.write(data);
}

//**************************************************************

//  Send din pitchbend
void dinMIDIsendPitchBend(int pb, byte ch) {
    int pitchLSB = pb & 0x007F;
    int pitchMSB = (pb >>7) & 0x007F;
    midiSend3B((0xE0 | ch), pitchLSB, pitchMSB);
}

//**************************************************************

//  Send din control change
void dinMIDIsendControlChange(byte ccNumber, int cc, byte ch) {
    midiSend3B((0xB0 | ch), ccNumber, cc);
}

//**************************************************************

//  Send din note on
void dinMIDIsendNoteOn(byte note, int vel, byte ch) {
    midiSend3B((0x90 | ch), note, vel);
}

//**************************************************************

//  Send din note off
void dinMIDIsendNoteOff(byte note, int vel, byte ch) {
    midiSend3B((0x80 | ch), note, vel);
}

//**************************************************************

//  Send din aftertouch
void dinMIDIsendAfterTouch(byte value, byte ch) {
    midiSend2B((0xD0 | ch), value);
}

//**************************************************************

//  Send din program change
void dinMIDIsendProgramChange(byte value, byte ch) {
    midiSend2B((0xC0 | ch), value);
}
