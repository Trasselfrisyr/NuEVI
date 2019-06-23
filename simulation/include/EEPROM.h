#ifndef __EEPROM_H
#define __EEPROM_H

#include <stdint.h>

struct EEPROMClass
{
    EEPROMClass();
    //Basic user access methods.
   //  EERef operator[]( const int idx )    { return idx; }
    uint8_t read( int idx ); //            { return EERef( idx ); }
    void write( int idx, uint8_t val ); //   { (EERef( idx )) = val; }
    void update( int idx, uint8_t val ); //  { EERef( idx ).update( val ); }

    //STL and C++11 iteration capability.
    // EEPtr begin()                        { return 0x00; }
    // EEPtr end()                          { return length(); } //Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
    uint16_t length(); //                    { return E2END + 1; }

    //Functionality to 'get' and 'put' objects to and from EEPROM.
//     template< typename T > T &get( int idx, T &t ){
//         EEPtr e = idx;
//         uint8_t *ptr = (uint8_t*) &t;
//         for( int count = sizeof(T) ; count ; --count, ++e )  *ptr++ = *e;
//         return t;
//     }

//     template< typename T > const T &put( int idx, const T &t ){
//         const uint8_t *ptr = (const uint8_t*) &t;
// #ifdef __arm__
//         eeprom_write_block(ptr, (void *)idx, sizeof(T));
// #else
//         EEPtr e = idx;
//         for( int count = sizeof(T) ; count ; --count, ++e )  (*e).update( *ptr++ );
// #endif
//         return t;
//     }

private:
    char someFakeEEPROM_memory[2048];

};

static EEPROMClass EEPROM __attribute__ ((unused));


#endif
