#include <Arduino.h>
#include "hardware.h"
#include "globals.h"
#include "config.h"

// Do things with status LED.
void statusLedOn() {
  digitalWrite(statusLedPin, HIGH);
}

void statusLedOff() {
  digitalWrite(statusLedPin, LOW);
}

void statusLed(bool state) {
  digitalWrite(statusLedPin, state);    
}

void statusLedFlip() {
  digitalWrite(statusLedPin, !digitalRead(statusLedPin));
}

void statusLedFlash(uint16_t delayTime) {
  statusLedOff();
  delay(delayTime/2);
  statusLedOn();
  delay(delayTime/2);
}

void statusLedBlink() {
  statusLedFlash(300);
  statusLedFlash(300);
}

void updateSensorLEDs() {
  if (breathLevel > breathThrVal) { // breath indicator LED, labeled "B" on PCB
    //analogWrite(bLedPin, map(breathLevel,0,4096,5,breathLedBrightness));
    analogWrite(bLedPin, map(constrain(breathLevel, breathThrVal, breathMaxVal), breathThrVal, breathMaxVal, MIN_LED_BRIGHTNESS, BREATH_LED_BRIGHTNESS));
  } else {
    analogWrite(bLedPin, 0);
  }
  if (portIsOn) { // portamento indicator LED, labeled "P" on PCB
    //analogWrite(pLedPin, map(biteSensor,0,4096,5,portamLedBrightness));
    analogWrite(pLedPin, map(constrain(oldport, 0, 127), 0, 127, MIN_LED_BRIGHTNESS, PORTAM_LED_BRIGHTNESS));
  } else {
    analogWrite(pLedPin, 0);
  }
}
