OPC opc;
PImage im;

int numLEDs;
String[] lines;
int[][] LEDs_loc;

int modeFlag = 0;
int numModes = 2;

void setup()
{
  size(256, 128, P3D);
  noStroke();
  // Load a sample image
  im = loadImage("flames_rotated.jpg");
  
  // Connect to the local instance of fcserver
  opc = new OPC(this, "127.0.0.1", 7890);
  
  opc.setColorCorrection(2.5, 1.0, 1.0, 1.0);

  opc.setDithering(true);
  
  //opc.setColorCorrection(2.5, 0.25, 0.25, 0.25);
  
  opc.showLocations(true);

  // Map one 64-LED strip to the center of the window
  //opc.ledStrip(0, 64, width/2, height/2, width / 70.0, 0, false);
  
  lines = loadStrings("LEDs.txt");
  //String[] lines = loadStrings("MM_latlon");
  
  numLEDs = lines.length;
  
  LEDs_loc = new int[numLEDs][2];

  for (int i = 0 ; i < numLEDs; i++) {
     //println(lines[i]);
     String[] tokens = split(lines[i], " ");
     opc.led(int(tokens[0]), 105-int(float(tokens[1])+.5), 105-int(float(tokens[2])+0.5));   
     LEDs_loc[i][0]  =  105-int(float(tokens[1])+.5);
     LEDs_loc[i][1]  =  105-int(float(tokens[2])+.5);
  }
  
    textureMode(NORMAL);
    beginShape();
    texture(im);
    vertex(0, 0, 0, 0);
    vertex(256, 0, 1, 0);
    vertex(256, 128, 1, 1);
    vertex(0, 128, 0, 1);
    endShape();
  
}

void keyPressed() 
{
  modeFlag++;
  println("Hello world!");
}

void draw()
{

  
  // Scale the image so that it matches the width of the window
  int imWidth = im.width * height / im.height;

  // Scroll down slowly, and wrap around
  float speed = 0.05;
  float x = (millis() * -speed) % imWidth;
  
  // Use two copies of the image, so it seems to repeat infinitely  
  //image(im, x, 0, imWidth, height);
  //image(im, x + imWidth, 0, imWidth, height);
  
  /*
  if(modeFlag%numModes == 0)
  {
    stroke(255);
    //noStroke();
    //fill(255);
  
    for (int i = 0 ; i < lines.length; i++) {  
       point(LEDs_loc[i][0], LEDs_loc[i][1]);
       //ellipse(LEDs_loc[i][0], LEDs_loc[i][1], 2, 2);
    }  
  }
  else
  {
    //stroke(255);
    noStroke();
    fill(255);
  
    for (int i = 0 ; i < lines.length; i++) {  
       //point(LEDs_loc[i][0], LEDs_loc[i][1]);
       ellipse(LEDs_loc[i][0], LEDs_loc[i][1], 2, 2);
    }  
  }*/

}
