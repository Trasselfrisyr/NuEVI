
#include <stdint.h>
#include "EEPROM.h"

// TODO: Fake eeprom a bit better, maybe even save to file.

static char someFakeEEPROM_memory[4096];


uint8_t EEPROMClass::read( int idx )
{
    return someFakeEEPROM_memory[idx];
}


void  EEPROMClass::write( int idx, uint8_t val )
{
    someFakeEEPROM_memory[idx] = val;
}

void  EEPROMClass::update( int idx, uint8_t val )
{
    someFakeEEPROM_memory[idx] = val;
}
uint16_t EEPROMClass::length()
{
    return sizeof(someFakeEEPROM_memory);
}

// TODO: Add missing functioality..