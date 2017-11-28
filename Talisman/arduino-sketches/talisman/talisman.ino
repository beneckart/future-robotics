
#include "SoftwareSerial.h"
//#include <Adafruit_NeoPixel.h>
#include "TinyGPS++.h"
#include "FS.h"
#include "NeoPatterns.cpp"
#include "Switch.h"

#define VERSION 0.9

//UNCOMMENT OUT IF AT BM!!
#define PRODUCTION

/*
 * User device variables
 */

#define DEVICE_ID 0

//#define GPS_BAUD 115200
#define GPS_BAUD 9600 

// delays
#define LED_MAP_UPDATE_DELAY_MS 250

#define BATTERY_CHECK_DELAY_MS 10000

#define DIAGNOSTIC_UPDATE_DELAY_MS 500

#define STALE_MAP_POINT_SEC 300 // 300 = 5 minutes until point drops from map

// low battery percentage
#define LOW_BATTERY 5 

// led brightness
#define LED_BRIGHTNESS 12 // 0-255 (12=%5)
#define LED_BRIGHTNESS_FADE 32 // needs to be at least 32 for smoother fades

// how quick we allow the animations to be in party mode
#define LED_MIN_DELAY_MS 20

#define BUTTON_DEBOUNCE 40

// if wanting to read gps log off serial line
//#define READ_LOG_THEN_QUIT 

/*
 * Main compile/debugging options
 */

#define LOGGING

#define LOG_UPDATE_DELAY_MS 20000

// for LED MATRIX SHIELD
//#define DEBUG_LED_MATRIX 

//#define DEBUG_OLED

#define OLED_UPDATE_DELAY_MS 2000

/*
 * Program Constants (change at your own risk!)
 */

#define NUM_TALISMAN 12

enum BUTTON_STATE { MODE_PARTY, MODE_MAP, MODE_DIAGNOSTIC, MODE_TRACKER_ONLY, MODE_ZOOM_1, MODE_ZOOM_2, MODE_ZOOM_3 };

#define GPS_SERIAL_BUFFER_LEN 256 //bytes

// min time between transmissions
#define LORA_SEND_DELAY_MS 1000

#define LORA_PKT_SIZE 11

#define LOG_DUMP_BAUD 9600
#define LORA_BAUD 9600



/* 
 *  Pin definitions
 */

// internal led
#define LED_PIN D4 // this is also LED_BUILTIN

#define BUTTON_PIN D3

#define GPS_RX_PIN D4
#define GPS_TX_PIN -1 // unused

// J1 jumper on pwr board must be jumped
#define BATTERY_PIN A0 

#ifdef DEBUG_OLED
#define NEOPIXEL_PIN D8
#else
#define NEOPIXEL_PIN D1
#endif

#define NUM_NEOPIXELS 52 // 12+16+24

// these pins allow LORA device to be put into sleep
// we currently hook these to ground (device doesn't
// allow floating input, but let's go ahead and 
// reserve D0, D6 for these if we use them in the future
#define LORA_M0 D0
#define LORA_M1 D6

/*
 * Debugging setup
 */

#ifdef DEBUG_OLED
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_SSD1306.h"
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
#endif

#ifdef DEBUG_LED_MATRIX
#include <WEMOS_Matrix_LED.h>
// for optional led matrix
#define LED_MATRIX_CLK D5
#define LED_MATRIX_DIN D7
MLED mled(1); //set intensity=1
#endif

#ifdef LOGGING
File logfile;
#endif

/*
 * Location-related constants
 */

#ifdef PRODUCTION
#define MAN_LAT 40.786400
#define MAN_LON -119.206500
#define PLAYA_ELEV 1190.  // m
#define SCALE 1.
#else
#define MAN_LAT 37.819900 // golden gate bridge
#define MAN_LON -122.478300
#define PLAYA_ELEV 67.  // m
#define SCALE 15.
#endif

#define ZOOM_1_SCALE 1
#define ZOOM_2_SCALE 2
#define ZOOM_3_SCALE 3

///// PLAYA COORDINATES CODE /////

#define DEG_PER_RAD (180. / 3.1415926535)
#define CLOCK_MINUTES (12 * 60)
#define METERS_PER_DEGREE (40030230. / 360.)
// Direction of north in clock units
#define NORTH 10.5  // hours
#define NUM_RINGS 27  // Esplanade through Z 
#define ESPLANADE_RADIUS (2500 * .3048)  // m
#define FIRST_BLOCK_DEPTH (440 * .3048)  // m
#define BLOCK_DEPTH (240 * .3048)  // m
// How far in from Esplanade to show distance relative to Esplanade rather than the man
#define ESPLANADE_INNER_BUFFER (250 * .3048)  // m
// Radial size on either side of 12 w/ no city streets
#define RADIAL_GAP 2.  // hours
// How far radially from edge of city to show distance relative to city streets
#define RADIAL_BUFFER .25  // hours

// Radius of Earth in meters
#define EARTH_RADIUS 6371000

/*
 * Main talisman objects (gps and leds)
 */

// SoftwareSerial(int receivePin, int transmitPin,  bool inverse_logic = false, unsigned int buffSize = 64);
SoftwareSerial swSerGPS(GPS_RX_PIN, GPS_TX_PIN, false, GPS_SERIAL_BUFFER_LEN);

TinyGPSPlus gps;

//Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_NEOPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
NeoPatterns leds(NUM_NEOPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800, &RingComplete);

Switch button = Switch(BUTTON_PIN, INPUT_PULLUP, LOW, 40, 400, 250);

/*uint32_t colorMap[12] = {leds.Color(255,0,0), //red
                         leds.Color(0,128,0), //green
                         leds.Color(0,0,255), //blue
                         leds.Color(255,255,0), //yellow
                         leds.Color(0,255,255), //aqua
                         leds.Color(128,0,128), //purple
                         leds.Color(0,255,0), //lime
                         leds.Color(0,128,128), //teal
                         leds.Color(128,128,0), //olive
                         leds.Color(255,0,255), //fuchsia
                         leds.Color(128,0,0), //maroon
                         leds.Color(128,128,128)}; //gray*/
uint32_t colorMap[12] = {leds.Color(31,120,180), //deep blue
                         leds.Color(178,223,138), // light green
                         leds.Color(51,160,44), // green
                         leds.Color(251,154,153), // pink
                         leds.Color(227,26,28), // red
                         leds.Color(253,191,111), // tan
                         leds.Color(255,127,0), // orangeish
                         leds.Color(202,178,214), // light purple
                         leds.Color(106,61,154), // purple
                         leds.Color(255,255,153), // light tan-ish
                         leds.Color(177,89,40), // brown
                         leds.Color(166,206,227)}; // light blue

/*
 * Global variables
 * 
 */

uint8_t button_state = MODE_PARTY; 

// timers
unsigned long last_send = 0UL; // to track timing gps updates
unsigned long last_recv = 0UL; // to track timing gps updates
unsigned long last_led = 0UL; // to track timing on lights
unsigned long last_log = 0UL; // for logging interval
unsigned long last_bat_check = 0UL; // battery check interval
unsigned long last_diagnostic = 0UL;
unsigned long last_oled = 0UL;

long int t_wait_from_t0 = 9999999;
unsigned long t0 = 0UL;

uint16_t device_ctr = 0; // for updating map lights 

uint8_t batPercOthers[NUM_TALISMAN];
uint8_t numSatOthers[NUM_TALISMAN];
uint8_t hourOthers[NUM_TALISMAN]; 
uint8_t minuteOthers[NUM_TALISMAN]; 
uint16_t distOthers[NUM_TALISMAN];
double latOthers[NUM_TALISMAN];
double lonOthers[NUM_TALISMAN];
uint8_t prevPixelId[NUM_TALISMAN];
uint8_t pixelId[NUM_TALISMAN];
unsigned long updateTimes[NUM_TALISMAN];

unsigned char lora_buffer[LORA_PKT_SIZE]; // circular buffer
unsigned int pos = 0; // current position in lora buffer

uint8_t batPerc = 99; //getBatPercentage();  

double lat = 0;
double lon = 0;
uint8_t numSat = 0; // (uint8_t) gps.satellites.value();
float spd = 0.0;
uint8_t month;// = gps.date.month();
uint8_t day;// = gps.date.day();
uint8_t hour;// = gps.time.hour();
uint8_t minute;// = gps.time.minute();
uint8_t second = 255;// dummy val to start

bool prev_update_sent = true;
bool blinkState = true;

void setup() {
 
#ifdef DEBUG_OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("ID:" + String(DEVICE_ID)); 
  display.println("booting...");
  display.display();
#endif
  
  swSerGPS.begin(GPS_BAUD);
  
#ifdef LOGGING
  bool result = SPIFFS.begin();
  
  #ifdef DEBUG_OLED           
  if(result) display.println("FSys mnted");
  else display.println("FSys fail");
  display.display();
  #endif

  // uncomment for reading back log file
  #ifdef READ_LOG_THEN_QUIT
  Serial.begin(LOG_DUMP_BAUD);
  while(!Serial) {;}
  logfile = SPIFFS.open("/gps_log.txt", "r");
  #ifdef DEBUG_OLED  
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("ID:" + String(DEVICE_ID)); 
    display.println("Logf dump!");
    display.println("Press btn");
    display.println(" to begin");
    display.display();
  #endif
  //delay(5000);
  while(digitalRead(BUTTON_PIN)){delay(50);}
  #ifdef DEBUG_OLED  
    display.println("dumping...");
    display.display();
  #endif
  for(int i = 0; i <5; i++) Serial.println();

  Serial.println("START OF LOG");
  
  while(logfile.available()) {
      //Lets read line by line from the file
      String line = logfile.readStringUntil('\n');
      Serial.println(line);
    }
  logfile.close();

  Serial.println("END OF LOG");
  #ifdef DEBUG_OLED  
    display.println("End dump");
    display.println("Reset me");
    display.display();
  #endif  
  delay(1000000);
  #endif // end log read

  // WARNING: uncomment to delete log!!!
  /*if (SPIFFS.exists("/gps_log.txt")){ SPIFFS.remove("/gps_log.txt");}
  delay(1000000);*/
    
  // this opens the file in append mode
  logfile = SPIFFS.open("/gps_log.txt", "a");
  
  if (!logfile) { 
    #ifdef DEBUG_OLED  
    display.println("Logf fail!");
    display.display();
    #endif
  }
  else {
    #ifdef DEBUG_OLED  
    display.println("Logf appnd");
    display.display();
    #endif    
  }
#endif // end logging code

  // these pins must be set to 0 for LORA device to work
  // UPDATE: now just tied to ground
  //pinMode(LORA_M0, OUTPUT);
  //pinMode(LORA_M1, OUTPUT);
  //digitalWrite(LORA_M0, LOW);
  //digitalWrite(LORA_M1, LOW);

  // set initial brightness of the talisman
  // 12/255 = %5 brightness
  leds.setBrightness(LED_BRIGHTNESS);
  leds.begin();

  // set initial pixel mappings to dummy vals
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    pixelId[i] = NUM_NEOPIXELS+1;
    prevPixelId[i] = NUM_NEOPIXELS+1;
  }

  Serial.begin(LORA_BAUD);

  // flash device_id times to show we are done with setup
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < DEVICE_ID+1; j++)
      leds.setPixelColor(j+24+16, colorMap[DEVICE_ID]);
    leds.show();
    delay(200);
    leds.ColorSet(leds.Color(0,0,0));
    delay(200);
  }

  randomSeed(analogRead(0));

  //leds.TheaterChase(leds.Color(255,255,0), leds.Color(0,0,50), 100);
  //leds.Fade(leds.Color(255,0,0), leds.Color(0,255,0), 255, 20, FORWARD);
  leds.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
  leds.show();

#ifdef DEBUG_OLED
  display.println("fin setup!");
  display.display();
#endif

  delay(1000);
}

void loop() {

  //wdt_disable();
  while(swSerGPS.available() > 0) // clear the buffer
  {
    unsigned char gps_byte = swSerGPS.read();
    
    if (gps.encode(gps_byte)) // feed the gps buffer
    {
      if(gps.location.isUpdated()) 
      {
        lat = gps.location.lat();
        lon = gps.location.lng();
        numSat = (uint8_t) gps.satellites.value();
        //spd = (float)gps.speed.mph(); 
      }
      
      if(gps.date.isUpdated()) 
      {
        month = gps.date.month();
        day = gps.date.day();
      }
      if(gps.time.isUpdated()) 
      {
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();

        // using current HMS, establish future time slot for transmission
        if(prev_update_sent)
        {
          prev_update_sent = false;
          t0 = millis();

          t_wait_from_t0 = ((DEVICE_ID - (second%NUM_TALISMAN) + NUM_TALISMAN)%(NUM_TALISMAN))*1000;
        }
      }
      
    }
    yield();
  } // end gps read

  //wdt_enable(5000);

  unsigned long currTime = millis();

  if(currTime - t0 > t_wait_from_t0)
  {
    uint8_t selfPixelId = sendPkt(lat, lon, batPerc, numSat);
    pixelId[DEVICE_ID] = selfPixelId;
    
    last_send = millis();
    updateTimes[DEVICE_ID] = last_send;
    t_wait_from_t0 = 9999999UL;
    prev_update_sent = true;
  } // end lora send
  
  // something on the LoRa line
  else if (Serial.available() > 0) 
  {
    lora_buffer[pos%LORA_PKT_SIZE] = (unsigned char) Serial.read();

    if(lora_buffer[pos%LORA_PKT_SIZE] == 0xEF && lora_buffer[(pos-1)%LORA_PKT_SIZE] == 0xBE)
    {      
      if(lora_buffer[(pos+1)%LORA_PKT_SIZE] == 0xDE && lora_buffer[(pos+2)%LORA_PKT_SIZE] == 0xAD)
      {
        int devId = (int)lora_buffer[(pos+3)%LORA_PKT_SIZE];
        batPercOthers[devId] = lora_buffer[(pos+4)%LORA_PKT_SIZE];
        numSatOthers[devId] = lora_buffer[(pos+5)%LORA_PKT_SIZE];
        hourOthers[devId] = lora_buffer[(pos+6)%LORA_PKT_SIZE];
        minuteOthers[devId] = lora_buffer[(pos+7)%LORA_PKT_SIZE];
        *((unsigned char *)&distOthers + 2*devId) = lora_buffer[(pos+8)%LORA_PKT_SIZE];
        *((unsigned char *)&distOthers + 2*devId+1) = lora_buffer[(pos+9)%LORA_PKT_SIZE];

        #ifdef DEBUG_OLED
        display_oled_header();
        display.println("recv id" + String(devId)); 
        display.println(playaStrFromPkt(hourOthers[devId], minuteOthers[devId], distOthers[devId]));
        display.display();
        #endif

        last_recv = millis();

        pixelId[devId] = get_pixel_id(hourOthers[devId], minuteOthers[devId], distOthers[devId]);
        updateTimes[devId] = last_recv;
      }
    }
    pos++;

  }

  else if(button_state == MODE_MAP && currTime - last_led >= LED_MAP_UPDATE_DELAY_MS)
  {
    uint8_t currDevId = device_ctr%NUM_TALISMAN;
    uint8_t currPixelId = pixelId[currDevId];
    if(currPixelId < NUM_NEOPIXELS && currTime - updateTimes[currDevId] > STALE_MAP_POINT_SEC*1000)
    {
      // point is stale, so remove it
      leds.setPixelColor(currPixelId, 0);
      pixelId[currDevId] = NUM_NEOPIXELS+1;
      prevPixelId[currDevId] = NUM_NEOPIXELS+1;;
    }  
    uint8_t prevPixelId_tmp = prevPixelId[currDevId];
    if(prevPixelId_tmp < NUM_NEOPIXELS)
      leds.setPixelColor(prevPixelId_tmp, 0);
    if(currPixelId < NUM_NEOPIXELS)
      leds.setPixelColor(currPixelId, colorMap[currDevId]);
    prevPixelId[currDevId] = currPixelId;

    //int pixMidRing = device_ctr*20;
    //for(int i = 0; i < 16; i++)
    //{
    //  leds.setPixelColor(i+24, leds.Color(pixMidRing,pixMidRing,pixMidRing));
    //}
    leds.show();  
    device_ctr++;
    last_led = millis();
  }
  else if(button_state == MODE_DIAGNOSTIC && currTime - last_diagnostic >= DIAGNOSTIC_UPDATE_DELAY_MS)
  {
    leds.clear();
    unsigned long stopLed = 24*batPerc/100;
    for(int i = 0; i < stopLed; i++)
    {
        leds.setPixelColor(i, leds.Color(0,50,0));
    }
    
    stopLed = 16*(currTime - last_send)/(LORA_SEND_DELAY_MS*NUM_TALISMAN);
    stopLed = stopLed > 16? 16 : stopLed;
    for(int i = 0; i < stopLed; i++)
    {
      leds.setPixelColor(24+16-i-1, leds.Color(0,0,50));
    }

    stopLed = numSat < 16? numSat: 16;
    for(int i = 0; i < stopLed; i++)
    {
        //leds.setPixelColor(i+24+16, colorMap[DEVICE_ID]);
        leds.setPixelColor(24+16-i-1, leds.Color(50,0,0)); //colorMap[DEVICE_ID]);
    }

    for(int i = 0; i < NUM_TALISMAN; i++)
    {
      if(pixelId[i] < NUM_NEOPIXELS)
      {
        //fresh data
        leds.setPixelColor(i+24+16, colorMap[i]);
      }
      if(i == DEVICE_ID && blinkState == false)
      {
        leds.setPixelColor(i+24+16, 0);
        blinkState = true;
      }
      else if(i == DEVICE_ID && blinkState == true)
      {
        blinkState = false;
      }
      
    }
    
    leds.show();
    last_diagnostic = millis();
  }

  else if(currTime - last_bat_check > BATTERY_CHECK_DELAY_MS)
  {
    batPerc = getBatPercentage();

    if(batPerc < LOW_BATTERY)
    {
      red_ring(); // low battery mode
    }
    last_bat_check = millis();
  }

#ifdef LOGGING
  else if(currTime - last_log >= LOG_UPDATE_DELAY_MS)
  {
    logfile.print(month); logfile.print(" "); // Hour (0-23) (u8)
    logfile.print(day); logfile.print(" "); // Hour (0-23) (u8)
    logfile.print(hour); logfile.print(" "); // Hour (0-23) (u8)
    logfile.print(minute); logfile.print(" ");// Minute (0-59) (u8)
    logfile.print(second); logfile.print(" "); // Second (0-59) (u8)
    logfile.print(DEVICE_ID); logfile.print(" "); 
    logfile.print(lat, 6); logfile.print(" ");
    logfile.print(lon, 6); logfile.print(" ");
    logfile.print(numSat); logfile.print(" ");
    //logfile.print(spd); logfile.print(" ");
    logfile.print(batPerc); logfile.print(" ");
    logfile.println(millis()); //f.print(" ");
    
    last_log = millis();
  }
#endif

#ifdef DEBUG_OLED
  else if(currTime - last_oled >= OLED_UPDATE_DELAY_MS)
  {
    display_oled_header();
    display.print(fmtPlayaStr(lat, lon));
    display.display();

    last_oled = millis();
  }
#endif

  else if(button_state == MODE_PARTY) // &&  currTime - last_led >= LED_PARTY_UPDATE_DELAY_MS)
  {
    leds.Update(); 
  }
  else if (button_state == MODE_ZOOM_1 && currTime - last_led >= LED_MAP_UPDATE_DELAY_MS)
  {
    // ZOOM_1 each ring is X meters for a total of 3*X meters max distance
    //set yourself at center, there is no center pixel apparently, soooooo we turn you off
    //leds.setPixelColor(currPixelId[DEVICE_ID], 0);

    //find position of man relative to you (reverse the normal MAP_MODE coordinates)

    //for each other find position relative to you

    /*uint8_t currDevId = device_ctr%NUM_TALISMAN;
    uint8_t currPixelId = pixelId[currDevId];
    if(currPixelId < NUM_NEOPIXELS && currTime - updateTimes[currDevId] > STALE_MAP_POINT_SEC*1000)
    {
      // point is stale, so remove it
      leds.setPixelColor(currPixelId, 0);
      pixelId[currDevId] = NUM_NEOPIXELS+1;
      prevPixelId[currDevId] = NUM_NEOPIXELS+1;;
    }  
    uint8_t prevPixelId_tmp = prevPixelId[currDevId];
    if(prevPixelId_tmp < NUM_NEOPIXELS)
      leds.setPixelColor(prevPixelId_tmp, 0);
    if(currPixelId < NUM_NEOPIXELS)
      leds.setPixelColor(currPixelId, colorMap[currDevId]);
    prevPixelId[currDevId] = currPixelId;

    
    leds.show();  
    device_ctr++;
    last_led = millis();*/

  }
  else if (button_state == MODE_ZOOM_2 && currTime - last_led >= LED_MAP_UPDATE_DELAY_MS)
  {

  }
  else if (button_state == MODE_ZOOM_3 && currTime - last_led >= LED_MAP_UPDATE_DELAY_MS)
  {

  }

  button.poll();

  if(button.pushed())
  {
    switch(button_state)
    {
      case MODE_PARTY:
          button_state = MODE_MAP;
          leds.ActivePattern = NONE;

          //leds.ActivePattern = FADE_INNER_RING;
          //leds.Interval = 100;
          //leds.TotalSteps = LED_BRIGHTNESS_FADE;
          //leds.setBrightness(LED_BRIGHTNESS_FADE);
          //leds.Color1 = leds.Color(0,0,0);
          //leds.Color2 = leds.Color(LED_BRIGHTNESS_FADE,LED_BRIGHTNESS_FADE,LED_BRIGHTNESS_FADE);
          //leds.StartIndex = 24;
          //leds.EndIndex = 24+16;
          leds.clear();
          leds.show(); 
          break;
      case MODE_MAP:
          button_state = MODE_ZOOM_1;
          leds.ActivePattern = NONE;
          leds.clear();
          leds.show(); 
          break;
      case MODE_ZOOM_1:
          button_state = MODE_ZOOM_2;
          leds.ActivePattern = NONE;
          leds.clear();
          leds.show();
          break;
      case MODE_ZOOM_2:
          button_state = MODE_ZOOM_3;
          leds.ActivePattern = NONE;
          leds.clear();
          leds.show();
          break;
      case MODE_ZOOM_3:
          button_state = MODE_DIAGNOSTIC;
          leds.ActivePattern = NONE;
          leds.clear();
          leds.show();
          break;
      case MODE_DIAGNOSTIC:
          button_state = MODE_TRACKER_ONLY;
          leds.ActivePattern = NONE;
          leds.clear();
          leds.show(); 
          break;
      case MODE_TRACKER_ONLY:
          button_state = MODE_PARTY;
          RingComplete();
          break;
      default:
          break;
    } //end switch    

  }
}

void RingComplete()
{
    
    if(leds.ActivePattern != FADE_INNER_RING)
    {
      
      pattern randPattern = (pattern)(random(5)+1); // starts with NONE 
      direction dir = (direction)random(2);
      uint32_t color2 = leds.Color1;
      uint32_t color1 = leds.Wheel(random(255));
      
      if(randPattern == RAINBOW_CYCLE) 
      {
        leds.RainbowCycle(LED_MIN_DELAY_MS, dir);
        leds.setBrightness(LED_BRIGHTNESS_FADE);
      }
      else if(randPattern == FADE)
      {
        leds.Fade(color2, color1, LED_BRIGHTNESS_FADE*2, LED_MIN_DELAY_MS, dir);
        leds.setBrightness(LED_BRIGHTNESS_FADE);
      }
      else if(randPattern == THEATER_CHASE)
      {
        leds.TheaterChase(color1, color2, random(80) + LED_MIN_DELAY_MS, dir);
        leds.setBrightness(LED_BRIGHTNESS);
      }
      else if(randPattern == COLOR_WIPE)
      {
        leds.ColorWipe(color1, random(20) + LED_MIN_DELAY_MS, dir);
        leds.setBrightness(LED_BRIGHTNESS);
      }
      else if(randPattern == SCANNER)
      {
        leds.Scanner(colorMap[DEVICE_ID], random(40) + LED_MIN_DELAY_MS);
        leds.setBrightness(LED_BRIGHTNESS_FADE);
      }    
      else
      {
        leds.ColorSet(colorMap[DEVICE_ID]);
        leds.setBrightness(LED_BRIGHTNESS);
      }
    }
}

uint8_t getBatPercentage()
{
  float batVolt = 5.0*((float)analogRead(A0))/1023.0; 
  uint8_t batPerc = (uint8_t)(100.0*(batVolt - 3.7)/0.6); // 4.3 = 100%, 3.7 = 0%
  if(batPerc > 100) batPerc = 100;
  if(batPerc < 0) batPerc = 0;
  return batPerc;
}

// low battery mode
// flash low brightness red until battery percentage goes back up
// flash twice quickly and then wait 2 seconds
void red_ring()
{
  #ifdef DEBUG_OLED
  display.clearDisplay();
  display.display();
  #endif
  swSerGPS.enableRx(false);
  leds.setBrightness(1);
  uint8_t batPerc = LOW_BATTERY;
  while(batPerc <= LOW_BATTERY)
  {
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.setPixelColor(i, leds.Color(255,0,0));
    leds.show();
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.setPixelColor(i, leds.Color(0,0,0));
    leds.show();
    //digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.setPixelColor(i, leds.Color(255,0,0));
    leds.show();
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.setPixelColor(i, leds.Color(0,0,0));
    leds.show();
    //digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
    batPerc = getBatPercentage();    
  }
  swSerGPS.enableRx(true);
}

String fmtPlayaStr(double lat, double lon) {
  if (lat == 0 && lon == 0) {
    return "I'm lost";
  } else {
    return playaStr(lat, lon);
  }
}

// 0=man, 1=espl, 2=A, 3=B, ...
float ringRadius(int n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return ESPLANADE_RADIUS;
  } else if (n == 2) {
    return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH;
  } else {
    return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH + (n - 2) * BLOCK_DEPTH;
  }
}

// Distance inward from ring 'n' to show distance relative to n vs. n-1
float ringInnerBuffer(int n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return ESPLANADE_INNER_BUFFER;
  } else if (n == 2) {
    return .5 * FIRST_BLOCK_DEPTH;
  } else {
    return .5 * BLOCK_DEPTH;
  }
}

int getReferenceRing(float dist) {
  for (int n = NUM_RINGS; n > 0; n--) {
    //Serial.println(n + ":" + String(ringRadius(n)) + " " + String(ringInnerBuffer(n)));
    if (ringRadius(n) - ringInnerBuffer(n) <= dist) {
      return n;
    }
  }
  return 0;
}

String getRefDisp(int n) {
  if (n == 0) {
    return ")(";
  } else if (n == 1) {
    return "Espl";
  } else {
    return String(char(int('A') + n - 2));
  }
}


String playaStr(double lat, double lon) {

  bool accurate = true; // later add functionality for looking at gps accuracy
  
  float dlat = (float)lat - MAN_LAT;
  float dlon = (float)lon - MAN_LON;

  float m_dx = dlon * METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
  float m_dy = dlat * METERS_PER_DEGREE;

  float dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy);
  float bearing = DEG_PER_RAD * atan2(m_dx, m_dy);

  float clock_hours = (bearing / 360. * 12. + NORTH);
  int clock_minutes = (int)(clock_hours * 60 + .5);
  // Force into the range [0, CLOCK_MINUTES)
  clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES;

  uint8_t hour = clock_minutes / 60;
  uint8_t minute = clock_minutes % 60;
  
  String clock_disp = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);

  int refRing;
  if (6 - abs(clock_minutes/60. - 6) < RADIAL_GAP - RADIAL_BUFFER) {
    refRing = 0;
  } else {
    refRing = getReferenceRing(dist);
  }
  float refDelta = dist - ringRadius(refRing);
  long refDeltaRounded = (long)(refDelta + .5);

  return clock_disp + " & " + getRefDisp(refRing) + (refDeltaRounded >= 0 ? "+" : "-") + String(refDeltaRounded < 0 ? -refDeltaRounded : refDeltaRounded) + "m" + (accurate ? "" : "-ish");
}

String playaStrFromPkt(uint8_t hour, uint8_t minute, uint16_t dist)
{
  String clock_disp = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
  int refRing = getReferenceRing((float)dist);
  float refDelta = (float)dist - ringRadius(refRing);
  long refDeltaRounded = (long)(refDelta + .5);
  return clock_disp + " & " + getRefDisp(refRing) + (refDeltaRounded >= 0 ? "+" : "-") + String(refDeltaRounded < 0 ? -refDeltaRounded : refDeltaRounded) + "m"; 
}

uint8_t sendPkt(double lat, double lon, uint8_t batPerc, uint8_t numSat)
{
  if(lat == 0)
  {
   // 11 byte message
    Serial.write((byte)0xDE); //DEAD is 2-byte start of message
    Serial.write((byte)0xAD);
    Serial.write((byte)DEVICE_ID);
    Serial.write((byte)batPerc); // uint8_t
    Serial.write((byte)numSat); // uin8_t
    Serial.write((byte)255);
    Serial.write((byte)255);
    Serial.write((byte)255);
    Serial.write((byte)255);
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double
    Serial.write((byte)0xBE); //BEEF is 2-byte end of message
    Serial.write((byte)0xEF);      
    return 255;
  }
  else
  {
    float dlat = (float)lat - MAN_LAT;
    float dlon = (float)lon - MAN_LON;
  
    //why do we only use cos and not sin as well? Strange.
    float m_dx = dlon * METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
    float m_dy = dlat * METERS_PER_DEGREE;
  
    float dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy);
    float bearing = DEG_PER_RAD * atan2(m_dx, m_dy);
  
    float clock_hours = (bearing / 360. * 12. + NORTH);
    uint16_t clock_minutes = (uint16_t)(clock_hours * 60 + .5);
    // Force into the range [0, CLOCK_MINUTES), also why + and % again?
    clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES;
  
    uint8_t hour = clock_minutes / 60;
    uint8_t minute = clock_minutes % 60;
  
    if(dist > 65534) dist = 65535;
    
    uint16_t distRounded = (uint16_t)(dist + .5); // meters
  
    // 11 byte message
    Serial.write((byte)0xDE); //DEAD is 2-byte start of message
    Serial.write((byte)0xAD);
    Serial.write((byte)DEVICE_ID);
    Serial.write((byte)batPerc); // uint8_t
    Serial.write((byte)numSat); // uin8_t
    Serial.write((byte)hour);
    Serial.write((byte)minute);
    Serial.write((byte *) &distRounded, 2);
    //I need this information since having the hour and minute relative to the man is probably too low
    //res to be useful for the zoom mode
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double
    Serial.write((byte)0xBE); //BEEF is 2-byte end of message
    Serial.write((byte)0xEF);  

    return get_pixel_id(hour, minute, dist);
  }
}

void display_oled_header()
{
  #ifdef DEBUG_OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(String(DEVICE_ID) + "v" + String(VERSION,1) + " " + String(millis()/60000) + "m"); 
  //display.println(lat, 6);
  //display.println(lon, 6);
  //display.println(String(spd) + "mph");
  //display.println(String(hour) + ":" + String(minute));
  //float batVolt = 5.0*((float)analogRead(A0))/1023.0; 
  //display.println(String(batVolt)); 
  display.println("#s=" + String(numSat) + " %" + String(batPerc)); 
  #endif
}

uint8_t get_pixel_id(uint8_t hr, uint8_t minute, uint16_t dist)
{
  if(hr == 255 && minute == 255)
    return 255;
  
  int refRing = getReferenceRing((float)dist);
  if(refRing < 2) // man to esplanade
  {
    //return 39-(((int)hr%12)*60 + minute)/45; // middle ring (39 to 24)
    return 40+((((int)hr%12)*60 + minute+30)/60+1)%12; // center ring (40 to 51)
  }
  else
  {
    return (((((int)hr%12)*60 + minute+15)/30)+1)%24; // outer ring (0-23)
  }
}

uint8_t get_scaled_pixel_id(double olat, double olon, uint16_t scale)
{
  float dlat = olat - (float) lat;
  float dlon = olon - (float) lon;

  float mdx  = dlon * METERS_PER_DEGREE * cos((float)lat/DEG_PER_RAD);
  float mdy = dlat * METERS_PER_DEGREE;

  float dist = sqrt(mdx*mdx + mdy*mdy);
  float bearing = DEG_PER_RAD * atan2(mdx, mdy);
}

double haversine_dist(double lat1, double lon1, double lat2, double lon2)
{
  double phi1 = lat1 / DEG_PER_RAD;
  double phi2 = lat2 / DEG_PER_RAD;
  double deltaPhi = (lat2 - lat1) / DEG_PER_RAD;
  double deltaLambda = (lon2 - lon1) / DEG_PER_RAD;

  double a = sin(deltaPhi/2) * sin(deltaPhi) + cos(phi1) * cos(phi2) * sin(deltaLambda/2) * sin(deltaLambda/2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));

  return EARTH_RADIUS * c;
}

double haversine_bearing(double lat1, double lon1, double lat2, double lon2) 
{
  double phi1 = lat1 / DEG_PER_RAD;
  double phi2 = lat2 / DEG_PER_RAD;
  double deltaPhi = (lat2 - lat1) / DEG_PER_RAD;
  double deltaLambda = (lon2 - lon1) / DEG_PER_RAD;

  double y = sin(deltaLambda) * cos(phi2);
  double x = cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(deltaLambda);

  return ((atan2(y, x) * DEG_PER_RAD) + 360) % 360
}

