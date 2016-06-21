#ifndef SERIAL_INDEX_H
#define SERIAL_INDEX_H

#include "IO.hpp"

enum Mode {
	Read = 1,
	Write = 2,
};

class SerialIndex : public IO
{
public:
	SerialIndex(Serial_ &s);
	~SerialIndex();

	SerialIndex&   ping(char *k);

	SerialIndex&   begin(void);
	SerialIndex&   begin(long);
	SerialIndex&   begin(long, int);
	SerialIndex&   begin(long, int, int);

	SerialIndex&   io(const char *k, bool theIn, bool theOut);
	SerialIndex&   in(char b);
	SerialIndex&   out();

	SerialIndex&   read(bool);
	SerialIndex&   write(bool);
	void           update(void);

	void           read();
	void           write();

private:
	Serial_&       serial;
	int            mode;
};

extern SerialIndex Index;

#endif // SERIAL_INDEX_H

/* Notes
 * [1] simple Vector class for Arduino; http://forum.arduino.cc/index.php?topic=45626.0
 * [2] how to free and reallocate memory; http://stackoverflow.com/questions/7159472/how-can-i-free-memory-from-my-structure-containing-an-array-of-dynamically-creat
 * [3] sizeof_array; http://stackoverflow.com/questions/3368883/how-does-this-size-of-array-template-function-work
 */
