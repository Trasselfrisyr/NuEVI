#include <Arduino.h>

#include "midi.h"
#include "hardware.h"

int midiChannel;


void midiSetChannel(uint8_t channel) {
  midiChannel = constrain(channel, 1, 16);
}

byte midiGetChannel() {
  return midiChannel;
}


void midiSendProgramChange(uint8_t patch) {
	usbMIDI.sendProgramChange(patch-1, midiChannel);
	dinMIDIsendProgramChange(patch-1, midiChannel-1);
}

void midiSendControlChange(uint8_t ccParam, uint8_t ccValue) {
      usbMIDI.sendControlChange(ccParam, ccValue, midiChannel);
      dinMIDIsendControlChange(ccParam, ccValue, midiChannel - 1);
}

void midiSendNoteOn(uint8_t note, uint8_t velocity) {
  usbMIDI.sendNoteOn(note, velocity, midiChannel);
  dinMIDIsendNoteOn(note, velocity, midiChannel - 1);
}

void midiSendNoteOff(uint8_t note) {
  //Always send velocity 0 on note off to avoid confusing some synthesizers
  usbMIDI.sendNoteOn(note, 0, midiChannel);
  dinMIDIsendNoteOn(note, 0, midiChannel - 1);
}


void midiSendAfterTouch(uint8_t value) {
  usbMIDI.sendAfterTouch(value, midiChannel);
  dinMIDIsendAfterTouch(value, midiChannel - 1);
}


void midiSendPitchBend(uint16_t value) {
    #if (TEENSYDUINO >= 141)
    usbMIDI.sendPitchBend(value-8192, midiChannel); // newer teensyduino "pitchBend-8192" older just "pitchBend"... strange thing to change
    #else
    usbMIDI.sendPitchBend(value, midiChannel);
    #endif
    dinMIDIsendPitchBend(value, midiChannel - 1);
}

void midiDiscardInput()
{
  while (usbMIDI.read()) {
    // read & ignore incoming messages
  }
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

void midiInitialize(uint8_t channel) {
  MIDI_SERIAL.begin(31250);   // start serial with midi baudrate 31250
  MIDI_SERIAL.flush();
  midiSetChannel(channel);
}



//Serial midi functions

//  Send a three byte din midi message
void midiSend3B(uint8_t midistatus, uint8_t data1, uint8_t data2) {
  MIDI_SERIAL.write(midistatus);
  MIDI_SERIAL.write(data1);
  MIDI_SERIAL.write(data2);
}

//**************************************************************

//  Send a two byte din midi message
void midiSend2B(uint8_t midistatus, uint8_t data) {
  MIDI_SERIAL.write(midistatus);
  MIDI_SERIAL.write(data);
}

//**************************************************************

//  Send din pitchbend
void dinMIDIsendPitchBend(uint16_t pb, uint8_t ch) {
    int pitchLSB = pb & 0x007F;
    int pitchMSB = (pb >>7) & 0x007F;
    midiSend3B((0xE0 | ch), pitchLSB, pitchMSB);
}

//**************************************************************

//  Send din control change
void dinMIDIsendControlChange(uint8_t ccNumber, uint8_t cc, uint8_t ch) {
    midiSend3B((0xB0 | ch), ccNumber, cc);
}

//**************************************************************

//  Send din note on
void dinMIDIsendNoteOn(uint8_t note, uint8_t vel, uint8_t ch) {
    midiSend3B((0x90 | ch), note, vel);
}

//**************************************************************

//  Send din note off
void dinMIDIsendNoteOff(uint8_t note, uint8_t vel, uint8_t ch) {
    midiSend3B((0x80 | ch), note, vel);
}

//**************************************************************

//  Send din aftertouch
void dinMIDIsendAfterTouch(uint8_t value, uint8_t ch) {
    midiSend2B((0xD0 | ch), value);
}

//**************************************************************

//  Send din program change
void dinMIDIsendProgramChange(uint8_t value, uint8_t ch) {
    midiSend2B((0xC0 | ch), value);
}
