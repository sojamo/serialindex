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


template <size_t N> struct type_of_size { typedef char type[N]; };
template <typename T, size_t Size> typename type_of_size<Size>::type& sizeof_array_helper(T(&)[Size]);
#define sizeof_array(pArray) sizeof(sizeof_array_helper(pArray))


template< typename _A, typename _B > bool compareType( _A a, _B b ) { return false; }
template< typename _A > bool compareType( _A a, _A b ) { return true; }


typedef void (*fptr)();

/* Value */
class Value 
{
public:
	char* index;
	char type;
	void* data;
	bool in;
	bool out;
	bool lock;
	Value(const char* s, char c) {
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


/* Format */
union Format 
{
	float* f;
	int* i;
	char* c;

};

/* Variable, stores the state of a variable */
template< typename T> 
struct Variable 
{
	float tolerance;
	T *now;
	T then;

	~Variable() {
		free(now);
	}
};


/* Array, stores the state of an array */
template< typename T> 
struct Array 
{
	size_t size;
	T now;
	T then;

	~Array() {
		free(now);
		free(then);
	}
};

/* Function, stores the reference to a function */
struct Function 
{
	char* index;
	fptr fn;
};


class SerialIndex
{
	public:

		SerialIndex( Stream &s );

		~SerialIndex() 
		{
			if(values!=NULL) {
				free(values);
			}
			values = NULL;
		}

		/* int */
		SerialIndex& add(const char* k, int &v, int theTolerance) 
		{ 
			addVariable(k,v,TYPE_INT,theTolerance);
			return *this; 
		}

		SerialIndex& add(const char* k, int &v) 
		{ 
			addVariable(k,v,TYPE_INT,0); 
			return *this; 
		}

		/* float */
		SerialIndex& add(const char* k, float &v, float theTolerance) 
		{
			addVariable(k,v,TYPE_FLOAT,theTolerance); 
			return *this; 
		} 

		SerialIndex& add(const char* k, float &v) 
		{
			addVariable(k,v,TYPE_FLOAT,0); 
			return *this; 
		} 

		/* int-array */
		template<int N> SerialIndex& add(const char* k, int (&v)[N]) 
		{
			addArray(k,v,sizeof_array(v), TYPE_INT_ARRAY);
			return *this; 
		}

		/* float-array */
		template<int N> SerialIndex& add(const char* k, float (&v)[N]) 
		{
			addArray(k,v,sizeof_array(v), TYPE_FLOAT_ARRAY);
			return *this; 
		}

		/* string */
		template<int N> SerialIndex& add(const char* k, char (&v)[N]) 
		{
			addArray(k,v,sizeof_array(v), TYPE_CHAR_ARRAY);
			return *this; 
		}

		SerialIndex& add(const char* k, char* &v) 
		{
			// TODO should not be used yet until I have figured out how to 
			// update char* from inside evaluate().
			// addArray(k,&v,strlen(v),TYPE_STRING);
			return *this; 
		}

		/* function */
		SerialIndex& listen(const char* k, void (*t)(void)) 
		{
			Function *fn = new Function();
			fn->index = (char *)malloc((strlen(k)+1) * sizeof(char));
			strcpy(fn->index, k);
			fn->fn = t;
			functions[functions_size++] = fn;
			return *this; 
		}


		SerialIndex& ping(char* k) {
			Serialio.print(k);
			Serialio.println(" pong");
			return *this; 
		}

		SerialIndex& begin(void);
		SerialIndex& begin(long);
		SerialIndex& begin(long,int);
		SerialIndex& begin(long,int,int);
		void update(void);
		SerialIndex& read( boolean );
  		SerialIndex& write( boolean );

		

		SerialIndex& io(const char* k, bool theIn, bool theOut) 
		{
			Value* value = get(k);
			if(value != NULL) {
				value->in = theIn;
				value->out = theOut;
			}
			return *this;
		}


		SerialIndex& in(char b) 
		{
			if(b >=' ') {
				buffer[bufferindex++] = b;
			}

			if( b == 10 ) {
				buffer[bufferindex]='\0';
				evaluate(buffer);
				bufferindex = 0;
			}
			return *this;
		}

		SerialIndex& out() 
		{
			size_t i;
			for(i=0;i<values_size;i++) {
				Value *value = values[i];

				if(value->out==true) {
					switch(value->type) {
						case(TYPE_INT):
						{
							Variable<int> *data = static_cast<Variable<int>*>(value->data);
							if(value->lock == false) { /* block sending if we have just received a value-update */
								if( abs(*data->now - data->then)>data->tolerance ) {
									/* send variable via serial */
									sendVariable(value->index, *data->now);
									/* updated data's then value */
									data->then = *data->now;
								}
							}
						}
						break;
						case(TYPE_FLOAT):
						{
							Variable<float> *data = static_cast<Variable<float>*>(value->data);
							if(value->lock == false) { /* block sending if we have just received a value-update */
								if( abs(*data->now - data->then)>data->tolerance ) {
									/* send variable via serial */
									sendVariable(value->index, *data->now);
									/* updated data's then value */
									data->then = *data->now;
								}
							}
						}
						break;

						case(TYPE_INT_ARRAY):
						{
							Array<int*> *data = static_cast<Array<int*>*>(value->data);
							if(value->lock == false) {
								size_t i;
								size_t len = data->size;
								for(i=0;i<len;i++) {
									if(*(data->now+i) != *(data->then+i)) {
										/* send data */
										sendArray(value->index, data->now, len);
										/* updated data's then array */
										size_t j;
										for(j=0;j<len;j++) {
											*(data->then+j) = *(data->now+j);
										}
										break;
									}
								}
								
							}
						}
						break;
						// TODO case(TYPE_FLOAT_ARRAY):break;
						// TODO case(TYPE_CHAR_ARRAY):break;
					}	
				}
				value->lock = false;
			}
			return *this;
		}


		template<typename T>
		SerialIndex& sendVariable(char* theIndex, T &t) {
			Serialio.print(theIndex);
			Serialio.print(DELIMITER);
			Serialio.print(t);
			Serialio.print("\n\r");
			return *this;
		}

		template<typename T>
		SerialIndex& sendArray(char* theIndex, T t, int theLen) {
			Serialio.print(theIndex);
			Serialio.print(DELIMITER);
			Serialio.print("[");
			size_t j;
			for(j=0;j<theLen-1;j++) {
				Serialio.print(*(t+j));
				Serialio.print(',');
			}
			Serial.print(*(t+(theLen-1)));
			Serialio.print("]\n\r");
			return *this;
		}

		void evaluate(const char* input) 
		{
			char t[strlen(input)+1]; /* need to add +1 to make the input end with a \0 */
			strcpy(t,input);
			int i = strlen(t); /* store the total length of the string */
			char* item = strtok(t,DELIMITER); /* split the string */
			char key[strlen(t)]; /* declare a buffer for the key*/
			char valueBuffer[i-strlen(t)]; /* declare a buffer for the value */ 
			int n = 0;

			while(item) {
				switch(n++) {
					case(0):strcpy(key,item);break;
					case(1):
					Value* value = get(key);

					if(value==NULL) {
						return;
					}

					if(value->in==false) {
						return;
					}

					value->lock = true;

					
					if(strchr(item,'[')!=NULL) { /* if we find a [, lets assume we are dealing with an array*/
						/* check if [ is the first character of the string to make sure it is an array */
						
						switch(value->type) {
							case(TYPE_INT_ARRAY):
							{
								Array<int*> *data = static_cast<Array<int*>*>(value->data);
								int* arr = toArray(item,TYPE_INT_ARRAY)->i;
								/* first element is the size of the array, hence i+1 below */
								size_t size = (arr[0]<data->size) ? arr[0]:data->size;
								size_t i;	
								for(i=0;i<size;i++) {
									*(data->now+i) = arr[i+1]==NULL ? *(data->now+i):arr[i+1];
								}
								/* invoke trigger function if available */
								(getFn(value->index))();

								/* update then*/
								size_t j;
								for(j=0;j<data->size;j++) {
									*(data->then+j) = *(data->now+j);
								}
								free(arr);
							}
							break;
							case(TYPE_FLOAT_ARRAY):
							{
								Array<float*> *data = static_cast<Array<float*>*>(value->data);
								float* arr = toArray(item,TYPE_FLOAT_ARRAY)->f;
								/* first element here is the size of array arr, hence i+1 below */
								size_t size = (arr[0]<data->size) ? arr[0]:data->size;
								size_t i;	
								for(i=0;i<size;i++) {
									*(data->now+i) = arr[i+1]==NULL ? *(data->now+i):arr[i+1];
								}
								/* invoke trigger function if available */
								(getFn(value->index))();

								/* update then*/
								size_t j;
								for(j=0;j<data->size;j++) {
									*(data->then+j) = *(data->now+j);
								}
								free(arr);
							}
							break;
						}

					} else {
						if(isNumeric(item)) {
							switch(value->type) {
								case(TYPE_INT):
								{
									Variable<int> *data = static_cast<Variable<int>*>(value->data);
									*data->now = atoi(item);
									/* invoke trigger function if available */
									(getFn(value->index))();
									/* update then */
									data->then = *data->now;
								}
								break;
								case(TYPE_FLOAT):
								{
									Variable<float> *data = static_cast<Variable<float>*>(value->data);
									*data->now = atof(item);
									/* invoke trigger function if available */
									(getFn(value->index))();
									/* update then */
									data->then = *data->now;
								}
								break;
							}	
						} else {
							/* if all checks failed so far, we will treat the value as a string */
							switch(value->type) {
								case(TYPE_STRING):
								{
									// Array<char**> *data = static_cast<Array<char**>*>(value->data);
									// TODO see add()
									// can't assign new value/pointer to data->now
								}
								break;
								case(TYPE_CHAR_ARRAY):
								{
									Array<char*> *data = static_cast<Array<char*>*>(value->data);
									if( strlen(item) < data->size) {
										strcpy(data->now,item);
									}
								}
								break;

							}
						}				
					}
					strcpy(valueBuffer,item);
					break;
				}
				item = strtok(NULL,DELIMITER);
			}
			delete item;
		}

		int isNumeric (const char* s) 
		{
		    if (s == NULL || *s == '\0' || isspace(*s)) {
		      return 0;
		    }
		    char * p;
		    strtod (s, &p);
		    return *p == '\0';
		}
		

		 Format* toArray(const char* item, const char theType) 
		 {
		  const char delimiter[] = ",";
		  char string[strlen(item)+1];
		  strcpy(string, item);
		  memmove (string, string+1, strlen (string+1));
		  char *s = string;
		  int count;
		  for (count=0; s[count]; s[count]==delimiter[0] ? count++ : *s++);
		  count++;
		  size_t index = 0;
		  union Format format;

		  switch(theType) {
		  	case(TYPE_FLOAT_ARRAY):format.f = new float[count+1]; format.f[index++] = count; break;
		  	case(TYPE_INT_ARRAY):format.i = new int[count+1]; format.i[index++] = count; break;
		  	case(TYPE_CHAR_ARRAY):format.c = new char[count+1]; format.c[index++] = count;break;
		  	default: return NULL;
		  }
		  
		  char* token;
		  token = strtok( string, delimiter );

		  while(token!=NULL) {
		    switch(theType) {
		    	case(TYPE_INT_ARRAY):format.i[index++] = (*token=='_') ? NULL : atoi(token);break;
		    	case(TYPE_FLOAT_ARRAY):format.f[index++] = (*token=='_') ? NULL : atof(token);break;
		    	// TODO case(TYPE_CHAR_ARRAY):format.c[index++] = (*token=='_') ? NULL : (char)(token);break;
		    	default:return NULL;
			}
		    token = strtok( NULL, delimiter ); /* Get next token */
		  }
		  return &format;
		}


		Value* get(const char* k) 
		{
			size_t i;
			for(i=0;i<values_size;i++) {
				Value *value = values[i];
				if(strcmp (value->index, k)==0) {
					return value;
				}
			}
			return NULL;
		}

		fptr getFn(const char* k) 
		{
			size_t i;
			for(i=0;i<functions_size;i++) {
				Function *fn = functions[i];
				if(strcmp (fn->index, k)==0) {
					return fn->fn;
				}

			}
			return &nil;
		}

		static void nil() {}

	private:
		Stream &Serialio;
		bool isWrite;
		bool isRead;
		char* buffer;
		int bufferindex;

		Value** values;
		int values_size;
		int values_capacity;

		Function** functions;
		int functions_size;
		int functions_capacity;

		 /* currently the capacity of the array containing all values is fixed but should be dynamic in the future. */
		void resize() 
		{
			if(values_size==0) {
				values = ( Value** )malloc(values_capacity * sizeof(Value*));
			} else if (values_size==values_capacity) {
				// TODO realloc
			}

			if(functions_size==0) {
				functions = ( Function** )malloc(functions_capacity * sizeof(Function*));
			} else if (functions_size==functions_capacity) {
				// TODO realloc
			}
		}

		template<class T> void addVariable( const char* k, T &t, char c, float theTolerance) 
		{
			Value *value = new Value(k,c);
			Variable<T> *data = new Variable<T>();
			data->now = &t;
			data->then = t;
			data->tolerance = theTolerance;
			value->data = data;
			values[values_size++] = value;
		}

		template<class T> void addArray(const char* k, T t, int s, char c) 
		{
			Value *value = new Value(k,c);
			Array<T> *data = new Array<T>();
			value->data = data;
			data->now = t;
			data->size = s;
			data->then = (T)malloc((data->size) * sizeof(T));
			memcpy(data->then, data->now, data->size);
			value->data = data;
			values[values_size++] = value;
		}

		
		
		
};


extern SerialIndex Index;

#endif /* SerialIndex_h */



/* Notes
 * [1] simple Vector class for Arduino; http://forum.arduino.cc/index.php?topic=45626.0
 * [2] how to free and reallocate memory; http://stackoverflow.com/questions/7159472/how-can-i-free-memory-from-my-structure-containing-an-array-of-dynamically-creat
 * [3] sizeof_array; http://stackoverflow.com/questions/3368883/how-does-this-size-of-array-template-function-work
 */
