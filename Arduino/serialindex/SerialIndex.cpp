#include "SerialIndex.h"

#define BAUDRATE 9600
#define CAPACITY 2
#define BUFFERSIZE 16



 SerialIndex::SerialIndex(Stream &s) : Serialio(s)
{
	begin();
}

void SerialIndex::begin(void)
{
	begin(BAUDRATE,CAPACITY,BUFFERSIZE);
}

void SerialIndex::begin(long theBaudrate)
{
	begin(theBaudrate,CAPACITY,BUFFERSIZE);
}

void SerialIndex::begin(long theBaudrate, int theCapacity)
{
	begin(theBaudrate,theCapacity,BUFFERSIZE);
}

void SerialIndex::begin(long theBaudrate, int theCapacity, int theBufferSize)
{
	values_size = 0;
	values_capacity = theCapacity;
	resize();

	
	Serial.begin(theBaudrate);
	Serialio = Serial;

	isRead = true;
	isWrite = true;
	buffer = new char[theBufferSize];

}


void SerialIndex::read( boolean b )
{
	isRead = b;
}

void SerialIndex::write( boolean b )
{
	isWrite = b;
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


SerialIndex Index( Serial);

