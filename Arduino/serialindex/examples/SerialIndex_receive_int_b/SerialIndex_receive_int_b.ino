
/**
 * use Processing sketch or type 
 * dim=50
 * to increase the brightness of the LED
 */

#include <SerialIndex.h>

int dim = 0;
const int led = 11;
int n;
void setup() {
  Index.begin(57600).add("n",n).add("dim", dim, 2).add("dim", &fdim);
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

