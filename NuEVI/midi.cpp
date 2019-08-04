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

// Send sysex commands to wireless module
void dinMIDIsendSysex(const uint8_t data[], const uint8_t length) {
  MIDI_SERIAL.write(0xF0); //Sysex command
  for(int i=0; i<length; ++i) {
    MIDI_SERIAL.write(data[i]);
  }
  MIDI_SERIAL.write(0xF7); //Sysex end
}

void sendWLPower(const uint8_t level) {
  uint8_t buf[6] = {
    0x00, 0x21, 0x11,  //Manufacturer id
    0x02,             //TX02
    0x02,             //Set power level
    0x00              //Power level value (0-3)
  };

  if(level>3) return; //Don't send invalid values

  buf[5] = level;
  dinMIDIsendSysex(buf, 6);

}

void sendWLChannel(const uint8_t channel) {
  uint8_t buf[6] = {
    0x00, 0x21, 0x11,  //Manufacturer id
    0x02,             //TX02
    0x05,             //Set channel
    0x04              //Channel value (4-80)
  };

  if(channel<4 || channel>80) return; //Don't send invalid values

  buf[5] = channel;
  dinMIDIsendSysex(buf, 6);

}

//Translate between "midi data" (only use 7 LSB per byte, big endian) and "teensy data" (little endian)
//Only 14 LSB of int value are used (2MSB are discarded), so only works for unsigned data 0-16383

//NOTE: This assumes code is running on a little-endian CPU, both for real device (Teensy) and simulator.
uint16_t midi16to14(const uint16_t realdata) {
  return (realdata & 0x3F80) >>7 | (realdata & 0x007F) <<8;
}

uint16_t midi14to16(const uint16_t mididata) {
  return (mididata & 0x7F00) >> 8 | (mididata & 0x007F) <<7 ;
}

//Read from a memory location, such as MIDI receive buffer
uint16_t midi14to16(const uint8_t* mididata) {
  uint8_t msb = *mididata;
  uint8_t lsb = *(mididata+1);

  return (msb & 0x007F) <<7 | (lsb & 0x007F);
}

//This is a bit different. MSB of each byte is just discarded (instead of discarding MSB for whole value). Just used for CRC (easier to compare)
uint32_t midi32to28(const uint32_t realdata) {
  uint8_t* p = (uint8_t*)&realdata;

  uint32_t r=0;
  for(int i=0; i<4; ++i) {
    r = r<<8 | (p[i] & 0x7F);
  }
  return r;
}
