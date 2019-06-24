
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include "EEPROM.h"

// TODO: Fake eeprom a bit better, maybe even save to file.

EEPROMClass::EEPROMClass() {
    memset(someFakeEEPROM_memory, 0xff, sizeof(someFakeEEPROM_memory));
    storage = NULL;
    autoUpdate = false;
}


uint8_t EEPROMClass::read( int idx )
{
    printf("Reading EEPROM address %u: %u\n", idx, 0xff&someFakeEEPROM_memory[idx]);
    return someFakeEEPROM_memory[idx];
}


void  EEPROMClass::write( int idx, uint8_t val )
{
    printf("Writing to EEPROM address %u = %u\n", idx, val);
    someFakeEEPROM_memory[idx] = val;

    if(autoUpdate && storage)
    {
        fseek(storage, idx, SEEK_SET);
        fputc(val, storage);
        fflush(storage);
    }
}

void  EEPROMClass::update( int idx, uint8_t val )
{
    write(idx, val);
}

uint16_t EEPROMClass::length()
{
    return sizeof(someFakeEEPROM_memory);
}

int16_t EEPROMClass::setStorage(const char* filename, bool write)
{

	//Close any open storage file
	if(storage)
	{
		fclose(storage);
		storage = NULL;
	}

    autoUpdate = write;

	storage = fopen(filename, "rb");
	

    //If only reading, fail if file does not exist (makes no sense otherwise)
    if(!storage && !autoUpdate) {
		printf("Could not open EEPROM storage file: '%s'\n", filename);
		return -1;
	}

    if(storage)
    {
        printf("Reading EEPROM storage file: '%s'\n", filename);
        rewind(storage);
        fread(someFakeEEPROM_memory, sizeof(someFakeEEPROM_memory), 1, storage);
    }

    if(!autoUpdate)
    {
        //No need for the file anymore, close it
        fclose(storage);
        storage = NULL;
    }

    //Create file if it doesn't exist (so we can write to it)
    if(!storage && autoUpdate)
    {
        storage = fopen(filename, "wb");
        if(!storage)
        {
            printf("Could not create EEPROM storage file: '%s'\n", filename);
            autoUpdate = false;
            return -2;
        }
    }

    if(storage && autoUpdate)
    {
        //Reopen file for writing without overwriting it
        storage = freopen(filename, "r+b", storage);

        if(!storage)
        {
            printf("Could not access EEPROM storage file for writing: '%s'\n", filename);
            autoUpdate = false;
            return -3;
        }

        printf("Writing any EEPROM changes to '%s'\n", filename);
    }

    return 0;
}

void EEPROMClass::closeStorage() {
    if(storage!=NULL)
    {
        fclose(storage);
        storage=NULL;
    }
}
