#include <stdio.h>
#include "IO.hpp"

#include "sample.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

static int    _int;
static float  _float;
static char   _string[32];
static int    _ints[3];
static float  _floats[3];
static char * _strings[3] = { new char[32], new char[32], new char[32] };

void int_cb()
{
	fprintf(stdout, "int:     %d\n", _int);
}

void float_cb()
{
	fprintf(stdout, "float:   %f\n", _float);
}

void string_cb()
{
	fprintf(stdout, "string:  %s\n", _string);
}

void ints_cb()
{
	size_t i;

	fprintf(stdout, "ints:    [");

	for (i = 0; i < LEN(_ints); i++)
		fprintf(stdout, " %d ", _ints[i]);

	fprintf(stdout, "]\n");
}

void floats_cb()
{
	size_t i;

	fprintf(stdout, "floats:  [");

	for (i = 0; i < LEN(_floats); i++)
		fprintf(stdout, " %f ", _floats[i]);

	fprintf(stdout, "]\n");
}

void strings_cb()
{
	size_t i;

	fprintf(stdout, "strings: [");

	for (i = 0; i < LEN(_strings); i++)
		fprintf(stdout, " %s ", _strings[i]);

	fprintf(stdout, "]\n");
}

int main()
{
	IO io;
	size_t i;

	io.add("int", _int).listen("int", &int_cb);
	io.add("float", _float).listen("float", &float_cb);
	io.add("string", _string).listen("string", &string_cb);
	io.add("ints", _ints).listen("ints", &ints_cb);
	io.add("floats", _floats).listen("floats", &floats_cb);

	for (i = 0; i < LEN(sample_txt); i++)
		io.read(sample_txt[i]);
}
