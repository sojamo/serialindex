# SerialIndex

The implementation of SerialIndex enables applications to update the value of a variable across platforms when change is detected. Here the value of a variable is linked through a text based protocol primarily used over serial communication but equally applicable to other forms of communication channels. The objective of this project is to automatically synchronize variables and their values across applications and platforms. At this point this is work in progress.

``` Processing

	import sojamo.serialindex.*;
	import processing.serial.*;

	SerialIndex index;

	int dim = 0; // outgoing to Arduino
	float n = 0; // incoming from Arduino

	void setup() {
	  size(300,200);
	  SerialIndex.begin(this, "/dev/tty.usbmodem1421",57600).add("n").add("dim").listen("n");
	}

	void draw() {
	  background(map(dim,0,50,0,255));
	  noStroke();
	  fill(0,255,128);
	  ellipse(width/2,height/2,n,n); 
	}

	void mouseDragged() {
	  dim = int(constrain(map(mouseX,0,width,0,50),0,50));
	  println(dim);
	}

	void n() {
	  println("got n",n);
	}
```



``` Arduino

	#include <SerialIndex.h>

	int n; // outgoing to Processing
	int dim = 0; // incoming from Processing
	const int led = 11;

	void setup() {
	  Index.begin(57600).add("n",n,4).add("dim", dim).listen("dim", &fdim);
	  pinMode(led, OUTPUT);
	}

	void loop() {
	  
	  n = analogRead(A0);
	  
	  // update SerialIndex
	  Index.update();
	  
	  // add some delay-time
	  delay(10);
	}

	/* fdim will be the callback function for
	 * changes made to variable dim */
	void fdim() {
	  analogWrite(led, dim);
	}
```


_19 March 2015_