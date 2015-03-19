
/**
 * type
 * n=1
 * to turn the built-in LED on, and type
 * n=0
 * to turn it off
 */

#include <SerialIndex.h>

int n = 1000;
const int led = 13;

void setup() {
  Index.begin(57600);
  Index.add("n", n);
  Index.add("n", &fn);
  pinMode(led, OUTPUT);
}

void loop() {
  // update SerialIndex
  Index.update();

  // add some delay-time
  delay(10);
}

/* fn will be the callback function for
 * changes made to variable n */
void fn() {
  digitalWrite(led, n == 0 ? LOW : HIGH);
}

