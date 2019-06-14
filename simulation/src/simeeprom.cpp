
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include "EEPROM.h"

// TODO: Fake eeprom a bit better, maybe even save to file.

EEPROMClass::EEPROMClass() {
    memset(someFakeEEPROM_memory, 0xff, sizeof(someFakeEEPROM_memory));
}


uint8_t EEPROMClass::read( int idx )
{
    printf("Reading EEPROM address %d: %d\n", idx, someFakeEEPROM_memory[idx]);
    return someFakeEEPROM_memory[idx];
}


void  EEPROMClass::write( int idx, uint8_t val )
{
    printf("Writing to EEPROM address %d = %d\n", idx, val);
    someFakeEEPROM_memory[idx] = val;
}

void  EEPROMClass::update( int idx, uint8_t val )
{
    write(idx, val);
}
uint16_t EEPROMClass::length()
{
    return sizeof(someFakeEEPROM_memory);
}

// TODO: Add missing functioality..