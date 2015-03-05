package sojamo.serialindex;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.logging.Logger;

import jssc.SerialPortList;

public final class SerialIndex extends Observable {

	static public boolean DEBUG = false;
	static public final Logger log = Logger.getLogger( SerialIndex.class.getName( ) );
	private final Map< String , Value > indices = new HashMap< String , Value >( );
	private final String delimiter = "=";
	private final Serial serial;
	private final BlockingQueue< String[] > queue;
	private final int queueCapacity = 512;
	private final Object target;

	public SerialIndex( String thePort , int theBaudrate ) {
		this( null , thePort , theBaudrate );
	}

	public SerialIndex( final Object theTarget , final String thePort , final int theBaudrate ) {
		target = theTarget;
		queue = new ArrayBlockingQueue< String[] >( queueCapacity );
		serial = new Serial( thePort , theBaudrate );
		if ( serial.isConnected( ) ) {
			serial.addObserver( new Observer( ) {
				public void update( Observable o , Object arg ) {
					String[] strs = ( ( String ) arg ).split( delimiter );
					if ( strs.length == 2 ) {
						queue.offer( strs );
						setChanged( );
						notifyObservers( strs );
					}
				}
			} );
		} else {
			log.warning( String.format( "Establishing connection with %s was unsuccessful. Maybe the device is not connected?" , thePort ) );
		}

		if ( theTarget != null ) {
			/* Lets check if our target is of type PApplet and if we are running a processing app.
			 * if this is the case, register our update function with PApplet's registerMethod */
			if ( theTarget.getClass( ).getSuperclass( ).getName( ).contains( "PApplet" ) ) {
				try {
					Method m = theTarget.getClass( ).getSuperclass( ).getDeclaredMethod( "registerMethod" , String.class , Object.class );
					m.setAccessible( true );
					try {
						m.invoke( theTarget , "draw" , this );
					} catch ( IllegalAccessException | IllegalArgumentException | InvocationTargetException e ) {
						debug( e );
					}
				} catch ( NoSuchMethodException | SecurityException e ) {
					debug( e );
				}
			}
		}
	}

	public void draw( ) {
		update( );
	}

	public SerialIndex subscribe( String theIndex , Object theValue ) {
		subscribe( theIndex );
		indices.get( theIndex ).setNow( theValue );
		return this;
	}

	public SerialIndex subscribe( String theIndex ) {
		Object member = evaluateMember( target , theIndex );
		if ( member != null && member instanceof Field ) {
			try {
				Field field = ( ( Field ) member );
				indices.put( theIndex , new Value( theIndex , target , field ) );
			} catch ( IllegalArgumentException e ) {
				printerr( e );
			}

		}
		return this;
	}

	public void update( ) {

		// 1) check current status of all subscriptions
		for ( Value value : indices.values( ) ) {
			try {
				int v = i( value.get( ) );
				if ( v != i( value.getNow( ) ) ) {
					send( String.format( "%s%s%s" , value.getIndex( ) , delimiter , v ) );
					value.setThen( value.getNow( ) );
					value.setNow( v );
					debug( "change detected" , value.getIndex( ) , value.getNow( ) , value.getThen( ) );
				}
			} catch ( IllegalArgumentException | IllegalAccessException e ) {
				printerr( e );
			}
		}

		// 2) update any changes received from the serial connection and invoke the target 
		if ( !queue.isEmpty( ) ) {
			Collection< String[] > c = new ArrayList< String[] >( );
			queue.drainTo( c );
			for ( String[] data : c ) {
				Value value = indices.get( data[ 0 ] );
				if ( value != null ) {
					value.setNow( i( data[ 1 ] ) );
					value.setThen( i( data[ 1 ] ) );
				}
				invoke( target , data[ 0 ] , i( data[ 1 ] ) );
				// TODO call Callbacks
			}
		}
	}

	public String toString( ) {
		return indices.toString( );
	}

	public boolean send( String theString ) {
		if ( serial != null ) {
			return serial.writeString( theString + "\n" );
		} else {
			return false;
		}
	}

	public List getList( ) {
		List l = new ArrayList( );
		Collections.addAll( l , SerialPortList.getPortNames( ) );
		return l;
	}

	public Map get( ) {
		return Collections.unmodifiableMap( indices );
	}

	static public void invoke( Object theObject , String theMember , Object ... theParams ) {
		Class[] cs = new Class[ theParams.length ];
		for ( int i = 0 ; i < theParams.length ; i++ ) {
			Class c = theParams[ i ].getClass( );
			cs[ i ] = classmap.containsKey( c ) ? classmap.get( c ) : c;
		}
		try {
			final Field f = theObject.getClass( ).getDeclaredField( theMember );
			f.setAccessible( true );
			Object o = theParams[ 0 ];
			Class cf = o.getClass( );
			if ( cf.equals( Integer.class ) ) {
				f.setInt( theObject , i( o ) );
			} else if ( cf.equals( Float.class ) ) {
				f.setFloat( theObject , f( o ) );
			} else if ( cf.equals( Long.class ) ) {
				f.setLong( theObject , l( o ) );
			} else if ( cf.equals( Double.class ) ) {
				f.setDouble( theObject , d( o ) );
			} else if ( cf.equals( Boolean.class ) ) {
				f.setBoolean( theObject , b( o ) );
			} else if ( cf.equals( Character.class ) ) {
				f.setChar( theObject , ( char ) i( o ) );
			} else {
				f.set( theObject , o );
			}

		} catch ( Exception e ) {
			try {
				final Method m = theObject.getClass( ).getDeclaredMethod( theMember , cs );
				m.setAccessible( true );
				try {
					m.invoke( theObject , theParams );
				} catch ( IllegalArgumentException | IllegalAccessException | InvocationTargetException e1 ) {
					debug( e1 );
				}

			} catch ( SecurityException | NoSuchMethodException e1 ) {
				debug( e1 );
			}
		}
	}

	private final Object evaluateMember( final Object theObject , final String theName ) {
		Class< ? > c = theObject.getClass( );
		while ( c != null ) {
			try {
				final Field field = c.getDeclaredField( theName );
				field.setAccessible( true );
				return field;
			} catch ( Exception e ) {
				try {
					final Method method = c.getMethod( theName , new Class< ? >[] { } );
					return method;
				} catch ( SecurityException e1 ) {
					printerr( e );
				} catch ( NoSuchMethodException e1 ) {
					printerr( e );
				}
			}
			c = c.getSuperclass( );
		}
		return null;
	}

	static public boolean b( Object o ) {
		return b( o , false );
	}

	static public boolean b( Object o , boolean theDefault ) {
		return ( o instanceof Boolean ) ? ( ( Boolean ) o ).booleanValue( ) : ( o instanceof Number ) ? ( ( Number ) o ).intValue( ) == 0 ? false : true : theDefault;
	}

	static public boolean b( String o ) {
		return b( o , false );
	}

	static public boolean b( String o , boolean theDefault ) {
		return o.equalsIgnoreCase( "true" ) ? true : theDefault;
	}

	static public int i( Object o ) {
		return i( o , Integer.MIN_VALUE );
	}

	static public int i( Object o , int theDefault ) {
		return ( o instanceof Number ) ? ( ( Number ) o ).intValue( ) : ( o instanceof String ) ? i( s( o ) ) : theDefault;
	}

	static public int i( String o ) {
		return i( o , Integer.MIN_VALUE );
	}

	static public int i( String o , int theDefault ) {
		return isNumeric( o ) ? Integer.parseInt( o ) : theDefault;
	}

	static public float f( Object o ) {
		return f( o , Float.MIN_VALUE );
	}

	static public float f( Object o , float theDefault ) {
		return ( o instanceof Number ) ? ( ( Number ) o ).floatValue( ) : ( o instanceof String ) ? f( s( o ) ) : theDefault;
	}

	static public float f( String o ) {
		return f( o , Float.MIN_VALUE );
	}

	static public float f( String o , float theDefault ) {
		return isNumeric( o ) ? Float.parseFloat( o ) : theDefault;
	}

	static public double d( Object o ) {
		return d( o , Double.MIN_VALUE );
	}

	static public double d( Object o , double theDefault ) {
		return ( o instanceof Number ) ? ( ( Number ) o ).doubleValue( ) : ( o instanceof String ) ? d( s( o ) ) : theDefault;
	}

	static public double d( String o ) {
		return d( o , Double.MIN_VALUE );
	}

	static public double d( String o , double theDefault ) {
		return isNumeric( o ) ? Double.parseDouble( o ) : theDefault;
	}

	static public long l( Object o ) {
		return l( o , Long.MIN_VALUE );
	}

	static public long l( Object o , long theDefault ) {
		return ( o instanceof Number ) ? ( ( Number ) o ).longValue( ) : ( o instanceof String ) ? l( s( o ) ) : theDefault;
	}

	static public String s( Object o ) {
		return ( o != null ) ? o.toString( ) : "";
	}

	static public String s( Object o , String theDefault ) {
		return ( o != null ) ? o.toString( ) : theDefault;
	}

	static Map< Class< ? > , Class< ? >> classmap = new HashMap( ) {
		{
			put( Integer.class , int.class );
			put( Float.class , float.class );
			put( Double.class , double.class );
			put( Boolean.class , boolean.class );
			put( Character.class , char.class );
			put( Long.class , long.class );
		}
	};

	static public boolean isNumeric( Object o ) {
		return isNumeric( o.toString( ) );
	}

	static public boolean isNumeric( String str ) {
		return str.matches( "(-|\\+)?\\d+(\\.\\d+)?" );
	}

	static public final float mapValue( float theValue , float theStart0 , float theStop0 , float theStart1 , float theStop1 ) {
		return theStart1 + ( theStop1 - theStart1 ) * ( ( theValue - theStart0 ) / ( theStop0 - theStart0 ) );
	}

	static public final double mapValue( double theValue , double theStart0 , double theStop0 , double theStart1 , double theStop1 ) {
		return theStart1 + ( theStop1 - theStart1 ) * ( ( theValue - theStart0 ) / ( theStop0 - theStart0 ) );
	}

	static public final float constrainValue( float theValue , float theMin , float theMax ) {
		return theValue < theMin ? theMin : ( theValue > theMax ? theMax : theValue );
	}

	static public final double constrainValue( double theValue , double theMin , double theMax ) {
		return theValue < theMin ? theMin : ( theValue > theMax ? theMax : theValue );
	}

	static public final int constrainValue( int theValue , int theMin , int theMax ) {
		return theValue < theMin ? theMin : ( theValue > theMax ? theMax : theValue );
	}

	static public void println( final Object ... strs ) {
		for ( Object str : strs ) {
			System.out.print( str + " " );
		}
		System.out.println( );
	}

	static public void debug( final Object ... strs ) {
		if ( DEBUG ) {
			println( strs );
		}
	}

	static public void printerr( final Object ... strs ) {
		for ( Object str : strs ) {
			System.err.print( str + " " );
		}
		System.err.println( );
	}

}
