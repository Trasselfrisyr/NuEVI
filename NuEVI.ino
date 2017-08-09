#include <Wire.h>
#include <Adafruit_MPR121.h>

/*
NAME:                 NuEVI
WRITTEN BY:           JOHAN BERGLUND
DATE:                 2017-08-08
FILE SAVED AS:        NuEVI.ino
FOR:                  PJRC Teensy LC or 3.2 and a MPR121 capactive touch sensor board                                     
PROGRAMME FUNCTION:   EVI Wind Controller using the Freescale MP3V5004GP breath sensor
                      and capacitive touch keys. Output to both USB MIDI and DIN MIDI.  
  
*/

//_______________________________________________________________________________________________ DECLARATIONS

#define ON_Thr 370      // Set threshold level before switching ON
#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define breath_max 1023 // Maximum breath level
#define PB_sens 8191    // Pitch Bend sensitivity 0 to 8191 where 8191 is full pb range, 4095 half range
#define VIB_depth 1023  // Vibrato depth 0 to 8191
#define touch_Thr 1800  // sensitivity for Teensy touch sensors
#define CCN_Port 5      // Controller number for portamento level
#define CCN_PortOnOff 65// Controller number for portamento on/off


// Send CC data no more than every CC_INTERVAL
// milliseconds
#define CC_INTERVAL 5 

// The three states of our state machine

// No note is sounding
#define NOTE_OFF 1

// We've observed a transition from below to above the
// threshold value. We wait a while to see how fast the
// breath velocity is increasing
#define RISE_WAIT 2

// A note is sounding
#define NOTE_ON 3


//variables setup

int state;                         // The state of the state machine
unsigned long ccSendTime = 0L;     // The last time we sent CC values
unsigned long breath_on_time = 0L; // Time when breath sensor value went over the ON threshold
int initial_breath_value;          // The breath value at the time we observed the transition

unsigned long lastDebounceTime = 0;// The last time the fingering was changed
unsigned long debounceDelay = 10;  // The debounce time; increase if the output flickers
int lastFingering = 0;             // Keep the last fingering value for debouncing

byte MIDIchannel=1;                // MIDI channel 1

int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;

int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
byte velocity;       // remapped midi velocity from breath sensor

int biteSensor=0;    // capacitance data from bite sensor, for midi cc and threshold checks
byte portIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
int biteThr=1730;    // Set threshold level before switching ON (value to use if no pots installed)
int biteMax=3300;    // Upper limit for pressure (value to use if no pots installed)

int pitchBend=0;
int oldpb=8192;
int pbThr=1200; 
int pbMax=2400;  

int vibThr=1800;
int oldvibRead=0;
byte dirUp=0;        // direction of first vibrato wave

int fingeredNote;    // note calculated from fingering (switches) and octave joystick position
byte activeNote;     // note playing
byte startNote=36;   // set startNote to C (change this value in steps of 12 to start in other octaves)

Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor


//_______________________________________________________________________________________________ SETUP

void setup() {

  state = NOTE_OFF;       // initialize state machine
  if (!touchSensor.begin(0x5A)) {
    while (1);  // Touch sensor initialization failed - stop doing stuff
  }
  Serial3.begin(31250);   // start serial with midi baudrate 31250
  Serial3.flush();
  
}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {
  pressureSensor = analogRead(A0); // Get the pressure sensor reading from analog pin A0

  if (state == NOTE_OFF) {
    if (pressureSensor > ON_Thr) {
      // Value has risen above threshold. Move to the ON_Delay
      // state. Record time and initial breath value.
      breath_on_time = millis();
      initial_breath_value = pressureSensor;
      state = RISE_WAIT;  // Go to next state
    }
  } else if (state == RISE_WAIT) {
    if (pressureSensor > ON_Thr) {
      // Has enough time passed for us to collect our second
      // sample?
      if (millis() - breath_on_time > ON_Delay) {
        // Yes, so calculate MIDI note and velocity, then send a note on event
        readSwitches();
        //fingeredNote = startNote + 24;
        // We should be at tonguing peak, so set velocity based on current pressureSensor value        
        // If initial value is greater than value after delay, go with initial value, constrain input to keep mapped output within 1 to 127
        velocity = map(constrain(max(pressureSensor,initial_breath_value),ON_Thr,breath_max),ON_Thr,breath_max,1,127);
        breathLevel=constrain(max(pressureSensor,initial_breath_value),ON_Thr,breath_max);
        breath(); // send breath data
        usbMIDI.sendNoteOn(fingeredNote, velocity, MIDIchannel); // send Note On message for new note 
        dinMIDIsendNoteOn(fingeredNote, velocity, MIDIchannel - 1);
        activeNote=fingeredNote;
        state = NOTE_ON;
      }
    } else {
      // Value fell below threshold before ON_Delay passed. Return to
      // NOTE_OFF state (e.g. we're ignoring a short blip of breath)
      state = NOTE_OFF;
    }
  } else if (state == NOTE_ON) {
    if (pressureSensor < ON_Thr) {
      // Value has fallen below threshold - turn the note off
      usbMIDI.sendNoteOff(activeNote, velocity, MIDIchannel); //  send Note Off message 
      dinMIDIsendNoteOff(activeNote, velocity, MIDIchannel - 1);
      breathLevel=0;
      state = NOTE_OFF;
    } else {
      readSwitches();
      //fingeredNote = startNote + 24;
      if (fingeredNote != lastFingering){ //
        // reset the debouncing timer
        lastDebounceTime = millis();
      }
      if ((millis() - lastDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state
        if (fingeredNote != activeNote) {
          // Player has moved to a new fingering while still blowing.
          // Send a note off for the current note and a note on for
          // the new note.      
          velocity = map(constrain(pressureSensor,ON_Thr,breath_max),ON_Thr,breath_max,7,127); // set new velocity value based on current pressure sensor level
          usbMIDI.sendNoteOn(fingeredNote, velocity, MIDIchannel); // send Note On message for new note      
          dinMIDIsendNoteOn(fingeredNote, velocity, MIDIchannel - 1);   
          usbMIDI.sendNoteOff(activeNote, 0, MIDIchannel); // send Note Off message for previous note (legato)
          dinMIDIsendNoteOff(activeNote, 0, MIDIchannel - 1);
          activeNote=fingeredNote;
        }
      }
    }
  }
  // Is it time to send more CC data?
  if (millis() - ccSendTime > CC_INTERVAL) {
    // deal with Breath, Pitch Bend and Modulation
    breath();
    pitch_bend();
    portamento();
    ccSendTime = millis();
  }
  lastFingering=fingeredNote; 
}
//_______________________________________________________________________________________________ FUNCTIONS

//  Send a three byte din midi message  
void midiSend(byte midistatus, byte data1, byte data2) {
  Serial3.write(midistatus);
  Serial3.write(data1);
  Serial3.write(data2);
}

//**************************************************************

//  Send din pitchbend  
void dinMIDIsendPitchBend(int pb, byte ch) {
    int pitchLSB = pb & 0x007F;
    int pitchMSB = (pb >>7) & 0x007F; 
    midiSend((0xE0 | ch), pitchLSB, pitchMSB);
}

//**************************************************************

//  Send din control change  
void dinMIDIsendControlChange(byte ccNumber, int cc, byte ch) {
    midiSend((0xB0 | ch), ccNumber, cc);
}

//**************************************************************

//  Send din note on  
void dinMIDIsendNoteOn(byte note, int vel, byte ch) {
    midiSend((0x90 | ch), note, vel);
}

//**************************************************************

//  Send din note off 
void dinMIDIsendNoteOff(byte note, int vel, byte ch) {
    midiSend((0x80 | ch), note, vel);
}

//**************************************************************

void breath(){
  int breathCC;
  breathLevel = breathLevel*0.8+pressureSensor*0.2; // smoothing of breathLevel value
  breathCC = map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,127);
  if (breathCC != oldbreath){ // only send midi data if breath has changed from previous value
    usbMIDI.sendControlChange(2, breathCC, MIDIchannel);
    dinMIDIsendControlChange(2, breathCC, MIDIchannel - 1);
    oldbreath = breathCC;
  }  
}

//**************************************************************

void pitch_bend(){
  int pbUp = touchRead(23);
  int pbDn = touchRead(22);
  int vibRead = touchRead(1);
  if ((vibRead > vibThr)&&(vibRead > (oldvibRead+7))){
    if (dirUp){
      pitchBend=oldpb*0.7+0.3*(8192 + VIB_depth);
    } else {
      pitchBend=oldpb*0.7+0.3*(8191 - VIB_depth);
    }
  } else if ((vibRead > vibThr)&&(vibRead < (oldvibRead-7))){
    if (dirUp){
      pitchBend=oldpb*0.7+0.3*(8191 - VIB_depth);
    } else {
      pitchBend=oldpb*0.7+0.3*(8192 + VIB_depth);
    }
  } else {
    pitchBend = oldpb*0.7+8192*0.3; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
  }
  oldvibRead = vibRead;
  if (pbUp > pbThr){
    pitchBend=pitchBend*0.8+0.2*map(constrain(pbUp,pbThr,pbMax),pbThr,pbMax,8192,(8192 + PB_sens));
  } else if (pbDn > pbThr){
    pitchBend=pitchBend*0.8+0.2*map(constrain(pbDn,pbThr,pbMax),pbThr,pbMax,8192,(8191 - PB_sens));
  } else if (oldvibRead < vibThr){
    pitchBend = pitchBend*0.8+8192*0.2; // released, so smooth your way back to zero
    if ((pitchBend > 8187) && (pitchBend < 8197)) pitchBend = 8192; // 8192 is 0 pitch bend, don't miss it bc of smoothing
  }
  pitchBend=constrain(pitchBend, 0, 16383);
  if (pitchBend != oldpb){// only send midi data if pitch bend has changed from previous value
    usbMIDI.sendPitchBend(pitchBend, MIDIchannel);
    dinMIDIsendPitchBend(pitchBend, MIDIchannel - 1);
    oldpb=pitchBend;
  }
}

//***********************************************************

void portamento(){
 biteSensor=biteSensor*0.6+0.4*touchRead(0);     // get sensor data, do some smoothing
 if (biteSensor >= biteThr) {                   // if we are over the threshold, send portamento
   if (!portIsOn) {
     portOn();
    }
    port();        
  } else if (portIsOn) {                        // we have just gone below threshold, so send zero value
    portOff(); 
  }
}

//***********************************************************

void portOn(){
  usbMIDI.sendControlChange(CCN_PortOnOff, 127, MIDIchannel);
  dinMIDIsendControlChange(CCN_PortOnOff, 127, MIDIchannel - 1);
  portIsOn=1;
}

//***********************************************************

void port(){
  int portCC;
  portCC = map(constrain(biteSensor,biteThr,biteMax),biteThr,biteMax,1,127);
  usbMIDI.sendControlChange(CCN_Port, portCC, MIDIchannel);
  dinMIDIsendControlChange(CCN_Port, portCC, MIDIchannel - 1);
}

//***********************************************************

void portOff(){
  usbMIDI.sendControlChange(CCN_Port, 0, MIDIchannel);
  dinMIDIsendControlChange(CCN_Port, 0, MIDIchannel - 1);
  usbMIDI.sendControlChange(CCN_PortOnOff, 0, MIDIchannel);
  dinMIDIsendControlChange(CCN_PortOnOff, 0, MIDIchannel - 1);
  portIsOn=0;
}

//***********************************************************

void readSwitches(){  
  
  // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
  byte K1;   // Valve 1 (pitch change -2) 
  byte K2;   // Valve 2 (pitch change -1)
  byte K3;   // Valve 3 (pitch change -3)
  byte K4;   // Left Hand index finger (pitch change -5)
  byte K5;   // Trill key 1 (pitch change +2)
  byte K6;   // Trill key 2 (pitch change +1)
  byte K7;   // Trill key 3 (pitch change +4)

  byte octave = 0;

  // Read touch pads (MPR121) and put value in variables
  uint16_t touchValue = touchSensor.touched();
  
  // Octave rollers
  if      ((touchValue >> 5) & 0x01) octave = 6;
  else if ((touchValue >> 4) & 0x01) octave = 5;
  else if ((touchValue >> 3) & 0x01) octave = 4;
  else if ((touchValue >> 2) & 0x01) octave = 3;
  else if ((touchValue >> 1) & 0x01) octave = 2;
  else if ((touchValue >> 0) & 0x01) octave = 1;
  
  // Valves and trill keys
  K1=((touchValue >> 6) & 0x01);
  K2=((touchValue >> 7) & 0x01);
  K3=((touchValue >> 8) & 0x01);
  K5=((touchValue >> 9) & 0x01);
  K6=((touchValue >> 10) & 0x01);
  K7=((touchValue >> 11) & 0x01); 
  // Read touch pads (Teensy built in) and put value in variables
  K4=touchRead(15) > touch_Thr;

  // Calculate midi note number from pressed keys  
  fingeredNote=startNote-2*K1-K2-3*K3-5*K4+2*K5+K6+4*K7+octave*12;
}



