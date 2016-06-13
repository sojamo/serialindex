#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifndef SerialIndex_h
#define SerialIndex_h

#define TYPE_INT 'i'
#define TYPE_FLOAT 'f'
#define TYPE_STRING 's'
#define TYPE_CHAR_ARRAY 'C'
#define TYPE_INT_ARRAY 'I'
#define TYPE_FLOAT_ARRAY 'F'
#define DELIMITER "="

template <size_t N>
struct type_of_size { typedef char type[N]; };

template <typename T, size_t Size>
typename type_of_size<Size>::type& sizeof_array_helper(T(&)[Size]);

#define sizeof_array(pArray) sizeof(sizeof_array_helper(pArray))

template< typename _A, typename _B >
bool compareType(_A a, _B b) { return false; }

template< typename _A >
bool compareType(_A a, _A b) { return true; }

typedef void (*fptr)();

/* Value */
class Value 
{
public:
	char *index;
	char  type;
	void *data;
	bool  in;
	bool  out;
	bool  lock;

	Value(const char *s, char c) {
		index = (char *)malloc((strlen(s)+1) * sizeof(char));
		strcpy(index, s);
		type = c;
		in = true;
		out = true;
		lock = false;
	}
	~Value() {
		free(index);
		free(data);
	}
};


// Format
union Format 
{
	float *f;
	int   *i;
	char  *c;
};

// Variable, stores the state of a variable
template<typename T> 
struct Variable 
{
	float tolerance;
	T    *now;
	T     then;

	~Variable() {
		free(now);
	}
};


// Array, stores the state of an array
template<typename T> 
struct Array 
{
	size_t size;
	T      now;
	T      then;

	~Array() {
		free(now);
		free(then);
	}
};

// Function, stores the reference to a function
struct Function 
{
	char *index;
	fptr  fn;
};


class SerialIndex
{
public:
	SerialIndex(Stream &s);
	~SerialIndex();

	// int
	SerialIndex& add(const char *k, int &v, int theTolerance);
	SerialIndex& add(const char *k, int &v);

	// float
	SerialIndex& add(const char *k, float &v, float theTolerance);
	SerialIndex& add(const char *k, float &v);

	// int-array
	template<int N>
	SerialIndex& add(const char *k, int (&v)[N]);

	// float-array
	template<int N>
	SerialIndex& add(const char *k, float (&v)[N]);

	// string
	template<int N>
	SerialIndex& add(const char *k, char (&v)[N]);

	SerialIndex& add(const char *k, char *&v);

	// function
	SerialIndex& listen(const char *k, void (*t)(void));

	SerialIndex& ping(char *k);

	SerialIndex& begin(void);
	SerialIndex& begin(long);
	SerialIndex& begin(long, int);
	SerialIndex& begin(long, int, int);
	void update(void);
	SerialIndex& read(boolean);
	SerialIndex& write(boolean);

	SerialIndex& io(const char *k, bool theIn, bool theOut);
	SerialIndex& in(char b);
	SerialIndex& out();

	template<typename T>
	SerialIndex& sendVariable(char *theIndex, T &t);

	template<typename T>
	SerialIndex& sendArray(char *theIndex, T t, int theLen);

	void         evaluate(const char *input);
	int          isNumeric (const char *s);
	Format *     toArray(const char *item, const char theType);

	Value *      get(const char *k);
	fptr         getFn(const char *k);

	static void  nil() {}

private:
	Stream    &Serialio;
	bool       isWrite;
	bool       isRead;
	char      *buffer;
	int        bufferindex;

	Value    **values;
	int        values_size;
	int        values_capacity;

	Function **functions;
	int        functions_size;
	int        functions_capacity;

	// currently the capacity of the array containing all values is fixed but should be dynamic in the future.
	void resize();

	template<class T>
	void addVariable(const char *k, T &t, char c, float theTolerance);

	template<class T>
	void addArray(const char *k, T t, int s, char c);
};

// int-array
template<int N>
SerialIndex& SerialIndex::add(const char *k, int (&v)[N])
{
	addArray(k,v,sizeof_array(v), TYPE_INT_ARRAY);
	return *this; 
}

// float-array
template<int N>
SerialIndex& SerialIndex::add(const char *k, float (&v)[N]) 
{
	addArray(k,v,sizeof_array(v), TYPE_FLOAT_ARRAY);
	return *this; 
}

/* string */
template<int N>
SerialIndex& SerialIndex::add(const char *k, char (&v)[N]) 
{
	addArray(k,v,sizeof_array(v), TYPE_CHAR_ARRAY);
	return *this; 
}

template<typename T>
SerialIndex& SerialIndex::sendVariable(char *theIndex, T &t)
{
	Serialio.print(theIndex);
	Serialio.print(DELIMITER);
	Serialio.print(t);
	Serialio.print("\n\r");
	return *this;
}

template<typename T>
SerialIndex& SerialIndex::sendArray(char *theIndex, T t, int theLen)
{
	Serialio.print(theIndex);
	Serialio.print(DELIMITER);
	Serialio.print("[");

	for (size_t j = 0; j < theLen - 1; j++) {
		Serialio.print(*(t + j));
		Serialio.print(',');
	}

	Serial.print(*(t + (theLen - 1)));
	Serialio.print("]\n\r");
	return *this;
}

template<class T>
void SerialIndex::addVariable(const char *k, T &t, char c, float theTolerance) 
{
	Value *value = new Value(k, c);
	Variable<T> *data = new Variable<T>();

	data->now = &t;
	data->then = t;
	data->tolerance = theTolerance;
	value->data = data;
	values[values_size++] = value;
}

template<class T>
void SerialIndex::addArray(const char *k, T t, int s, char c) 
{
	Value *value = new Value(k, c);
	Array<T> *data = new Array<T>();

	value->data = data;
	data->now = t;
	data->size = s;
	data->then = (T) malloc((data->size) * sizeof(T));
	memcpy(data->then, data->now, data->size);
	value->data = data;
	values[values_size++] = value;
}

extern SerialIndex Index;

#endif // SerialIndex_h

/* Notes
 * [1] simple Vector class for Arduino; http://forum.arduino.cc/index.php?topic=45626.0
 * [2] how to free and reallocate memory; http://stackoverflow.com/questions/7159472/how-can-i-free-memory-from-my-structure-containing-an-array-of-dynamically-creat
 * [3] sizeof_array; http://stackoverflow.com/questions/3368883/how-does-this-size-of-array-template-function-work
 */
