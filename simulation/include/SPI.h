#ifndef __SPI_H__
#define __SPI_H__

#include <cstdint>


#define SPIClass SimSPI


#ifndef SPI_MODE0
	#define SPI_MODE0 0
#endif

#ifndef SPI_CLOCK_DIV2
	#define SPI_CLOCK_DIV2 0
#endif

#define SPDR *SPI.SPI_dataReg
#define SPCR *SPI.SPI_ctrlReg
#define SPSR *SPI.SPI_statusReg

//#define SPSR 0xff
#define SPIF (SPI.SPI_IF)


// Forward declare
class SimSPI;
class DataRegister;
class SPICtrlRegister;
class SPIStatusRegister;


extern SimSPI SPI;


class ISPIDevice
{
public:
	virtual uint8_t spiSlaveWrite( uint8_t ) = 0;
};



class SimSPI
{
public:
	SimSPI();

	void AddDevice( ISPIDevice* device );

	void begin();

	void transfer( uint8_t something );

	void setDataMode( uint8_t mode );
	void setClockDivider( uint32_t divider );
	void writeReg( DataRegister *reg, uint8_t data);

	// Members
	DataRegister* SPI_dataReg;
	SPICtrlRegister* SPI_ctrlReg;
	SPIStatusRegister* SPI_statusReg;


	uint8_t SPI_IF;

private:
	ISPIDevice** devices;
	uint32_t devicesArraySize;

	ISPIDevice* currentSlave;

};


class HWRegister
{
public:
	HWRegister( SimSPI& owner) : spi_( owner ) {}
protected:
	SimSPI& spi_;
};


class DataRegister : HWRegister
{
public:
	DataRegister( SimSPI& owner) : HWRegister( owner ) {}
	void operator=(uint8_t data)
	{
		spi_.writeReg( this, data );
	}

	operator uint8_t()
	{
		// TODO: Get last read byte
		return 0xffu;
	}
};


class SPIStatusRegister : HWRegister
{
public:
	SPIStatusRegister( SimSPI& owner) : HWRegister( owner ) {}
	void operator =(uint8_t value)
	{
		reg_ = value;
	}

	void operator |=(uint8_t value)
	{
		reg_ |= value;
	}

	void operator &=(uint8_t value)
	{
		reg_ &= value;
	}
/*
	operator uint8_t()
	{
		return reg_|0xff;
	}
*/
	operator uint32_t()
	{
		return reg_|0xff;
	}
	
	uint8_t reg_;
};

class SPICtrlRegister : HWRegister
{
public:
	SPICtrlRegister( SimSPI& owner) : HWRegister( owner ) {}

	void operator =(uint8_t value)
	{
		reg_ = value;
	}

	void operator |=(uint8_t value)
	{
		reg_ |= value;
	}

	void operator &=(uint8_t value)
	{
		reg_ &= value;
	}


	operator uint8_t()
	{
		return reg_;
	}


	uint8_t reg_;
};


#endif
