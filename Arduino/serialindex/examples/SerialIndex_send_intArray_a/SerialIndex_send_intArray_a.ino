#include <SerialIndex.h>

int arr[] = {1,2,3};

void setup() {
  Index.begin(57600);
  Index.add("arr", arr);
}

void loop() {
  
  arr[1]++;
  
  // update SerialIndex
  Index.update();

  // add some delay-time
  delay(10);

}

