package sojamo.serialindex;

import java.lang.reflect.Field;

public class Value {

	static public char INT = 'i';
	static public char FLOAT = 'f';
	static public char STRING = 's';
	static public char INT_ARRAY = 'I';
	static public char FLOAT_ARRAY = 'F';
	static public char CHAR_ARRAY = 'C';
	static public char NOT_DEFINED = '?';

	private Object now;
	private Object then;
	private final String index;
	private final Object target;
	private final Field field;
	private char type = NOT_DEFINED;

	Value( String theIndex , Object theTarget , Field theField ) {
		index = theIndex;
		target = theTarget;
		field = theField;
		Class t = field.getType( );
		if ( t.isPrimitive( ) ) {
			type = ( t.equals( int.class ) ) ? 'i' : ( t.equals( float.class ) ) ? 'f' : '?';
		} else {
			type = ( t.equals( String.class ) ) ? 's' : t.equals( int[].class ) ? 'I' : t.equals( float[].class ) ? 'F' : t.equals( char[].class ) ? 'C' : '?';
		}
		try {
			now = field.get( target );
		} catch ( IllegalArgumentException | IllegalAccessException e ) {
			e.printStackTrace( );
		}
		setThen( now );
	}

	Value setNow( Object theValue ) {
		now = theValue;
		return this;
	}

	Value setThen( Object theValue ) {
		then = theValue;
		return this;
	}

	Object getNow( ) {
		return now;
	}

	Object getThen( ) {
		return then;
	}

	public String getIndex( ) {
		return index;
	}

	public char getType( ) {
		return type;
	}

	public Object get( ) throws IllegalArgumentException , IllegalAccessException {
		return field.get( target );
	}
}
