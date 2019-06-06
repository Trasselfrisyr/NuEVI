#ifndef __WIRE_H__
#define __WIRE_H__

#include <cstdint>
#define TwoWire SimWire

class SimWire
{
public:
    SimWire(bool verbose = false);

    void begin();
    void begin( uint8_t address ); // For slaves.
    void end();
    void setClock(uint32_t);
    void beginTransmission(uint8_t address);
    void beginTransmission(int);
    uint8_t endTransmission();
    uint8_t endTransmission(uint8_t);
    uint8_t requestFrom(uint8_t address, uint8_t count);
    uint8_t requestFrom(uint8_t, uint8_t, uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t, uint32_t, uint8_t, uint8_t);
    size_t write(uint8_t);
    size_t write(const uint8_t *, size_t);
    int available();
    int read();
    int peek();
    void flush();
    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );
private:
    bool verbose_;
};

extern SimWire Wire;


#endif
