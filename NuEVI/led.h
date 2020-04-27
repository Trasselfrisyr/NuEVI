#ifndef __LED_H
#define __LED_H

void statusLedOn();
void statusLedOff();
void statusLedFlip();
void statusLed(bool state);
void statusLedFlash(uint16_t delayTime);
void statusLedBlink();
void updateSensorLEDs();
void ledMeter(byte indicatedValue);

#endif
