#ifndef IO_HPP
#define IO_HPP

#include "util.h"

#define BAUDRATE       (9600)
#define CAPACITY       (6)
#define BUFFERSIZE     (128)
#define MAX_KEY_LENGTH (16)

typedef void (*Function)(void);

enum Context {
	Key,
	Value,
	IntValue,
	FloatValue,
	StringValue,
	ArrayValue,
	IntArrayValue,
	FloatArrayValue,
	SliceArrayValue,
	IntSliceArrayValue,
	FloatSliceArrayValue,
	Skip,
};

enum Type {
	Unknown = 0,
	Int,
	Float,
	String,
	IntArray,
	FloatArray,
};

enum ValidateResult {
	Ok,
	Continue,
	Invalid,
};

class IO {
public:
	IO();
	~IO();

	// int
	IO&            add(const char *k, int &v, int theTolerance);
	IO&            add(const char *k, int &v);

	// float
	IO&            add(const char *k, float &v, float theTolerance);
	IO&            add(const char *k, float &v);

	// int-array
	template<int N>
	IO&            add(const char *k, int (&v)[N]);

	// float-array
	template<int N>
	IO&            add(const char *k, float (&v)[N]);

	// string
	template<int N>
	IO&            add(const char *k, char (&v)[N]);
	IO&            add(const char *k, char *&v);

	// function
	IO&            listen(const char *k, void (*v)(void));

	void           read(char);
	void           read_key(char c);
	void           read_value(char c);
	void           read_int(char c);
	void           read_float(char c);
	void           read_string(char c);
	void           read_array(char c);
	void           read_int_array(char c);
	void           read_float_array(char c);
	void           read_slice_array(char c);
	void           read_int_slice_array(char c);
	void           read_float_slice_array(char c);
	void           read_skip(char c);

	ValidateResult validate_int(char *s, char *e);
	ValidateResult validate_float(char *s, char *e);
	ValidateResult validate_string(char *s, char *e);
	ValidateResult validate_int_array(char *s, char *e);
	ValidateResult validate_float_array(char *s, char *e);
	ValidateResult validate_int_slice_array(char *s, char *e);
	ValidateResult validate_float_slice_array(char *s, char *e);
	ValidateResult validate_int_slice(char *s, char *e);
	ValidateResult validate_float_slice(char *s, char *e);

	void           eval(char *s, char *e);
	void           eval_int(char *s, char *e);
	void           eval_float(char *s, char *e);
	void           eval_string(char *s, char *e);
	void           eval_int_array(char *s, char *e);
	void           eval_int_array_nth(char *s, char *e, size_t i);
	void           eval_float_array(char *s, char *e);
	void           eval_float_array_nth(char *s, char *e, size_t i);
	void           eval_int_slice_array(char *s, char *e);
	void           eval_float_slice_array(char *s, char *e);
	void           eval_int_slice(char *s, char *e);
	void           eval_float_slice(char *s, char *e);

private:
	const char **  keys;
	Type *         types;
	void **        values;
	Function *     functions;
	char *         buffer;
	Context        context;
	size_t         ibuffer;
	size_t         ikey;
	size_t         nkeys;
	size_t         capacity;

	size_t         find_key(const char *s);
	size_t         find_key(const char *s, const char *e);
	void           reset_context(void);
	bool           is_eol();

	template<class T>
	IO&            add(const char *k, T &v, Type t);
};

// generic
template<class T>
IO& IO::add(const char *k, T &v, Type t)
{
	if (!k || nkeys >= capacity)
		goto out;

	if (find_key(k) < SIZE_MAX)
		goto out;

	types[nkeys] = t;
	keys[nkeys] = k;
	values[nkeys] = &v;
	functions[nkeys] = 0;
	nkeys++;

out:
	return *this; 
}

// int-array
template<int N>
IO& IO::add(const char *k, int (&v)[N])
{
	return add(k, v, Type::IntArray); 
}

// float-array
template<int N>
IO& IO::add(const char *k, float (&v)[N]) 
{
	return add(k, v, Type::FloatArray); 
}

// string
template<int N>
IO& IO::add(const char *k, char (&v)[N]) 
{
	return add(k, v, Type::String); 
}

#endif
