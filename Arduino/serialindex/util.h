#ifndef UTIL_H
#define UTIL_H

#if defined(ARDUINO)
	#if ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif
#else
	#include <stdlib.h>
	#include <string.h>
	#include <stdio.h>
	#include <stdint.h>
	#include <ctype.h>
#endif

#define LEN(x) (sizeof(x) / sizeof(x[0]))

extern const char SLICE_RANGE_DELIMITER[];
extern const size_t SLICE_RANGE_DELIMITER_LEN;

static inline int atois(const char *s, char *e)
{
	int value;
	char tmp;

	tmp = *e;
	*e = 0;
	value = atoi(s);
	*e = tmp;

	return value;
}

static inline double strtods(const char *s, char *e, char **endptr)
{
	double value;
	char tmp;

	tmp = *e;
	*e = 0;
	value = strtod(s, NULL);
	*e = tmp;

	return value;
}

bool is_slice_range_delimiter(const char *s);

#endif
