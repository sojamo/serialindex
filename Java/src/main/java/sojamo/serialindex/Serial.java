package sojamo.serialindex;

import java.util.Observable;

import jssc.SerialPort;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;

public class Serial extends Observable implements SerialPortEventListener {

	private final SerialPort port;
	private byte[] buffer = new byte[ 32768 ];
	private int inBuffer = 0;
	private int readOffset = 0;
	private boolean isConnected;
	private final long startTime;

	public Serial( String thePort , int theBaudrate ) {
		this( thePort , theBaudrate , 'N' , 8 , 1 );
	}

	public Serial( String thePort , int theBaudrate , char theParity , int theDataBits , float theStopBits ) {
		startTime = System.currentTimeMillis( );
		// setup parity
		char parity;
		if ( theParity == 'O' ) {
			parity = SerialPort.PARITY_ODD;
		} else if ( theParity == 'E' ) {
			parity = SerialPort.PARITY_EVEN;
		} else if ( theParity == 'M' ) {
			parity = SerialPort.PARITY_MARK;
		} else if ( theParity == 'S' ) {
			parity = SerialPort.PARITY_SPACE;
		} else {
			parity = SerialPort.PARITY_NONE;
		}

		// setup stop bits
		int stopBitsIdx = SerialPort.STOPBITS_1;
		if ( theStopBits == 1.5f ) {
			stopBitsIdx = SerialPort.STOPBITS_1_5;
		} else if ( theStopBits == 2 ) {
			stopBitsIdx = SerialPort.STOPBITS_2;
		}

		port = new SerialPort( thePort );
		try {
			port.openPort( );
			port.setParams( theBaudrate , theDataBits , stopBitsIdx , parity );
			port.addEventListener( this , SerialPort.MASK_RXCHAR );
			isConnected = true;
		} catch ( SerialPortException e ) {
			System.err.println( "Error opening serial port " + e.getPortName( ) + ": " + e.getExceptionType( ) );
			isConnected = false;
		}

	}

	public boolean isConnected( ) {
		return isConnected;
	}

	public boolean writeString( String theString ) {
		if ( !isConnected ) {
			return false;
		}
		try {
			SerialIndex.debug( "Serial.writeString()" , theString );
			return port.writeString( theString );
		} catch ( SerialPortException e ) {
			SerialIndex.printerr( e );
			return false;
		}
	}

	public byte[] readBytesUntil( int inByte ) {
		if ( inBuffer == readOffset ) {
			return null;
		}

		synchronized ( buffer ) {
			// look for needle in buffer
			int found = -1;
			for ( int i = readOffset ; i < inBuffer ; i++ ) {
				if ( buffer[ i ] == ( byte ) inByte ) {
					found = i;
					break;
				}
			}
			if ( found == -1 ) {
				return null;
			}

			int toCopy = found - readOffset + 1;
			byte[] dest = new byte[ toCopy ];
			System.arraycopy( buffer , readOffset , dest , 0 , toCopy );
			readOffset += toCopy;
			if ( inBuffer == readOffset ) {
				inBuffer = 0;
				readOffset = 0;
			}
			return dest;
		}
	}

	public String readStringUntil( int inByte ) {
		byte temp[] = readBytesUntil( inByte );
		if ( temp == null ) {
			return null;
		} else {
			return new String( temp );
		}
	}

	public void serialEvent( SerialPortEvent theEvent ) {
		if ( theEvent.getEventType( ) == SerialPortEvent.RXCHAR ) {
			int toRead;

			try {
				while ( 0 < ( toRead = port.getInputBufferBytesCount( ) ) ) {
					synchronized ( buffer ) {
						// enlarge buffer if necessary
						if ( buffer.length < inBuffer + toRead ) {
							byte temp[] = new byte[ buffer.length << 1 ];
							System.arraycopy( buffer , 0 , temp , 0 , inBuffer );
							buffer = temp;
						}
						byte[] read = port.readBytes( toRead );
						System.arraycopy( read , 0 , buffer , inBuffer , read.length );
						inBuffer += read.length;
					}
				}
				String s = readStringUntil( ( byte ) 10 );

				if ( s != null ) {
					SerialIndex.debug( "Serial received " + s.trim( ) + " " + ( int ) ( ( System.currentTimeMillis( ) - startTime ) / 1000 ) );
					setChanged( );
					notifyObservers( s.trim( ) );
				}
			} catch ( SerialPortException e ) {
				throw new RuntimeException( "Error reading from serial port " + e.getPortName( ) + ": " + e.getExceptionType( ) );
			}
		}

	}
}
