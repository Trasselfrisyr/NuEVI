#include <cstdint>
#include <cstdio>


#include "Arduino.h"

/********************************
 *
 */
void SimSerial::begin(uint32_t)
{

}


void SimSerial::print(const char* str)
{
	printf( "[Serial::print] %s\n", str );
}


void SimSerial::print(uint32_t intValue)
{
	printf( "[Serial::print] %d\n", intValue );
}

void SimSerial::println()
{
	printf("\n");
}


void SimSerial::println(uint32_t intValue)
{
	printf( "[Serial::println] %d\n", intValue );
}


void SimSerial::println(const char *str)
{
	printf( "[Serial::println] %s\n", str );
}

