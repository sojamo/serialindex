#include "SerialIndex.h"

#define BAUDRATE 9600
#define CAPACITY 2
#define FCAPACITY 2
#define BUFFERSIZE 16



 SerialIndex::SerialIndex(Stream &s) : Serialio(s)
{
	begin();
}

SerialIndex& SerialIndex::begin(void)
{
	return begin(BAUDRATE,CAPACITY,BUFFERSIZE);
}

SerialIndex& SerialIndex::begin(long theBaudrate)
{
	return begin(theBaudrate,CAPACITY,BUFFERSIZE);
}

SerialIndex& SerialIndex::begin(long theBaudrate, int theCapacity)
{
	return begin(theBaudrate,theCapacity,BUFFERSIZE);
}

SerialIndex& SerialIndex::begin(long theBaudrate, int theCapacity, int theBufferSize)
{
	values_size = 0;
	values_capacity = theCapacity;

	functions_size = 0;
	functions_capacity = FCAPACITY;

	resize();

	
	Serial.begin(theBaudrate);
	Serialio = Serial;

	isRead = true;
	isWrite = true;
	buffer = new char[theBufferSize];

	return *this;
}


SerialIndex& SerialIndex::read( boolean b )
{
	isRead = b;
	return *this;
}

SerialIndex& SerialIndex::write( boolean b )
{
	isWrite = b;
	return *this;
}

void SerialIndex::update(void)
{
	if(isRead==true) {
		if( Serialio.available( ) ) {
			char b;
			while (Serialio.available( ) ){
				b = Serialio.read();
				in(b);
			}
		}
	}

	if(isWrite==true) {
		out();
	}
}


SerialIndex Index( Serial );

