import processing.serial.*; 
 
//String PLAYBACK_FILE = "playback2017.txt"; 
//String PLAYBACK_FILE = "playback2018.txt"; 
String PLAYBACK_FILE = "playback2017-2018.txt"; //both years in one file
 
// relative speed of simulator
// 60 = a minute of real time per second of simulation
// 600 = 10 minutes of real time per second of simulation
// 1800 = 30 minutes of real time per second of simulation
// 3600 = an hour of real time per second of simulation
// etc.. (a week is 168 hours, so 3600--> <3 minute simulation)
int SIM_TIME_RATIO = 1800;
int SIMULATION_TIME_OFFSET_HRS = 0; //skip some hours at the beginning

final int FRAMERATE = 60;

// NOTE: if this is changed you need to change 
// it inside size() on first line of setup()
final int RESOLUTION = 1200; 

// should be greater than number of MAC addresses in logs
// in 2017, there were 10 unique addresses
// in 2018, there were 13 unique addresses
int NUM_TALISMAN = 24; 
float DOT_SIZE = 0.04; // as a percentage of the map
float PULSE_PERCENT = 5.0;

int STALEDATA_SECONDS = 60*60*3; // 3 hours

float simTime0 = millis()-1000*60*60*SIMULATION_TIME_OFFSET_HRS/SIM_TIME_RATIO;
float simTimeCurr = millis();

String[] playbackData;
int playbackIdx = 0;
 
//Serial myPort;    // The serial port
//OPC opc;
PImage dot;

boolean SHOW_LOCATIONS = true;

int LORA_PKT_SIZE = 11;

int[] lora_buffer = new int[LORA_PKT_SIZE];
int pos = 0;

int[] batPercOthers = new int[NUM_TALISMAN];
int[] numSatOthers = new int[NUM_TALISMAN];
int[] hourOthers = new int[NUM_TALISMAN]; 
int[] minuteOthers = new int[NUM_TALISMAN]; 
int[] distOthers = new int[NUM_TALISMAN];
float[] spdOthers = new float[NUM_TALISMAN];
float[] latOthers = new float[NUM_TALISMAN];
float[] lonOthers = new float[NUM_TALISMAN];
//int[] prevPixelId = new int[NUM_TALISMAN];
//int[] pixelId = new int[NUM_TALISMAN];
int[] updateTimes = new int[NUM_TALISMAN];
boolean[] logIsFull = new boolean[NUM_TALISMAN];

int last_recv = 0;

float SCALE = 1.0;
float MINLAT = 0;
float MINLON = 0;

int MAXDIST = 1000;
boolean REVERSE = true;

int[] deviceLocX = new int[NUM_TALISMAN];
int[] deviceLocY = new int[NUM_TALISMAN];

int[] lastPlaybackIdx = new int[NUM_TALISMAN];
int[] numDataPoints = new int[NUM_TALISMAN];
int[] devDatapoint = new int[NUM_TALISMAN];

float[] LATARR = new float[500];
float[] LONARR = new float[500];
int[] pixelLocationsLED_x = new int[500];
int[] pixelLocationsLED_y = new int[500];
int[][][] pixelTrails;

String[] nameList = new String[NUM_TALISMAN];

StringDict nameMap;       
IntDict idNumMap;

PImage bm_map;
PGraphics backgroundMap;
PGraphics trailMap;
int TRAILSTROKE = 4;
                    
void setup()
{
  size(1200, 1200);
  frameRate(FRAMERATE);
  
  nameMap = new StringDict();
  nameMap.set("FCD8C53A7D80", "Steve#1"); //0, 2 entries
  nameMap.set("C8D7C53A7D80", "Hank#2"); //1, 2411 entries
  nameMap.set("54D8C53A7D80", "Dad"); //2, 17.5k entries
  nameMap.set("88D8C53A7D80", "Steve#4"); //3, 2 entries
  nameMap.set("A8D8C53A7D80", "Hank#1"); //4, 278 entries
  nameMap.set("18D8C53A7D80", "Minimap"); //0, 10k entries
  nameMap.set("E0D8C53A7D80", "Elliot"); //4, 13k entries (2.7k w/0 as well)
  nameMap.set("60D8C53A7D80", "Jimmy"); //5, 12k entries (3: 2 entries, 9: 44 entries)
  nameMap.set("98D7C53A7D80", "Luca"); //6, 14.4k entries (2k data points w/5 as well)
  nameMap.set("7CD9C53A7D80", "??-1"); // 0 entries
  nameMap.set("??", "??-2"); //
  nameMap.set("08D9C53A7D80", "Shervin"); //9, 14k entries (4:48, 5:412, 15:908)
  nameMap.set("94D8C53A7D80", "JP"); //11, 10k entries
  nameMap.set("7CD8C53A7D80", "Ash"); //11, 17.1k entries
  nameMap.set("device00_2017", "2017-1"); //, 0):  7793 good entries
  nameMap.set("device06_2017", "2017-2");//, 1):   702 good entries
  nameMap.set("device05_2017", "2017-3");//, 2):  2612 good entries
  nameMap.set("device02_2017", "2017-4");//, 3):  2123 good entries
  nameMap.set("device03_2017", "2017-5");//, 4):  2143 good entries
  nameMap.set("device04_2017", "2017-6");//, 5):  1729 good entries
  nameMap.set("device01_2017", "2017-7");//, 6):  3454 good entries
  nameMap.set("device08_2017", "2017-8");//, 0):     5 good entries
  nameMap.set("device09_2017", "2017-9");//, 9):  5930 good entries
  nameMap.set("device10_2017", "2017-10");//, 10):  5695 good entries
  
  idNumMap = new IntDict();
  idNumMap.set("18D8C53A7D80", 0); //0, 10k entries
  idNumMap.set("C8D7C53A7D80", 1); //1, 2411 entries
  idNumMap.set("54D8C53A7D80", 2); //2, 17.5k entries
  idNumMap.set("88D8C53A7D80", 3); //3, 2 entries
  idNumMap.set("A8D8C53A7D80", 4); //4, 278 entries
  idNumMap.set("60D8C53A7D80", 5); //5, 12k entries (3: 2 entries, 9: 44 entries)
  idNumMap.set("98D7C53A7D80", 6); //6, 14.4k entries (2k data points w/5 as well)
  idNumMap.set("E0D8C53A7D80", 7); //4, 13k entries (2.7k w/0 as well)
  idNumMap.set("FCD8C53A7D80", 8); //0, 2 entries
  idNumMap.set("08D9C53A7D80", 9); //9, 14k entries (4:48, 5:412, 15:908)
  idNumMap.set("94D8C53A7D80", 10); //11, 10k entries
  idNumMap.set("7CD8C53A7D80", 11); //11, 17.1k entries
  idNumMap.set("device00_2017", 12); //, 0):  7793 good entries
  idNumMap.set("device06_2017", 13);//, 1):   702 good entries
  idNumMap.set("device05_2017", 14);//, 2):  2612 good entries
  idNumMap.set("device02_2017", 15);//, 3):  2123 good entries
  idNumMap.set("device03_2017", 16);//, 4):  2143 good entries
  idNumMap.set("device04_2017", 17);//, 5):  1729 good entries
  idNumMap.set("device01_2017", 18);//, 6):  3454 good entries
  idNumMap.set("device08_2017", 19);//, 0):     5 good entries
  idNumMap.set("device09_2017", 20);//, 9):  5930 good entries
  idNumMap.set("device10_2017", 21);//, 10):  5695 good entries

  playbackData = loadStrings(PLAYBACK_FILE);
  
  // run through data once to find:
  // 1) last log entry for each device
  // 2) how many entries per device
  for(int i = 0; i < playbackData.length; i++)
  {
    int devId = idNumMap.get(split(playbackData[i], " ")[1]);
    lastPlaybackIdx[devId] = i;
    numDataPoints[devId]++;
  }
  
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    if(numDataPoints[i] == 0) logIsFull[i] = true;  
  }
  
  for(int i = 0; i < NUM_TALISMAN; i++) nameList[i] = "";
  
  //populate name list from idNumMap
  String[] theKeys = idNumMap.keyArray();
  for(int i = 0; i < theKeys.length; i++)
  {
    String devId_unique = theKeys[i];
    int devId_int = idNumMap.get(devId_unique);
    String name = nameMap.get(devId_unique);
    nameList[devId_int] = name;
  }
  
  // find the longest log
  int maxDatapoints = 0;
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    println("Talisman " + i + ": " + numDataPoints[i] + " datapoints");
    if(numDataPoints[i] > maxDatapoints) maxDatapoints = numDataPoints[i];
  }
  
  // preallocate trails to longest log in contiguous 3d tensor
  pixelTrails = new int[NUM_TALISMAN][maxDatapoints][2];
  
  for(int i=0; i < NUM_TALISMAN; i++) {shuffledIdx.append(i);}
  
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    deviceLocX[i] = (int)random(RESOLUTION-1);
    deviceLocY[i] = (int)random(RESOLUTION-1);
  }
  
  // Connect to the local instance of fcserver
  //opc = new OPC(this, "127.0.0.1", 7890);
  //opc = new OPC(this, "10.10.10.1", 7890);
  //opc = new OPC(this, "192.168.0.103", 7890);
  
  //opc.showLocations(SHOW_LOCATIONS);
  
  //opc.setColorCorrection(2.5, 1.0, 1.0, 1.0);

  dot = loadImage("dot3.png");
  bm_map = loadImage("BM_map_black.png");
  bm_map.resize(RESOLUTION, RESOLUTION);
  
  String[] lines = loadStrings("MM_latlon");
  float[] latArr = new float[lines.length];
  float[] lonArr = new float[lines.length];
  int[] idArr = new int[lines.length];
  for (int i = 0 ; i < lines.length;  i++) {
     String[] tokens = split(lines[i], " ");
     int id = int(tokens[0]);
     float latlon[] = latlon_to_meters(float(tokens[1]), float(tokens[2]));
     latArr[i] = latlon[0];
     lonArr[i] = latlon[1];
     idArr[i] = id;
     LATARR[i] = latArr[i];
     LONARR[i] = lonArr[i];
  }
  
  // now we have the (rotated) lat lon with temple at due north
  // let's find the bounds of the map and remap them to pixel coords  
  float minLat = min(latArr)-100;
  float maxLat = max(latArr)+100;
  float minLon = min(lonArr)-100;
  float maxLon = max(lonArr)+100;
  
  float widthLat = maxLat - minLat;
  float widthLon = maxLon - minLon;
  
  float scale = 1.0/max(widthLat, widthLon);
  //println(widthLat);
  //println(widthLon);
  
  SCALE = scale;
  MINLAT = minLat;
  MINLON = minLon;
  

  for(int i = 0; i < latArr.length; i++)
  {
    int[] pixel_coord = to_pixel_coord(latArr[i], lonArr[i], scale, minLat, minLon, REVERSE);
    //println(idArr[i] + ": " + pixel_coord[0] + " " + pixel_coord[1]);
    //opc.led(idArr[i], pixel_coord[0], pixel_coord[1]);  
    pixelLocationsLED_x[i] = pixel_coord[0];
    pixelLocationsLED_y[i] = pixel_coord[1];
  }
  
  //let draw the map background once
  backgroundMap = createGraphics(RESOLUTION, RESOLUTION);
  backgroundMap.beginDraw();
  //backgroundMap.background(0);
  // draw map
  for(int i = 0; i < 500; i++)
  {
    float dotSize = height * DOT_SIZE/6.0f;// * (1+5*PULSE_PERCENT/100.0 * cos(2*3.14*frameIdx/120.0));;
    backgroundMap.tint(color(255,0,0));//, 100);
    backgroundMap.image(dot, pixelLocationsLED_x[i] - dotSize/2, pixelLocationsLED_y[i] - dotSize/2, dotSize, dotSize);
  }
  backgroundMap.endDraw();
  
  //instantiate trailMap
  trailMap = createGraphics(RESOLUTION, RESOLUTION);
      
  println("Done with setup!");
}

void grabPlaybackData()
{
   //f.write("%d %d %f %f %d %f %f %d %d %d %d %d %d\n" 
   //% (time_in_s, dev_id, lat, lon, num_sat, spd, bat_perc, time_since_boot, month, day, hour, minute, second))
   simTimeCurr = millis() - simTime0; //time since simulation began
   float realTimeCurr = simTimeCurr*SIM_TIME_RATIO; //current simulation time
   
   // now, starting at playbackIdx, go forward until past realTimeCurr
   if(playbackIdx >= playbackData.length) return;
   String[] tokens = split(playbackData[playbackIdx], " ");
   int milliseconds = 1000*int(tokens[0]);
   trailMap.beginDraw();
   trailMap.strokeWeight(TRAILSTROKE); // for thickness of the line
   while(milliseconds < realTimeCurr)
   {
     int devId = idNumMap.get(tokens[1]);
     
     float lat = float(tokens[3]);
     float lon = float(tokens[4]);

     numSatOthers[devId] = int(tokens[5]);
     spdOthers[devId] = float(tokens[6]);
     batPercOthers[devId] = int(tokens[7]);
     
     //simMonth = int(tokens[9]);
     //simDay = int(tokens[10]);
     //simHour = int(tokens[11]);
     //simMinute = int(tokens[12]);
     
     latOthers[devId] = lat;
     lonOthers[devId] = lon;
     
     float[] latlon_m = latlon_to_meters(lat, lon);   
     int[] pixelCoords = to_pixel_coord(latlon_m[0], latlon_m[1], SCALE, MINLAT, MINLON, REVERSE);    
        
     pixelTrails[devId][devDatapoint[devId]][0] = pixelCoords[0];  
     pixelTrails[devId][devDatapoint[devId]][1] = pixelCoords[1];  
     
     if(devDatapoint[devId] > 0)
     {
       int trailx = pixelTrails[devId][devDatapoint[devId]][0];
       int traily = pixelTrails[devId][devDatapoint[devId]][1];
        
       int trailx2 = pixelTrails[devId][devDatapoint[devId]-1][0];
       int traily2 = pixelTrails[devId][devDatapoint[devId]-1][1];
        
       if(trailx != 0 && traily != 0 && trailx2 != 0 && traily2 != 0)
       {
          float dist = sqrt((trailx - trailx2)*(trailx - trailx2) + (traily - traily2)*(traily - traily2));
          if(dist < RESOLUTION/20)
          {
            trailMap.stroke(colorMap24Trans[devId]); // for color the line
          
            trailMap.line(trailx, traily, trailx2, traily2); 
          }
          
       }  
     }
     devDatapoint[devId]++;
     
        
     //hourOthers[devId] = ;
     //minuteOthers[devId] = ;
     //distOthers[devId] = ;

     //println(str(hourOthers[devId]) + " " + str(minuteOthers[devId]) + " " + str(distOthers[devId]));

     updateTimes[devId] = milliseconds/1000;    
     
     if(lastPlaybackIdx[devId] == playbackIdx)
     {
       logIsFull[devId] = true; 
     }
     
     playbackIdx++;
     if(playbackIdx >= playbackData.length) break;
     tokens = split(playbackData[playbackIdx], " ");
     milliseconds = 1000*int(tokens[0]);
     //println("updating...", milliseconds/1000.0, realTimeCurr/1000.0, playbackIdx);
   }
   //println("done with update", milliseconds/1000.0, realTimeCurr/1000.0, playbackIdx);
   trailMap.endDraw();
}

void fakeFrame()
{
      int devId = (int)0;
      batPercOthers[devId] = 99;
      numSatOthers[devId] = 11;
      hourOthers[devId] = 12;
      minuteOthers[devId] = 30;
      //distOthers[devId] = int(random(10))*100;
      distOthers[devId] = 100;

      //println(str(hourOthers[devId]) + " " + str(minuteOthers[devId]) + " " + str(distOthers[devId]));

      //last_recv = 0;//millis();
      last_recv = millis();

      updateTimes[devId] = last_recv; 
}

//Aug 1 is a Wednesday
String[] dayNames = {"Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday", "Monday" };
String parseDate(float seconds)
{
  int currDay = int(seconds / (24*60*60)) + 25;
  int currHour = int(seconds / (60*60)) % 24;
  int currMinute = int(seconds / (60)) % 60;
  
  int dispDay = currDay;
  int dispHour = currHour - 7; // GMT to PST
  if(dispHour < 1)
  {
    dispHour += 24;
    dispDay -= 1;
  }
  
  String dispMonthStr = "Aug ";
  if(dispDay >= 32)
  {
      dispMonthStr = "Sep ";
      dispDay -= 31;
  }
  
  String ampm = "a.m.";
  if(dispHour > 12)
  {
    ampm = "p.m.";
    dispHour -= 12;
  }
  
  String dispHourStr = str(dispHour);
  if(dispHour <= 9)
     dispHourStr = " " + str(dispHour);
     
  String dispMinuteStr = str(currMinute);
  if(currMinute <= 9)
     dispMinuteStr = "0" + str(currMinute);
     
  String dayNameStr = dayNames[dispDay%7];

  //", " + dispMonthStr + str(dispDay) + " " + 
  return dayNameStr + " " + dispHourStr + ":" + dispMinuteStr + " " + ampm;
}

int frameIdx = 0;
IntList shuffledIdx = new IntList();
float sim_finish_time;
boolean save_finish_time = true;

void draw()
{
  background(0);
  
  tint(255);
  
  image(bm_map, 0, 0);
  //image(backgroundMap, 0, 0); 
  image(trailMap, 0, 0);
  
  float curr_sim_in_real = (millis()-simTime0)*SIM_TIME_RATIO/1000.0f;
  
  boolean are_we_done = true;
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    if(!logIsFull[i]) are_we_done = false;
  }  
  
  if(are_we_done && save_finish_time)
  {
    sim_finish_time = curr_sim_in_real;
    save_finish_time = false; 
  }
  
  String dateStr;
  if(are_we_done)
    dateStr = parseDate(sim_finish_time);
  else
    dateStr = parseDate(curr_sim_in_real);
  //curr_sim_in_real = sim_finish_time;
  
  textSize(64);
  textAlign(LEFT, TOP);
  fill(color(255,255,255));//, 100);
  text(dateStr, 10, 5); 
  
  if(are_we_done)
  {
    textSize(46);
    text("Simulation Complete!", 10, 5+64+8); 
  }
  
  for(int j = 0; j < NUM_TALISMAN; j++)
  {
      int i = shuffledIdx.get(j);
      
      boolean is_stale = false;
      int s_in_real = updateTimes[i];
      if(curr_sim_in_real - s_in_real > STALEDATA_SECONDS)
        is_stale = true;

      // Change the dot size as a function of time, to make it "throb" if live updates coming
      float dotSize = height * DOT_SIZE;
      if(!is_stale) dotSize *= (1.0 + PULSE_PERCENT/100.0 * sin(millis() * 0.01));
      float[] latlon_m = latlon_to_meters(latOthers[i], lonOthers[i]);
      int[] pixelCoords = to_pixel_coord(latlon_m[0], latlon_m[1], SCALE, MINLAT, MINLON, REVERSE);    
      if(pixelCoords[0] != -1 && !logIsFull[i])
      {
          //color rgb = color(255,random(25),random(25));
          tint(colorMap24[i]);//, 100);
          image(dot, pixelCoords[0] - dotSize/2, pixelCoords[1] - dotSize/2, dotSize, dotSize);
      }  
    }

    for(int j = 0; j < NUM_TALISMAN; j++)
    {   
        int i = shuffledIdx.get(j);
        float[] latlon_m = latlon_to_meters(latOthers[i], lonOthers[i]);
        int[] pixelCoords = to_pixel_coord(latlon_m[0], latlon_m[1], SCALE, MINLAT, MINLON, REVERSE);    

        boolean is_stale = false;
        int s_in_real = updateTimes[i];
        if(curr_sim_in_real - s_in_real > STALEDATA_SECONDS)
            is_stale = true;
 
        /*if(is_stale)
        {
            textSize(14);
            //fill(colorMap[i]);
            fill(color(255,255,255));
            //stroke(color(255,255,255));
            textAlign(CENTER, TOP);
            text("Last Log: " + parseDate(s_in_real), pixelCoords[0], pixelCoords[1]+int(height*DOT_SIZE/2.0));         
        }*/
        /*if(logIsFull[i])
        {
           textSize(20);
           fill(color(255,255,255));
           textAlign(CENTER, TOP);
           text("Log Full :(", pixelCoords[0], pixelCoords[1]+int(height*DOT_SIZE));          
        }*/
    
        if(pixelCoords[0] != -1 && !is_stale)
        {
            String namei = nameList[i];
            if(namei.indexOf("2017") == -1)
            {
              textSize(64);
              textAlign(CENTER, TOP);
              stroketext(namei, pixelCoords[0], pixelCoords[1]-int(height*DOT_SIZE*2), 3);
              fill(colorMap24[i]);
              text(namei, pixelCoords[0], pixelCoords[1]-int(height*DOT_SIZE*2));
            }
          

        }
    }

    if(frameIdx % 60 == 0)
    {
      shuffledIdx.shuffle(); 
    }
    
    frameIdx++;
    grabPlaybackData();
}
