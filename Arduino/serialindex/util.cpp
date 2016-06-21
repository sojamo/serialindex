#include "util.h"

const char   SLICE_RANGE_DELIMITER[]     = "..";
const size_t SLICE_RANGE_DELIMITER_LEN   = LEN(SLICE_RANGE_DELIMITER) - 1;

bool is_slice_range_delimiter(const char *s)
{
	size_t i;

	for (i = 0; i < SLICE_RANGE_DELIMITER_LEN; i++) {
		if (s[i] != SLICE_RANGE_DELIMITER[i])
			return false;
	}

	return true;
}
