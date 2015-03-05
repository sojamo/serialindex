import sojamo.serialindex.*;
import jssc.*;

SerialIndex index;

int var1 = 0;

void setup() {
  size(300,200);
  index = new SerialIndex(this, "/dev/tty.usbmodem1421",115200);
  index.subscribe("var1");
  background(0);
}

void draw() {
  fill(0);
  rect(0,0,width,height/2);
  fill(255);
  text("current: "+var1, 20 , height/2 - 20);
}

void mousePressed() {
  var1 = int(random(100,500));
  fill(0);
  rect(0,height/2,width,height/2);
  fill(255);
  text("most recent sent: "+var1, 20,height/2 + 20);
}
