#include <Arduino.h>
#include "hardware.h"
#include "globals.h"
#include "config.h"

// Do things with status LED.
void statusLedOn() {
  digitalWrite(statusLedPin, HIGH);
#if defined(SEAMUS)
  analogWrite(sLedPin, SPCKEY_LED_BRIGHTNESS);
#endif
}

void statusLedOff() {
  digitalWrite(statusLedPin, LOW);
#if defined(SEAMUS)
  analogWrite(sLedPin, 0);
#endif
}

void statusLed(bool state) {
  digitalWrite(statusLedPin, state);
#if defined(SEAMUS)
  analogWrite(sLedPin, state*SPCKEY_LED_BRIGHTNESS);
#endif
}

void statusLedFlip() {
  digitalWrite(statusLedPin, !digitalRead(statusLedPin));
#if defined(SEAMUS)
  if (digitalRead(statusLedPin)) analogWrite(sLedPin, SPCKEY_LED_BRIGHTNESS); else analogWrite(sLedPin, 0);
#endif
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
  #if defined(NURAD)
  if (exSensorIndicator){
    analogWrite(eLedPin, map(constrain(exSensorIndicator, 0, 127), 0, 127, MIN_LED_BRIGHTNESS, EXTCON_LED_BRIGHTNESS));
  } else {
    analogWrite(eLedPin, 0);
  }
  #endif
}

void ledMeter(byte indicatedValue){
  analogWrite(bLedPin, map(constrain(indicatedValue, 0, 127), 0, 127, 0, BREATH_LED_BRIGHTNESS)); // full glow at maximum value
  analogWrite(pLedPin, map(constrain(indicatedValue, 0, 127), 127, 0, 0, PORTAM_LED_BRIGHTNESS)); // full glow at minimum value
}
