package sojamo.serialindex;

import java.lang.reflect.Field;

public class Value {

	private Object now;
	private Object then;
	private final String index;
	private final Object target;
	private final Field field;

	Value( String theIndex , Object theTarget , Field theField ) {
		index = theIndex;
		target = theTarget;
		field = theField;
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

	public Object get( ) throws IllegalArgumentException , IllegalAccessException {
		return field.get( target );
	}
}
