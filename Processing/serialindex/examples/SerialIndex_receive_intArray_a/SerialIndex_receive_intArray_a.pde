import sojamo.serialindex.*;
import processing.serial.*;

SerialIndex index;

int[] arr = new int[3];

void setup() {
  size(300,200);
  SerialIndex.begin(this, "/dev/tty.usbmodem1421",57600).add("arr");
}

void draw() {
  background(240);
  noStroke();
  fill(0,255,128);
  int s = arr[1]%100;
  ellipse(width/2,height/2,s,s); 
}

