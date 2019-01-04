OPC opc;
String[] headlines = {
  "Deck the halls",
  "Grape pie for good boys and girls",
  "Don't tip a lit candle",
  "Stockings hung with care",
  "Marshmallows and HOT chocolate warm my belly",
  "Hello Santa",
  "Mermaids clean the ocean",
  "Al I want for Christmas is Al ^-^",
  "* * * * * *",
  "Cooooookies!!" ,
  "PRESENTS!!!" ,
  "Ho ho ho!!" ,
  "Elise and Elena are cuties" ,
  "Santa is Welcome :)" ,
  "Ben is AWESOME" ,
  
};

PFont f; // Global font variable
float x; // Horizontal location
int index = 0;

void setup() {
  size(660,220);
  f = createFont( "8pixel",180,false);
  textSize(1);
  background(0);
  x = width;


//obviously edit this:
  opc = new OPC(this, "10.0.0.14", 7890);
  //float rows = 8;
  //float spacing = height / rows;
  //opc.ledStrip(0, 24, width/2, (height/spacing)+rows/2, spacing, 0, false);
  //opc.ledStrip(64, 24, width/2, (height/spacing)+spacing+rows/2, spacing, 0, false);
  //opc.ledStrip(128, 24, width/2,(height/spacing)+spacing*2+rows/2, spacing, 0, false);
  //opc.ledStrip(192, 24, width/2, (height/spacing)+spacing*3+rows/2, spacing, 0, false);
  //opc.ledStrip(256, 24, width/2, (height/spacing)+spacing*4+rows/2, spacing, 0, false);
  //opc.ledStrip(320, 24, width/2, (height/spacing)+spacing*5+rows/2, spacing, 0, false);
  //opc.ledStrip(384, 24, width/2, (height/spacing)+spacing*6+rows/2, spacing, 0, false);
  //opc.ledStrip(448, 24, width/2, (height/spacing)+spacing*7+rows/2, spacing, 0, false);
  int pitch = 11;
  int k = 0;
  for(int i = 0; i < 20; i++)
  {
     for(int j = 0; j < 25; j++)
     {
        boolean fwd = i%2 == 1;
        if(fwd)
        {
          opc.led(k, (25-j-1)*pitch, (20-i-1)*pitch+8);
        }
        else
        {
          opc.led(k, j*pitch, (20-i-1)*pitch+8);
        }
        k++;
     }
  }
}

void draw() {
  background(0);
  //fill(255,0,0);
  
  textFont(f,255);
  textAlign (LEFT);

  text(headlines[index],x,200); 
  
  x = x - 15;
  
  float w = textWidth(headlines[index]); 
  if (x < -w) {
    x = width;
    index = (index + 1) % headlines.length; 
    fill(random(255),random(255),random(255));
  }
  delay(25);
}
