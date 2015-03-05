# SerialIndex

The implementation of SerialIndex enables applications to update the value of a variable across platforms when change is detected. Here the value of a variable is linked through a text based protocol primarily used over serial communication but equally applicable to other forms of communication channels. The objective of this project is to automatically synchronize variables and their values across applications and platforms. At this point this is work in progress.

``` Processing

	import sojamo.serialindex.*;
	import jssc.*;

	SerialIndex index;

	int var1 = 0;

	void setup() {
	  size(300,200);
	  index = new SerialIndex(this, "/dev/tty.usbmodem1421",9600);
	  index.subscribe("var1");
	  background(0);
	}

	void draw() {
	  fill(0);
	  rect(0,0,width,height/2);
	  fill(255);
	  text("current value: "+var1, 20 , height/2 - 20);
	}

	void mousePressed() {
	  var1 = int(random(100,500));
	  fill(0);
	  rect(0,height/2,width,height/2);
	  fill(255);
	  text("most recent update transmitted: "+var1, 20,height/2 + 20);
	}
```



``` Arduino

	#include <SerialIndex.h>

	int var1 = 500;
	long t1, t2;

	void setup() {
	  Index.begin(9600);
	  Index.subscribe("var1", var1);
	}

	void loop() {
	  
	  // check Index.updated every 20ms
	  if ((millis() - t1) >= 20) {
	    Index.update();
	    t1 = millis();
	  }
	  
	  // increment var1 every 2 seconds
	  if ((millis() - t2) >= 2000) {
	    var1++;
	    t2 = millis();
	  }

	  // add some delay-time
	  delay(10);
	}
```


_4 March 2015_