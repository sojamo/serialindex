/**
 * SerialIndex, send int example B
 * In this example variable var1 is added to SerialIndex and
 * is monitored for changes over time. the value of var1 is 
 * continuously increasing and you will see the current state
 * inside the Serial Monitor when active.
 * To make changes to var1, use the Serial Monitor's send option 
 * and type (make sure the line ending is set to 'Both NL & CR'): 
 * var1=0
 * this will reset the value of var1 to 0.
 */
#include <SerialIndex.h>

int var1 = 500;

void setup() {
  Index.begin(57600);
  Index.add("var1", var1, 4);
}

void loop() {
  
  var1 = analogRead(A0);
  
  // update SerialIndex
  Index.update();

  // add some delay-time
  delay(10);

}

