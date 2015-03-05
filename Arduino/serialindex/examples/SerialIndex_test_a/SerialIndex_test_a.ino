#include <SerialIndex.h>

int var1 = 500;
long t1, t2;

void setup() {
  Index.begin(115200);
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
