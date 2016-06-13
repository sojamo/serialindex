#include "SerialIndex.h"

#define BAUDRATE 9600
#define CAPACITY 2
#define FCAPACITY 2
#define BUFFERSIZE 16

SerialIndex::SerialIndex(Stream &s) : Serialio(s)
{
	begin();
}

SerialIndex::~SerialIndex() 
{
	if (values != NULL)
		free(values);

	values = NULL;
}

// int
SerialIndex& SerialIndex::add(const char *k, int &v, int theTolerance) 
{ 
	addVariable(k, v, TYPE_INT, theTolerance);
	return *this; 
}

SerialIndex& SerialIndex::add(const char *k, int &v) 
{ 
	addVariable(k, v, TYPE_INT, 0); 
	return *this; 
}

// float
SerialIndex& SerialIndex::add(const char *k, float &v, float theTolerance) 
{
	addVariable(k, v, TYPE_FLOAT, theTolerance); 
	return *this; 
} 

SerialIndex& SerialIndex::add(const char *k, float &v) 
{
	addVariable(k, v, TYPE_FLOAT, 0); 
	return *this; 
}

// string
SerialIndex& SerialIndex::add(const char *k, char *&v) 
{
	// TODO should not be used yet until I have figured out how to 
	// update char* from inside evaluate().
	// addArray(k,&v,strlen(v),TYPE_STRING);
	return *this; 
}

// function
SerialIndex& SerialIndex::listen(const char *k, void (*t)(void)) 
{
	Function *fn = new Function();

	fn->index = (char *)malloc((strlen(k) + 1) * sizeof(char));
	strcpy(fn->index, k);
	fn->fn = t;
	functions[functions_size++] = fn;

	return *this; 
}

SerialIndex& SerialIndex::ping(char *k)
{
	Serialio.print(k);
	Serialio.println(" pong");
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

void SerialIndex::update(void)
{
	if (isRead) {
		while (Serialio.available()) {
			char b = Serialio.read();
			in(b);
		}
	}

	if (isWrite)
		out();
}

SerialIndex& SerialIndex::read(boolean b)
{
	isRead = b;
	return *this;
}

SerialIndex& SerialIndex::write(boolean b)
{
	isWrite = b;
	return *this;
}

SerialIndex& SerialIndex::io(const char *k, bool theIn, bool theOut) 
{
	Value* value = get(k);

	if (value != NULL) {
		value->in = theIn;
		value->out = theOut;
	}

	return *this;
}

SerialIndex& SerialIndex::in(char b) 
{
	if (b >= ' ')
		buffer[bufferindex++] = b;

	if (b == '\n') {
		buffer[bufferindex] = '\0';
		evaluate(buffer);
		bufferindex = 0;
	}

	return *this;
}

SerialIndex& SerialIndex::out()
{
	for (size_t i = 0; i < values_size; i++) {
		Value *value = values[i];

		if (value->out) {
			switch (value->type) {
			case TYPE_INT:
				{
					Variable<int> *data = static_cast<Variable<int>*>(value->data);

					// block sending if we have just received a value-update
					if (value->lock)
						break;

					if (abs(*data->now - data->then) > data->tolerance) {
						// send variable via serial
						sendVariable(value->index, *data->now);

						// updated data's then value
						data->then = *data->now;
					}
				}
			break;

			case TYPE_FLOAT:
				{
					Variable<float> *data = static_cast<Variable<float>*>(value->data);

					// block sending if we have just received a value-update
					if (value->lock)
						break;

					if (abs(*data->now - data->then) > data->tolerance) {
						// send variable via serial
						sendVariable(value->index, *data->now);

						// updated data's then value
						data->then = *data->now;
					}
				}
			break;

			case TYPE_INT_ARRAY:
				{
					Array<int*> *data = static_cast<Array<int*>*>(value->data);

					if (value->lock)
						break;

					for (size_t i = 0; i < data->size; i++) {
						if (*(data->now+i) != *(data->then + i)) {
							// send data
							sendArray(value->index, data->now, data->size);

							// updated data's then array 
							for (size_t j = 0; j < data->size; j++) {
								*(data->then + j) = *(data->now + j);
							}
							break;
						}
					}
				}
			break;

			// TODO: TYPE_FLOAT_ARRAY
			// TODO: TYPE_CHAR_ARRAY
			}
		}

		value->lock = false;
	}

	return *this;
}

void SerialIndex::evaluate(const char *input) 
{
	char t[strlen(input) + 1];         // need to add +1 to make the input end with a \0
	strcpy(t, input);
	int i = strlen(t);                 // store the total length of the string
	char *item = strtok(t, DELIMITER); // split the string
	char key[strlen(t)];               // declare a buffer for the key
	char valueBuffer[i - strlen(t)];   // declare a buffer for the value
	int n = 0;

	while (item) {
		switch (n++) {
		case 0:
			strcpy(key, item);
			break;

		case 1:
			Value* value = get(key);

			if (value == NULL || value->in == false)
				return;

			value->lock = true;

			// if we find a [, lets assume we are dealing with an array
			if (strchr(item,'[') != NULL) {
				// check if [ is the first character of the string to make sure it is an array
				switch (value->type) {
				case TYPE_INT_ARRAY:
					{
						Array<int*> *data = static_cast<Array<int*>*>(value->data);
						int* arr = toArray(item, TYPE_INT_ARRAY)->i;
						size_t size = (arr[0] < data->size) ? arr[0] : data->size; // first element is the size of the array, hence i+1 below
						for (size_t i = 0; i < size; i++) {
							*(data->now + i) = arr[i + 1] == NULL ? *(data->now + i) : arr[i + 1];
						}

						// invoke trigger function if available
						(getFn(value->index))();

						// update then
						for (size_t j = 0; j < data->size; j++) {
							*(data->then + j) = *(data->now + j);
						}

						free(arr);
					}
				break;

				case TYPE_FLOAT_ARRAY:
					{
						Array<float*> *data = static_cast<Array<float*>*>(value->data);
						float* arr = toArray(item,TYPE_FLOAT_ARRAY)->f;
						size_t size = (arr[0]<data->size) ? arr[0] : data->size; // first element here is the size of array arr, hence i+1 below

						for (size_t i = 0; i < size; i++) {
							*(data->now + i) = arr[i + 1] == NULL ? *(data->now + i) : arr[i + 1];
						}

						// invoke trigger function if available
						(getFn(value->index))();

						// update then
						for (size_t j = 0; j < data->size; j++) {
							*(data->then + j) = *(data->now + j);
						}

						free(arr);
					}
				break;

				}
			} else {
				if (isNumeric(item)) {
					switch (value->type) {
					case TYPE_INT:
						{
							Variable<int> *data = static_cast<Variable<int>*>(value->data);

							*data->now = atoi(item);

							// invoke trigger function if available
							(getFn(value->index))();

							// update then
							data->then = *data->now;
						}
					break;

					case TYPE_FLOAT:
						{
							Variable<float> *data = static_cast<Variable<float>*>(value->data);

							*data->now = atof(item);

							// invoke trigger function if available
							(getFn(value->index))();

							// update then
							data->then = *data->now;
						}
					break;

					}	
				} else {
					/* if all checks failed so far, we will treat the value as a string */
					switch(value->type) {
					case TYPE_STRING:
						{
							// Array<char**> *data = static_cast<Array<char**>*>(value->data);
							// TODO see add()
							// can't assign new value/pointer to data->now
						}
					break;

					case TYPE_CHAR_ARRAY:
						{
							Array<char*> *data = static_cast<Array<char*>*>(value->data);

							if (strlen(item) < data->size)
								strcpy(data->now, item);
						}
					break;

					}
				}				
			}

			strcpy(valueBuffer, item);
			break;
		}

		item = strtok(NULL, DELIMITER);
	}

	delete item;
}

int SerialIndex::isNumeric(const char *s)
{
	char * p;

	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;

	strtod(s, &p);

	return *p == '\0';
}

Format* SerialIndex::toArray(const char *item, const char theType) 
{
	const char delimiter[] = ",";
	char string[strlen(item) + 1];
	strcpy(string, item);
	memmove (string, string + 1, strlen (string + 1));
	char *s = string;
	int count;
	size_t index = 0;
	union Format format;

	for (count = 0; s[count]; s[count] == delimiter[0] ? count++ : *s++)
		;

	count++;

	switch (theType) {
	case TYPE_FLOAT_ARRAY:
		format.f = new float[count + 1]; format.f[index++] = count;
		break;

	case TYPE_INT_ARRAY:
		format.i = new int[count + 1]; format.i[index++] = count;
		break;

	case TYPE_CHAR_ARRAY:
		format.c = new char[count + 1]; format.c[index++] = count;
		break;

	default:
		return NULL;
	}

	char *token = strtok(string, delimiter);

	while (token != NULL) {
		switch (theType) {
		case TYPE_INT_ARRAY:
			format.i[index++] = (*token == '_') ? NULL : atoi(token);
			break;

		case TYPE_FLOAT_ARRAY:
			format.f[index++] = (*token == '_') ? NULL : atof(token);
			break;
		/* TODO
		case TYPE_CHAR_ARRAY:
			format.c[index++] = (*token == '_') ? NULL : (char)(token);
			break;
		*/

		default:
			return NULL;
		}

		token = strtok(NULL, delimiter); // Get next token
	}

	return &format;
}

Value* SerialIndex::get(const char *k) 
{
	for (size_t i = 0; i < values_size; i++) {
		Value *value = values[i];

		if (strcmp(value->index, k) == 0)
			return value;
	}

	return NULL;
}

fptr SerialIndex::getFn(const char *k) 
{
	for (size_t i = 0; i < functions_size; i++) {
		Function *fn = functions[i];

		if (strcmp(fn->index, k) == 0)
			return fn->fn;
	}

	return &nil;
}


/***********
 * PRIVATE *
 ***********/

void SerialIndex::resize() 
{
	if (values_size == 0)
		values = (Value**) malloc(values_capacity * sizeof(Value*));
	else if (values_size == values_capacity)
		; // TODO realloc
		
	if (functions_size == 0)
		functions = (Function**) malloc(functions_capacity * sizeof(Function*));
	else if (functions_size == functions_capacity)
		; // TODO realloc
}

SerialIndex Index(Serial);
