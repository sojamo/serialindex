#include "IO.hpp"

static const char   KV_DELIMITER                = '=';
static const char   EOL[]                       = "\r\n";
static const char   SLICE_DELIMITER             = '=';
static const size_t EOL_LEN                     = LEN(EOL) - 1;

IO::IO()
{
	context   = Context::Key;
	capacity  = CAPACITY;
	keys      = new const char *[capacity];
	types     = new Type[capacity];
	values    = new void*[capacity];
	functions = new Function[capacity];
	buffer    = new char[BUFFERSIZE];
	ibuffer   = 0;
	ikey      = -1;
	nkeys     = 0;
}

IO::~IO()
{
	delete buffer;
	delete types;
	delete keys;
	delete values;
	delete functions;
}

// int
IO& IO::add(const char *k, int &v, int theTolerance) 
{
	return add(k, v, Type::Int);
}

IO& IO::add(const char *k, int &v) 
{
	return add(k, v, Type::Int);
}

// float
IO& IO::add(const char *k, float &v, float theTolerance)
{
	return add(k, v, Type::Float);
}

IO& IO::add(const char *k, float &v) 
{
	return add(k, v, Type::Float);
}

// string
IO& IO::add(const char *k, char *&v) 
{
	return add(k, v, Type::String);
}

// function
IO& IO::listen(const char *k, void (*v)(void)) 
{
	int i;

	if (!k)
		goto out;

	i = find_key(k);
	if (i < 0)
		goto out;

	functions[i] = v;

out:
	return *this; 
}

void IO::read(char c)
{
	buffer[ibuffer++] = c;

	if (ibuffer >= BUFFERSIZE) {
		ibuffer = 0;
		context = Context::Skip;
	}

	switch (context) {
	case Context::Key:
		read_key(c);
		break;

	case Context::Value:
		read_value(c);
		break;

	case Context::IntValue:
		read_int(c);
		break;

	case Context::FloatValue:
		read_float(c);
		break;

	case Context::StringValue:
		read_string(c);
		break;

	case Context::ArrayValue:
		read_array(c);
		break;

	case Context::IntArrayValue:
		read_int_array(c);
		break;

	case Context::FloatArrayValue:
		read_float_array(c);
		break;

	case Context::SliceArrayValue:
		read_slice_array(c);
		break;

	case Context::IntSliceArrayValue:
		read_int_slice_array(c);
		break;

	case Context::FloatSliceArrayValue:
		read_float_slice_array(c);
		break;

	case Context::Skip:
		read_skip(c);
		break;
	}
}

void IO::read_key(char c)
{
	if (c == KV_DELIMITER) {
		buffer[ibuffer - 1] = 0;

		ikey = find_key(buffer);
		if (ikey == SIZE_MAX)
			goto skip;

		context = Context::Value;
		ibuffer = 0;
	} else if (!isalpha(c)) {
		goto skip;
	}

	return;

skip:
	context = Context::Skip;
}

void IO::read_value(char c)
{
	const Type type = types[ikey];

	switch (c) {
	case '0'...'9':
		if (type == Type::Int)
			context = Context::IntValue;
		else if (type == Type::Float)
			context = Context::FloatValue;
		else
			goto skip;
		return;

	case '[':
		if (type != Type::IntArray && type != Type::FloatArray)
			goto skip;

		context = Context::ArrayValue;
		return;

	case '{':
		if (type != Type::IntArray && type != Type::FloatArray)
			goto skip;

		context = Context::SliceArrayValue;
		return;

	default:
		if (type != Type::String)
			goto skip;

		context = Context::StringValue;
		return;
	}

skip:
	context = Context::Skip;
	return;
}

void IO::read_int(char c)
{
	if (is_eol()) {
		if (validate_int(&buffer[0], &buffer[ibuffer - EOL_LEN]) == ValidateResult::Ok) {
			eval(&buffer[0], &buffer[ibuffer - EOL_LEN]);
			return;
		}

		reset_context();
	}
}

void IO::read_float(char c)
{
	if (is_eol()) {
		if (validate_float(&buffer[0], &buffer[ibuffer - EOL_LEN]) == ValidateResult::Ok) {
			eval(&buffer[0], &buffer[ibuffer - EOL_LEN]);
			return;
		}

		reset_context();
	}
}

void IO::read_string(char c)
{
	if (is_eol()) {
		switch (validate_string(&buffer[0], &buffer[ibuffer - EOL_LEN])) {
		case ValidateResult::Ok:
			eval(&buffer[0], &buffer[ibuffer - EOL_LEN]);

		case ValidateResult::Continue:
			return;

		default:
			reset_context();
		}
	}
}

void IO::read_array(char c)
{
	const Type type = types[ikey];

	switch (c) {
	case '0'...'9':
		if (type == Type::IntArray)
			context = Context::IntArrayValue;
		else if (type == Type::FloatArray)
			context = Context::FloatArrayValue;
		else
			goto skip;
		break;

	default:

skip:
		context = Context::Skip;
		return;
	}
}

void IO::read_int_array(char c)
{
	if (c == ']') {
		if (validate_int_array(&buffer[1], &buffer[ibuffer + 1]) == ValidateResult::Ok) {
			eval(&buffer[1], &buffer[ibuffer + 1]);
			return;
		}

		context = Context::Skip;
	}
}

void IO::read_float_array(char c)
{
	if (c == ']') {
		if (validate_float_array(&buffer[1], &buffer[ibuffer + 1]) == ValidateResult::Ok) {
			eval(&buffer[1], &buffer[ibuffer + 1]);
			return;
		}

		context = Context::Skip;
	}
}

void IO::read_slice_array(char c)
{
	const Type type = types[ikey];

	switch (c) {
	case '0'...'9':
		if (type == Type::IntArray)
			context = Context::IntSliceArrayValue;
		else if (type == Type::FloatArray)
			context = Context::FloatSliceArrayValue;
		else
			goto skip;
		break;

	default:

skip:
		context = Context::Skip;
		return;
	}
}

void IO::read_int_slice_array(char c)
{
	if (c == '}') {
		if (validate_int_slice_array(&buffer[1], &buffer[ibuffer + 1]) == ValidateResult::Ok)
			eval(&buffer[1], &buffer[ibuffer + 1]);

		context = Context::Skip;
	}
}

void IO::read_float_slice_array(char c)
{
	if (c == '}') {
		if (validate_float_slice_array(&buffer[1], &buffer[ibuffer + 1]) == ValidateResult::Ok)
			eval(&buffer[1], &buffer[ibuffer + 1]);

		context = Context::Skip;
	}
}

void IO::read_skip(char c)
{
	if (!is_eol())
		return;

	reset_context();
}

ValidateResult IO::validate_int(char *s, char *e)
{
	const char *p;

	for (p = s; p < e; p++) {
		if (!isdigit(*p))
			return ValidateResult::Invalid;
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_float(char *s, char *e)
{
	const char *p;
	int ndots = 0;

	for (p = s; p < e; p++) {
		if (isdigit(*p))
			continue;
		else if (*p == '.' && ndots == 0)
			ndots++;
		else
			return ValidateResult::Invalid;
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_string(char *s, char *e)
{
	const char fc = *s;
	const bool has_start_quote = fc == '\'' || fc == '"';
	const bool has_end_quote = e - s > 1 && *(e - 1) == fc;

	if (has_start_quote) {
		if (!has_end_quote)
			return ValidateResult::Continue;
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_int_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == ']') {
			if (validate_int(pp, p) != ValidateResult::Ok)
				return ValidateResult::Invalid;

			pp = p + 1;
			i++;
		}
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_float_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == ']') {
			if (validate_float(pp, p) != ValidateResult::Ok)
				return ValidateResult::Invalid;

			pp = p + 1;
			i++;
		}
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_int_slice_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == '}') {
			if (validate_int_slice(pp, p) != ValidateResult::Ok)
				return ValidateResult::Invalid;

			pp = p + 1;
			i++;
		}
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_float_slice_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == '}') {
			if (validate_float_slice(pp, p) != ValidateResult::Ok)
				return ValidateResult::Invalid;

			pp = p + 1;
			i++;
		}
	}

	return ValidateResult::Ok;
}

ValidateResult IO::validate_int_slice(char *s, char *e)
{
	char *p;
	char *dp = 0;
	char *rdp = 0;

	for (p = s; p < e; p++) {
		if (dp)
			return validate_int(p, e);

		if (*p == SLICE_DELIMITER) {
			dp = p;
		} else if (is_slice_range_delimiter(p)) {
			if (rdp)
				goto invalid;
			rdp = p;
			p += SLICE_RANGE_DELIMITER_LEN - 1;
		} else if (!isdigit(*p)) {
			goto invalid;
		}
	}

invalid:
	return ValidateResult::Invalid;
}

ValidateResult IO::validate_float_slice(char *s, char *e)
{
	char *p;
	char *dp = 0;
	char *rdp = 0;

	for (p = s; p < e; p++) {
		if (dp)
			return validate_float(p, e);

		if (*p == SLICE_DELIMITER) {
			dp = p;
		} else if (is_slice_range_delimiter(p)) {
			if (rdp)
				goto invalid;
			rdp = p;
			p += SLICE_RANGE_DELIMITER_LEN - 1;
		} else if (!isdigit(*p)) {
			goto invalid;
		}
	}

invalid:
	return ValidateResult::Invalid;
}

void IO::eval(char *s, char *e)
{
	switch (context) {
	case Context::IntValue:
		eval_int(s, e);
		break;

	case Context::FloatValue:
		eval_float(s, e);
		break;

	case Context::StringValue:
		eval_string(s, e);
		break;

	case Context::IntArrayValue:
		eval_int_array(s, e);
		break;

	case Context::FloatArrayValue:
		eval_float_array(s, e);
		break;

	case Context::IntSliceArrayValue:
		eval_int_slice_array(s, e);
		break;

	case Context::FloatSliceArrayValue:
		eval_float_slice_array(s, e);
		break;

	default:
		return;
	}

	if (functions[ikey])
		functions[ikey]();

	reset_context();
}

void IO::eval_int(char *s, char *e)
{
	int *value = (int *) values[ikey];

	*value = atois(s, e);
}

void IO::eval_float(char *s, char *e)
{
	float *value = (float *) values[ikey];
	
	*value = strtods(s, e, NULL);
}

void IO::eval_string(char *s, char *e)
{
	char *value = (char *) values[ikey];

	if (*s == '\'' || *s == '"') {
		s += 1;
		e -= 1;
	}

	*e = 0;
	strcpy(value, s);
}

void IO::eval_int_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == ']') {
			eval_int_array_nth(pp, p, i);
			pp = p + 1;
			i++;
		}
	}
}

void IO::eval_int_array_nth(char *s, char *e, size_t i)
{
	int *array = (int *) values[ikey];

	if (*s == ']' && *e == ']')
		return;

	array[i] = atois(s, e);
}

void IO::eval_float_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == ']') {
			eval_float_array_nth(pp, p, i);
			pp = p + 1;
			i++;
		}
	}
}

void IO::eval_float_array_nth(char *s, char *e, size_t i)
{
	float *array = (float *) values[ikey];

	if (*s == ']' && *e == ']')
		return;

	array[i] = strtods(s, e, NULL);
}

void IO::eval_int_slice_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == '}') {
			eval_int_slice(pp, p);
			pp = p + 1;
			i++;
		}
	}
}

void IO::eval_float_slice_array(char *s, char *e)
{
	char *pp, *p;
	size_t i = 0;

	for (pp = p = s; p < e; p++) {
		if (*p == ',' || *p == '}') {
			eval_float_slice(pp, p);
			pp = p + 1;
			i++;
		}
	}
}

void IO::eval_int_slice(char *s, char *e)
{
	char *p, *dp = 0, *rdp = 0;
	int *array = (int *) values[ikey];
	int value = 0;
	size_t start = 0, end = 0;
	size_t i;

	for (p = s; p < e; p++) {
		if (*p == SLICE_DELIMITER)
			dp = p;
		else if (is_slice_range_delimiter(p))
			rdp = p;
	}

	if (rdp) {
		if (rdp > s)
			start = atois(s, rdp);

		if (rdp < dp - SLICE_RANGE_DELIMITER_LEN)
			end = atois(rdp + SLICE_RANGE_DELIMITER_LEN, dp);
	} else {
		start = atois(s, dp);
		end = start + 1;
	}

	value = atois(dp + 1, e);

	for (i = start; i < end; i++)
		array[i] = value;
}

void IO::eval_float_slice(char *s, char *e)
{
	char *p, *dp = 0, *rdp = 0;
	float *array = (float *) values[ikey];
	float value = 0;
	size_t start = 0, end = 0;
	size_t i;

	for (p = s; p < e; p++) {
		if (*p == SLICE_DELIMITER)
			dp = p;
		else if (is_slice_range_delimiter(p))
			rdp = p;
	}

	if (rdp) {
		if (rdp > s)
			start = atois(s, rdp);

		if (rdp < dp - SLICE_RANGE_DELIMITER_LEN)
			end = atois(rdp + SLICE_RANGE_DELIMITER_LEN, dp);
	} else {
		start = atois(s, dp);
		end = start + 1;
	}

	value = strtods(dp + 1, e, NULL);

	for (i = start; i < end; i++)
		array[i] = value;
}

bool IO::is_eol()
{
	size_t i;

	if (ibuffer < EOL_LEN)
		return false;

	for (i = 0; i < EOL_LEN; i++) {
		if (buffer[ibuffer - EOL_LEN + i] != EOL[i] )
			return false;
	}

	return true;
}

void IO::reset_context(void)
{
	context = Context::Key;
	ibuffer = 0;
}

size_t IO::find_key(const char *s) 
{
	size_t i;

	for (i = 0; i < nkeys; i++) {
		if (strcmp(keys[i], s) == 0)
			return i;
	}

	return SIZE_MAX;
}

size_t IO::find_key(const char *start, const char *end) 
{
	size_t i;

	for (i = 0; i < nkeys; i++) {
		if (strncmp(keys[i], start, end - start) == 0)
			return i;
	}

	return -1;
}
