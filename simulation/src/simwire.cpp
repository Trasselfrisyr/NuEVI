
#include <stdio.h>

#include "Wire.h"

/********************************
 *
 */

SimWire::SimWire( bool verbose )
: verbose_( verbose )
{

}

void SimWire::setClock(uint32_t)
{
	// Ignore.. lol
}



void SimWire::begin()
{

}

void SimWire::beginTransmission(uint8_t address)
{
	if( verbose_ )
		printf("[SimWire::beginTransmission] $%02x\n", address);
}


void SimWire::beginTransmission(int address)
{
	beginTransmission((uint8_t)address);
}


uint8_t SimWire::endTransmission()
{
	if( verbose_ )
		printf("[SimWire::endTransmission]\n");
	return 0;
}

uint8_t SimWire::endTransmission(uint8_t what)
{
	if( verbose_ )
		printf("[SimWire::endTransmission %d]\n", what);
	return 0;
}



uint8_t SimWire::requestFrom(uint8_t address, uint8_t count)
{
	if( verbose_ )
		printf("[SimWire::requestFrom] $%02x for %d bytes\n", address, count);

	// TODO: We must check if there is an actual slave for that address.
	return 0;
}

int SimWire::read()
{
	// TODO: Verify that bus is in read mode.
	// if( current_slave_ != NULL ) {
	// 	return current_slave_->i2cReadData( );
	// } else {
		printf("No slave selected, returning ones\n");
		return 0xffu;
	// }
}

int SimWire::available()
{
	// TODO: This needs to be implemented!!
	return 0;
}


size_t SimWire::write(uint8_t __attribute__((unused)) data)
{
	// // TODO: Verify that bus is in write mode.
	// if( current_slave_ != NULL ) {
	// 	current_slave_->i2cWriteData( data );
	// } else {
		printf("No slave selected i2c writes to the void.\n");
	// }
	return 1;
}

