import sojamo.serialindex.*;
import processing.serial.*;

SerialIndex index;

int dim = 0;
float n = 0;

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
