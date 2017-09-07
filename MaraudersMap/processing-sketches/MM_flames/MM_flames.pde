OPC opc;
PImage im;

void setup()
{
  size(200, 200);

  // Load a sample image
  im = loadImage("flames.jpeg");
  
  // Connect to the local instance of fcserver
  opc = new OPC(this, "127.0.0.1", 7890);
  
  opc.setColorCorrection(2.5, 1.0, 1.0, 1.0);

  opc.setDithering(true);
  
  //opc.setColorCorrection(2.5, 0.25, 0.25, 0.25);
  
  opc.showLocations(true);

  // Map one 64-LED strip to the center of the window
  //opc.ledStrip(0, 64, width/2, height/2, width / 70.0, 0, false);
  
  String[] lines = loadStrings("LEDs.txt");
  //String[] lines = loadStrings("MM_latlon");

  for (int i = 0 ; i < lines.length; i++) {
     //if(i < 64) continue;
     println(lines[i]);
     String[] tokens = split(lines[i], " ");
     opc.led(int(tokens[0]), 105-int(float(tokens[1])+.5), 105-int(float(tokens[2])+0.5));   
  }
  
}

void draw()
{
  // Scale the image so that it matches the width of the window
  int imHeight = im.height * width / im.width;

  // Scroll down slowly, and wrap around
  float speed = 0.005;
  float y = (millis() * -speed) % imHeight;
  
  // Use two copies of the image, so it seems to repeat infinitely  
  image(im, 0, y, width, imHeight);
  image(im, 0, y + imHeight, width, imHeight);
}
