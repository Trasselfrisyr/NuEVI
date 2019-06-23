#include <Arduino.h>
#include "midi.h"

void midiSetChannel(byte __attribute__((unused)) channel){}

byte midiGetChannel(){ return 1; }

void midiSendProgramChange(int __attribute__((unused)) patch)
{

}

void midiSendControlChange(int __attribute__((unused)) ccParam, int __attribute__((unused)) ccValue)
{

}

void midiSendNoteOn(byte __attribute__((unused)) note, int __attribute__((unused)) velocity)
{

}

void midiSendNoteOff(byte __attribute__((unused)) note)
{

}

void midiSendAfterTouch(byte __attribute__((unused)) value)
{

}

void midiSendPitchBend(int __attribute__((unused)) value)
{

}

void midiDiscardInput()
{

}

void midiReset()
{

}

void midiPanic()
{
    // turn all notes off
}

void midiInitialize(byte __attribute__((unused)) channel)
{

}