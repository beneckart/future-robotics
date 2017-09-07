import processing.serial.*; 
 
Serial myPort;    // The serial port
OPC opc;
PImage dot;

int NUM_TALISMAN = 12;
float DOT_SIZE = 0.05; // as a percentage of the map
float PULSE_PERCENT = 20.0;

int LORA_PKT_SIZE = 11;

int[] lora_buffer = new int[LORA_PKT_SIZE];
int pos = 0;

int[] batPercOthers = new int[NUM_TALISMAN];
int[] numSatOthers = new int[NUM_TALISMAN];
int[] hourOthers = new int[NUM_TALISMAN]; 
int[] minuteOthers = new int[NUM_TALISMAN]; 
int[] distOthers = new int[NUM_TALISMAN];
//int[] prevPixelId = new int[NUM_TALISMAN];
//int[] pixelId = new int[NUM_TALISMAN];
int[] updateTimes = new int[NUM_TALISMAN];

int last_recv = 0;

float MAN_LAT = 40.786400;
float MAN_LON = -119.206500;
float DEG_PER_RAD = (180. / 3.1415926535);
float METERS_PER_DEGREE = (40030230. / 360.);


float NORTH = 10.5;
float ROTATE_THETA = -NORTH/12.0*2.0*PI; //3.14159265;

float SCALE = 1.0;
float MINLAT = 0;
float MINLON = 0;


int[] deviceLocX = new int[NUM_TALISMAN];
int[] deviceLocY = new int[NUM_TALISMAN];

final int RESOLUTION = 500;

/*color[] colorMap ={color(255,0,0), //red
                   color(0,128,0), //green
                   color(0,0,255), //blue
                   color(255,255,0), //yellow
                   color(0,255,255), //aqua
                   color(128,0,128), //purple
                   color(0,255,0), //lime
                   color(0,128,128), //teal
                   color(128,128,0), //olive
                   color(255,0,255), //fuchsia
                   color(128,0,0), //maroon
                   color(128,128,128)}; //gray*/
                   
color[] colorMap = {color(31,120,180), //deep blue
                    color(178,223,138), // light green
                    color(51,160,44), // green
                    color(251,154,153), // pink
                    color(227,26,28), // red
                    color(253,191,111), // tan
                    color(255,127,0), // orangeish
                    color(202,178,214), // light purple
                    color(106,61,154), // purple
                    color(255,255,153), // light tan-ish
                    color(177,89,40), // brown
                    color(166,206,227)}; // light blue
                   

void setup()
{
  size(500, 500);
  frameRate(30);
  
  printArray(Serial.list()); 
  myPort = new Serial(this, "/dev/ttyUSB0", 9600); 
  
  // set up serial port to listen
  // buiuld circular buffer (just like arduino)
  // update deviceLocation and timeUpdatedArr
  // remove old points from deviceLocation array
  
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    deviceLocX[i] = (int)random(500);
    deviceLocY[i] = (int)random(500);
  }
  
  // Connect to the local instance of fcserver
  opc = new OPC(this, "127.0.0.1", 7890);
  
  opc.showLocations(true);
  
  opc.setColorCorrection(2.5, 1.0, 1.0, 1.0);

  dot = loadImage("dot3.png");
  
  String[] lines = loadStrings("MM_latlon");
  
  float[] latArr = new float[lines.length];
  float[] lonArr = new float[lines.length];
  int[] idArr = new int[lines.length];
  for (int i = 0 ; i < lines.length;  i++) {
     String[] tokens = split(lines[i], " ");
     int id = int(tokens[0]);
     //float lat = float(tokens[1])*METERS_PER_DEGREE;
     //float lon = float(tokens[2])*METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
     //float latR = rotateX(lat, lon, ROTATE_THETA);
     //float lonR = rotateY(lat, lon, ROTATE_THETA);
     float latlon[] = latlon_to_meters(float(tokens[1]), float(tokens[2]));
     latArr[i] = latlon[0];
     lonArr[i] = latlon[1];
     idArr[i] = id;
  }

  // now we have the (rotated) lat lon with temple at due north
  // let's find the bounds of the map and remap them to pixel coords  
  float minLat = min(latArr);
  float maxLat = max(latArr);
  float minLon = min(lonArr);
  float maxLon = max(lonArr);
  
  float widthLat = maxLat - minLat;
  float widthLon = maxLon - minLon;
  
  float scale = 1.0/max(widthLat, widthLon);
  //println(widthLat);
  //println(widthLon);
  
  SCALE = scale;
  MINLAT = minLat;
  MINLON = minLon;
  
  // we know RESOLUTION, so we can use the bounds to 
  // first scale to 0-1
  
  //println("!" + scale + " " + MINLAT + " " + MINLON);
  
  for(int i = 0; i < latArr.length; i++)
  {
    int[] pixel_coord = to_pixel_coord(latArr[i], lonArr[i], scale, minLat, minLon, true);
    //println(idArr[i] + ": " + pixel_coord[0] + " " + pixel_coord[1]);
    opc.led(idArr[i], pixel_coord[0], pixel_coord[1]);  
  }
}

float[] playa_to_latlon(int hour, int minute, int dist)
{
    float clock_hours = hour + ((float)minute)/60.0;
    
    float bearing = (((clock_hours - NORTH)/12.)*360.);
    
    float m_dy = dist * cos(bearing/DEG_PER_RAD); 
    float m_dx = dist * sin(bearing/DEG_PER_RAD);
    
    float dlon = m_dx/(METERS_PER_DEGREE*cos(MAN_LAT / DEG_PER_RAD));
    float dlat = m_dy/METERS_PER_DEGREE;
    
    float lat = dlat + MAN_LAT;
    float lon = dlon + MAN_LON;

    float[] latlon = new float[2];

    latlon[0] = lat;
    latlon[1] = lon;
    return latlon;
}

float[] latlon_to_meters(float lat, float lon)
{
   float lat_m = lat*METERS_PER_DEGREE;
   float lon_m = lon*METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
   float latR = rotateX(lat_m, lon_m, ROTATE_THETA);
   float lonR = rotateY(lat_m, lon_m, ROTATE_THETA);
   float[] latlon = new float[2];
   latlon[0] = latR;
   latlon[1] = lonR;
   return latlon;
   
}

int[] to_pixel_coord(float lat, float lon, float scale, float minLat, float minLon, boolean reverse)
{
    float lat_normalized = scale*(lat - minLat);
    float lon_normalized = scale*(lon - minLon);
    
    int pixel_coord[] = new int[2];
    //pixel_coord[0] = min(int((lat_normalized*RESOLUTION)+0.5), RESOLUTION-1); //
    if(reverse)
      pixel_coord[0] = RESOLUTION-min(int((lat_normalized*RESOLUTION)+0.5), RESOLUTION);
    else
      pixel_coord[0] = min(int((lat_normalized*RESOLUTION)+0.5), RESOLUTION-1); //
    pixel_coord[1] = min(RESOLUTION - min(int((lon_normalized*RESOLUTION)+0.5), RESOLUTION), RESOLUTION-1);
    //pixel_coord[1] = min(int((lon_normalized*RESOLUTION)+0.5), RESOLUTION-1);
    return pixel_coord;
}

float rotateX(float x, float y, float theta)
{
    float xr = x*cos(theta) - y*sin(theta);
    //yr = x*sin(theta) + y*cos(theta)
    return xr;
}

float rotateY(float x, float y, float theta)
{
    //xr = x*cos(theta) - y*sin(theta)
    float yr = x*sin(theta) + y*cos(theta);
    return yr;
}

void serialEvent(Serial p) 
{ 
    int readIt =  (int)myPort.read();
    if(readIt != -1)
    {
        lora_buffer[pos%LORA_PKT_SIZE] = readIt; 

        int before = 0;
        if((pos-1)%LORA_PKT_SIZE < 0)
            before = (pos-1)%LORA_PKT_SIZE + LORA_PKT_SIZE;
        else        
            before = (pos-1)%LORA_PKT_SIZE;

        println(str(pos%LORA_PKT_SIZE) + ":" + str(before) + "--" + str(lora_buffer[pos%LORA_PKT_SIZE]) + " " + str(lora_buffer[before]));

        if(lora_buffer[before] == 0xBE && lora_buffer[pos%LORA_PKT_SIZE] == 0xEF)
        {      
            //println("BEEF");
            if(lora_buffer[(pos+1)%LORA_PKT_SIZE] == 0xDE && lora_buffer[(pos+2)%LORA_PKT_SIZE] == 0xAD)
            {
                //println("DEAD");
                int devId = (int)lora_buffer[(pos+3)%LORA_PKT_SIZE];
                batPercOthers[devId] = lora_buffer[(pos+4)%LORA_PKT_SIZE];
                numSatOthers[devId] = lora_buffer[(pos+5)%LORA_PKT_SIZE];
                hourOthers[devId] = lora_buffer[(pos+6)%LORA_PKT_SIZE];
                minuteOthers[devId] = lora_buffer[(pos+7)%LORA_PKT_SIZE];
                distOthers[devId] = lora_buffer[(pos+8)%LORA_PKT_SIZE] + lora_buffer[(pos+9)%LORA_PKT_SIZE]*256;

                println(str(hourOthers[devId]) + " " + str(minuteOthers[devId]) + " " + str(distOthers[devId]));

                last_recv = millis();

                updateTimes[devId] = last_recv;
            }
      
        }
        pos++;
    }
}

int frameIdx = 0;

void draw()
{
    background(0);
  
    //int i = (frameIdx/15)%NUM_TALISMAN;
    for(int i = 0; i < NUM_TALISMAN; i++)
    {
        // Change the dot size as a function of time, to make it "throb"
        float dotSize = height * DOT_SIZE * (1.0 + PULSE_PERCENT/100.0 * sin(millis() * 0.01));

        int hr;
        int minute;
        int dist;

        hr = hourOthers[i];
        minute = minuteOthers[i];
        dist = distOthers[i];
      
        if(dist == 0)
            continue;
      
        float[] latlon = playa_to_latlon(hr, minute, dist);
        float[] latlon_m = latlon_to_meters(latlon[0], latlon[1]);
        int[] pixelCoords = to_pixel_coord(latlon_m[0], latlon_m[1], SCALE, MINLAT, MINLON,false);    
    
        if(pixelCoords[0] != -1)
        {
            //color rgb = color(255,random(25),random(25));
            tint(colorMap[i]);//, 100);
            //blendMode(ADD);
            blendMode(8);
            image(dot, pixelCoords[0] - dotSize/2, pixelCoords[1] - dotSize/2, dotSize, dotSize);
        }
    }
    
    // REDRAW "ACTIVE" overlaid dot
    int i = (frameIdx)%NUM_TALISMAN;
    // Change the dot size as a function of time, to make it "throb"
    float dotSize = height * DOT_SIZE * (1.0 + PULSE_PERCENT/100.0 * sin(millis() * 0.01));

    int hr;
    int minute;
    int dist;

    hr = hourOthers[i];
    minute = minuteOthers[i];
    dist = distOthers[i];

    float[] latlon = playa_to_latlon(hr, minute, dist);
    float[] latlon_m = latlon_to_meters(latlon[0], latlon[1]);
    int[] pixelCoords = to_pixel_coord(latlon_m[0], latlon_m[1], SCALE, MINLAT, MINLON, false);    

    if(pixelCoords[0] != -1 && dist != 0)
    {
        //color rgb = color(255,random(25),random(25));
        tint(colorMap[i]);//, 100);
        blendMode(1);
        image(dot, pixelCoords[0] - dotSize/2, pixelCoords[1] - dotSize/2, dotSize, dotSize);
    }
    frameIdx++;
}
