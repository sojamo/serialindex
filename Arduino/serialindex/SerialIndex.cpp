#include "SerialIndex.h"
#include "IO.hpp"

SerialIndex::SerialIndex(Serial_ &s) :
	serial(s)
{
	begin();

	mode = 0;
}

SerialIndex::~SerialIndex() 
{
}

SerialIndex& SerialIndex::ping(char *k)
{
	// TODO
	return *this; 
}

SerialIndex& SerialIndex::begin(void)
{
	return begin(BAUDRATE, CAPACITY, BUFFERSIZE);
}

SerialIndex& SerialIndex::begin(long theBaudrate)
{
	return begin(theBaudrate, CAPACITY, BUFFERSIZE);
}

SerialIndex& SerialIndex::begin(long theBaudrate, int theCapacity)
{
	return begin(theBaudrate, theCapacity, BUFFERSIZE);
}

SerialIndex& SerialIndex::begin(long theBaudrate, int theCapacity, int theBufferSize)
{
	serial.begin(theBaudrate);

	// TODO

	return *this;
}

SerialIndex& SerialIndex::io(const char *k, bool theIn, bool theOut) 
{
	// TODO
	return *this;
}

SerialIndex& SerialIndex::in(char b) 
{
	// TODO
	return *this;
}

SerialIndex& SerialIndex::out()
{
	// TODO
	return *this;
}

SerialIndex& SerialIndex::read(bool b)
{
	if (b)
		mode |= Mode::Read;
	else
		mode ^= Mode::Read;

	return *this;
}

SerialIndex& SerialIndex::write(bool b)
{
	if (b)
		mode |= Mode::Write;
	else
		mode ^= Mode::Write;

	return *this;
}

void SerialIndex::update(void)
{
	if (mode & Mode::Read != 0)
		read();

	if (mode & Mode::Write != 0)
		write();
}

void SerialIndex::read()
{
	while (serial.available())
		IO::read(serial.read());
}

void SerialIndex::write()
{
	// TODO
}

SerialIndex Index(Serial);
