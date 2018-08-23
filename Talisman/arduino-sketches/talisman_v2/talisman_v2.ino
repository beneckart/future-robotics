
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "TalismanPatterns.cpp"
#include <TinyGPS++.h>
#include "FS.h"
#include "SPIFFS.h"
#include "Switch.h"
#include "SSD1306.h" 

#define VERSION 2.1

//UNCOMMENT OUT IF AT BM!!
#define PRODUCTION

#define DEVICE_ID 0 //later, replace this with chipid

/*
 * User device variables
 */
 
// delays
#define LED_MAP_UPDATE_DELAY_MS 250

#define BATTERY_CHECK_DELAY_MS 60*1000*5

#define DIAGNOSTIC_UPDATE_DELAY_MS 500

#define STALE_MAP_POINT_SEC 300 // 300 = 5 minutes until point drops from map

// low battery percentage
#define LOW_BATTERY -1

// led brightness
#define LED_BRIGHTNESS 4 // 0-255 (12=%5)
#define LED_BRIGHTNESS_FADE 32 // needs to be at least 32 for smoother fades

// how quick we allow the animations to be in party mode
#define LED_MIN_DELAY_MS 15

#define BUTTON_DEBOUNCE 40

// if wanting to read gps log off serial line
//#define READ_LOG_THEN_QUIT 

/*
 * Main compile/debugging options
 */

#define LOGGING

#define LOG_UPDATE_DELAY_MS 1000

#define DEBUG_OLED

#define OLED_UPDATE_DELAY_MS 2000

/*
 * Program Constants (change at your own risk!)
 */

#define NUM_TALISMAN 6

#define RECEIVE_ONLY 0

#define NUM_NEOPIXELS 120

enum BUTTON_STATE { MODE_PARTY, MODE_PARTY2, MODE_MAP, MODE_DIAGNOSTIC, MODE_TRACKER_ONLY };

// min time between transmissions
#define LORA_SEND_DELAY_MS 500

#define LORA_PKT_SIZE 11

#define LOG_DUMP_BAUD 115200 // too fast or just right?

// LoRa stuff
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET (!!!!!)
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    868E6 //
#define SPREADING_FACTOR 9

// 1-20, default in library is 17 db
// latest github version supports 20db (test this?) but max before that was 17db
#define TX_PWR 17 

//7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, and 250E3
//125E3 is library default, check and make sure 500e3 works (latest github version has this I think)
#define LORA_BW 250E3 
#define LORA_CODING 5 //between 5..8

/* 
 *  Pin definitions
 */

// neopixel pins
#define LED_PIN1 14 // this is also LED_BUILTIN (?)
#define LED_PIN2 2

#define BUTTON_PIN 39

#define BATTERY_PIN 35 

/*
 * Debugging setup
 */

#ifdef DEBUG_OLED
SSD1306 display(0x3c, 21, 22);
#endif

#ifdef LOGGING
File logfile;
#define FORMAT_SPIFFS_IF_FAILED true
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
//#define MAN_LAT 40.42425
//#define MAN_LON -79.982274
//#define PLAYA_ELEV 272.  // m
//#define SCALE 15.
#define MAN_LAT 37.75687
#define MAN_LON -122.4149
#define PLAYA_ELEV -17.
#define SCALE 15.
//Time      : 7:0:57 when really 12:03am

#endif

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

/*
 * Main talisman objects (gps and leds)
 */
uint64_t chipid;  
 
TinyGPSPlus gps;

Switch button = Switch(BUTTON_PIN, INPUT_PULLUP, LOW, 40, 400, 250);

TalismanPatterns<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod> leds(NUM_NEOPIXELS, LED_PIN1, RingComplete);
TalismanPatterns<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> leds2(NUM_NEOPIXELS, LED_PIN2, RingComplete);

/*uint32_t colorMap[12] = {RgbColor(255,0,0), //red
                         RgbColor(0,128,0), //green
                         RgbColor(0,0,255), //blue
                         RgbColor(255,255,0), //yellow
                         RgbColor(0,255,255), //aqua
                         RgbColor(128,0,128), //purple
                         RgbColor(0,255,0), //lime
                         RgbColor(0,128,128), //teal
                         RgbColor(128,128,0), //olive
                         RgbColor(255,0,255), //fuchsia
                         RgbColor(128,0,0), //maroon
                         RgbColor(128,128,128)}; //gray*/
RgbColor colorMap[12] = {RgbColor(31,120,180), //deep blue
                         RgbColor(178,223,138), // light green
                         RgbColor(51,160,44), // green
                         RgbColor(251,154,153), // pink
                         RgbColor(227,26,28), // red
                         RgbColor(253,191,111), // tan
                         RgbColor(255,127,0), // orangeish
                         RgbColor(202,178,214), // light purple
                         RgbColor(106,61,154), // purple
                         RgbColor(255,255,153), // light tan-ish
                         RgbColor(177,89,40), // brown
                         RgbColor(166,206,227)}; // light blue

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
unsigned long gps_update_time = 0UL;
long int t_wait_from_t0 = 9999999;
unsigned long t0 = 0UL;

uint16_t device_ctr = 0; // for updating map lights 

uint8_t batPercOthers[NUM_TALISMAN];
uint8_t numSatOthers[NUM_TALISMAN];
uint8_t hourOthers[NUM_TALISMAN]; 
uint8_t minuteOthers[NUM_TALISMAN]; 
uint16_t distOthers[NUM_TALISMAN];
uint8_t prevPixelId[NUM_TALISMAN];
uint8_t pixelId[NUM_TALISMAN];
unsigned long updateTimes[NUM_TALISMAN];

unsigned char lora_buffer[LORA_PKT_SIZE]; // circular buffer
unsigned int pos = 0; // current position in lora buffer

uint8_t batPerc = 99; //getBatPercentage();  

double lat = 0;
double lon = 0;
double prevLat = 0;
double prevLon = 0;
uint8_t numSat = 0; // (uint8_t) gps.satellites.value();
float spd = 0.0;
uint8_t month;// = gps.date.month();
uint8_t day;// = gps.date.day();
uint8_t hour;// = gps.time.hour();
uint8_t minute;// = gps.time.minute();
uint8_t second = 255;// dummy val to start
uint8_t centisecond = 255;
uint32_t millisecond = 1000;

double displacement = 0.0;

bool prev_update_sent = true;
bool blinkState = true;

void setup() {
 
#ifdef DEBUG_OLED
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);


  // Clear the buffer.
  display.clear();
  //display.setTextSize(1);
  //display.setTextColor(WHITE);
  //display.setCursor(0,0);
  display.drawString(0,0,"ID:" + String(DEVICE_ID)); 
  display.drawString(0,15,"booting...");
  display.display();
#endif
  Serial.begin(115200);//, SERIAL_8N1); 
  chipid = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

  // GPS, hardcoded for T-Beam
  Serial1.begin(9600, SERIAL_8N1, 12, 15); 
  
  
#ifdef LOGGING
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
  }

  // later integrate above with below
  /*#ifdef DEBUG_OLED           
  if(result) display.println("FSys mnted");
  else display.println("FSys fail");
  display.display();
  #endif*/

  // uncomment for reading back log file
  #ifdef READ_LOG_THEN_QUIT
  //Serial.begin(LOG_DUMP_BAUD);
  //while(!Serial) {;}
  logfile = SPIFFS.open("/gps_log.txt", "r");
  #ifdef DEBUG_OLED  
    display.clear(); //TODO
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
  logfile.println("log start");
  
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

  // set initial pixel mappings to dummy vals
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    pixelId[i] = NUM_NEOPIXELS+1;
    prevPixelId[i] = NUM_NEOPIXELS+1;
  }

  //initialize LoRa
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
  LoRa.receive();
  
  LoRa.setSpreadingFactor(SPREADING_FACTOR);   

  //LoRa.setSyncWord(0xBE); //ignore everything but BE, defaults to 0x12
  
  //LoRa.setTxPower(TX_PWR); //affects battery life how?

  LoRa.setSignalBandwidth(LORA_BW);
  
  LoRa.setCodingRate4(LORA_CODING); //between 5..8

  //preamble length in symbols, defaults to 8. Supported values are between 6 and 65535
  //LoRa.setPreambleLength(preambleLength);

  //LoRa.enableCrc();
  //LoRa.disableCrc(); //default

  //LoRa.receive();

  //byte b = LoRa.random(); //could use for determining which talisman to send
  
  Serial.println("LoRa init ok");

  leds.Begin();
  leds2.Begin();

  leds.SetBrightness(LED_BRIGHTNESS);
  leds2.SetBrightness(LED_BRIGHTNESS);
  
  leds.Show();
  leds2.Show();
  
  // flash device_id times to show we are done with setup
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < DEVICE_ID+1; j++)
      leds.SetPixelColor(j+24+16, colorMap[DEVICE_ID]);
    leds.Show();
    delay(200);
    leds.ClearTo(RgbColor(0,0,0));
    leds.Show();
    delay(200);
  }
  leds.Show();
  randomSeed(analogRead(0));

  ////leds.TheaterChase(RgbColor(255,255,0), RgbColor(0,0,50), 100);
  ////leds.Fade(RgbColor(255,0,0), RgbColor(0,255,0), 255, 20, FORWARD);
  leds.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
  leds2.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);

#ifdef DEBUG_OLED
  display.println("fin setup!");
  display.display();
#endif

  Serial.println("fin setup!");

  delay(500);
}

void loop() {

  //wdt_disable();
  if(Serial1.available() > 0) // clear the buffer
  {
    unsigned char gps_byte = Serial1.read();
    
    if (gps.encode(gps_byte)) // feed the gps buffer
    {
      if(gps.location.isUpdated()) 
      {
        lat = gps.location.lat();
        lon = gps.location.lng();
        numSat = (uint8_t) gps.satellites.value();
        spd = (float)gps.speed.mph(); 
        //Serial.println("gps updated -- " + String(hour) + " " + String(minute) + " " + String(second));
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
        centisecond = gps.time.centisecond();
        millisecond = second*1000 + centisecond*10;
        gps_update_time = millis();
      }
      
    }
    yield();
  } // end gps read

  //wdt_enable(5000);

  unsigned long currTime = millis();
  if(!RECEIVE_ONLY && ((millisecond+currTime-gps_update_time)/LORA_SEND_DELAY_MS)%NUM_TALISMAN == DEVICE_ID && currTime - last_send >= LORA_SEND_DELAY_MS*(NUM_TALISMAN-1))
  {
    Serial.println("LoRa sent " + String((millisecond+currTime-gps_update_time)/LORA_SEND_DELAY_MS) + " " + String(currTime - last_send));
    uint8_t selfPixelId = sendPkt(lat, lon, batPerc, numSat);
    
    pixelId[DEVICE_ID] = selfPixelId;
    
    last_send = currTime;
    updateTimes[DEVICE_ID] = last_send;

  } // end lora send
  
  // something on the LoRa line
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    // TODO: is this a better construction?
    /*while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("Snr: " + String(LoRa.packetSnr()));
    Serial.println();    
    */
    for(int ii = 0; ii < packetSize; ii++)
    {
    lora_buffer[pos%LORA_PKT_SIZE] = (char) LoRa.read();
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

        Serial.print("id=" + String(devId) + ": ");
        Serial.println(playaStrFromPkt(hourOthers[devId], minuteOthers[devId], distOthers[devId]));

        #ifdef DEBUG_OLED
        display_oled_header();
        display.drawString(0,14,"id=" + String(devId) + ": "); 
        display.drawString(0,26,playaStrFromPkt(hourOthers[devId], minuteOthers[devId], distOthers[devId]));
        display.display();
        #endif

        last_recv = millis();

        pixelId[devId] = get_pixel_id(hourOthers[devId], minuteOthers[devId], distOthers[devId]);
        updateTimes[devId] = last_recv;
      }
    }
    pos++;
    }
  }

  if(button_state == MODE_MAP && currTime - last_led >= LED_MAP_UPDATE_DELAY_MS)
  {
    //Serial.println("map update start");
    uint8_t currDevId = device_ctr%NUM_TALISMAN;
    uint8_t currPixelId = pixelId[currDevId];
    if(currPixelId < NUM_NEOPIXELS && currTime - updateTimes[currDevId] > STALE_MAP_POINT_SEC*1000)
    {
      // point is stale, so remove it
      leds.SetPixelColor(currPixelId, 0);
      pixelId[currDevId] = NUM_NEOPIXELS+1;
      prevPixelId[currDevId] = NUM_NEOPIXELS+1;;
    }  
    uint8_t prevPixelId_tmp = prevPixelId[currDevId];
    if(prevPixelId_tmp < NUM_NEOPIXELS)
      leds.SetPixelColor(prevPixelId_tmp, 0);
    if(currPixelId < NUM_NEOPIXELS)
      leds.SetPixelColor(currPixelId, colorMap[currDevId]);
    prevPixelId[currDevId] = currPixelId;

    //int pixMidRing = device_ctr*20;
    //for(int i = 0; i < 16; i++)
    //{
    //  leds.SetPixelColor(i+24, RgbColor(pixMidRing,pixMidRing,pixMidRing));
    //}
    leds.Show();  
    device_ctr++;
    last_led = millis();
    //Serial.println("map update");
  }
  else if(button_state == MODE_DIAGNOSTIC && currTime - last_diagnostic >= DIAGNOSTIC_UPDATE_DELAY_MS)
  {
    Serial.println("diagnostic update start");
    leds.ClearTo(RgbColor(0));
    unsigned long stopLed = 24*batPerc/100;
    for(int i = 0; i < stopLed; i++)
    {
        leds.SetPixelColor(i, RgbColor(0,50,0));
    }
    
    stopLed = 16*(currTime - last_send)/(LORA_SEND_DELAY_MS*NUM_TALISMAN);
    stopLed = stopLed > 16? 16 : stopLed;
    for(int i = 0; i < stopLed; i++)
    {
      leds.SetPixelColor(24+16-i-1, RgbColor(0,0,50));
    }

    stopLed = numSat < 16? numSat: 16;
    for(int i = 0; i < stopLed; i++)
    {
        //leds.SetPixelColor(i+24+16, colorMap[DEVICE_ID]);
        leds.SetPixelColor(24+16-i-1, RgbColor(50,0,0)); //colorMap[DEVICE_ID]);
    }

    for(int i = 0; i < NUM_TALISMAN; i++)
    {
      if(pixelId[i] < NUM_NEOPIXELS)
      {
        //fresh data
        leds.SetPixelColor(i+24+16, colorMap[i]);
      }
      if(i == DEVICE_ID && blinkState == false)
      {
        leds.SetPixelColor(i+24+16, RgbColor(0));
        blinkState = true;
      }
      else if(i == DEVICE_ID && blinkState == true)
      {
        blinkState = false;
      }
      
    }
    
    leds.Show();
    last_diagnostic = millis();
    Serial.println("diagnostic update");
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
  if(currTime - last_log >= LOG_UPDATE_DELAY_MS)
  {
    displacement = METERS_PER_DEGREE*sqrt((lat-prevLat)*(lat-prevLat) + (lon-prevLon)*(lon-prevLon));
    if(displacement > 4 || spd > 1.0)
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
      logfile.print(spd); logfile.print(" ");
      logfile.print(batPerc); logfile.print(" ");
      logfile.println(millis()); //f.print(" ");
      // takes either 0.5 to 3.0 ms to log
      last_log = millis();
      prevLon = lon;
      prevLat = lat;
    }
    else
    {
      // don't check again for delay time
      last_log = millis(); 
    }
    
  }
#endif

#ifdef DEBUG_OLED
  else if(currTime - last_oled >= OLED_UPDATE_DELAY_MS)
  {
    
    //display_oled_header();
    //display.drawString(0,26, fmtPlayaStr(lat, lon));
    //display.display();

    last_oled = millis();
  }
#endif

  else if(button_state == MODE_PARTY || button_state == MODE_PARTY2) // &&  currTime - last_led >= LED_PARTY_UPDATE_DELAY_MS)
  {
    leds.Update(); 
    leds.Show();
    leds2.Update();
    leds2.Show();
    //Serial.println("party  update");
    
  }

  button.poll();

  if(button.pushed())
  {
    Serial.println("button pushed!");
    switch(button_state)
    {
      case MODE_PARTY:
          button_state = MODE_PARTY2;
          leds.ActivePattern = RAINBOW_CYCLE;
          leds.RainbowCycle(30);
          leds.SetBrightness(50);
          leds2.RainbowCycle(30);
          leds2.SetBrightness(50);
          break;
      case MODE_PARTY2:
          button_state = MODE_MAP;
          leds.ActivePattern = NONE;
          leds2.ActivePattern = NONE;

          ////leds.ActivePattern = FADE_INNER_RING;
          ////leds.Interval = 100;
          ////leds.TotalSteps = LED_BRIGHTNESS_FADE;
          ////leds.SetBrightness(LED_BRIGHTNESS_FADE);
          ////color1 = RgbColor(0,0,0);
          ////color2 = RgbColor(LED_BRIGHTNESS_FADE,LED_BRIGHTNESS_FADE,LED_BRIGHTNESS_FADE);
          ////leds.StartIndex = 24;
          ////leds.EndIndex = 24+16;
          leds.ClearTo(RgbColor(0));
          leds.Show(); 
          leds2.ClearTo(RgbColor(0));
          leds2.Show(); 
          Serial.println("map mode");
          break;
      case MODE_MAP:
          button_state = MODE_DIAGNOSTIC;
          leds.ActivePattern = NONE;
          leds.ClearTo(RgbColor(0));
          leds.Show(); 
          leds2.ActivePattern = NONE;
          leds2.ClearTo(RgbColor(0));
          leds2.Show(); 
          Serial.println("diagnostic mode");
          break;
      case MODE_DIAGNOSTIC:
          button_state = MODE_TRACKER_ONLY;
          leds.ActivePattern = NONE;
          leds.ClearTo(RgbColor(0));
          leds.Show(); 
          leds2.ActivePattern = NONE;
          leds2.ClearTo(RgbColor(0));
          leds2.Show(); 
          Serial.println("tracker mode");
          break;
      case MODE_TRACKER_ONLY:
          button_state = MODE_PARTY;
          RingComplete();
          Serial.println("party mode");
          break;
      default:
          break;
    } //end switch    

  }
}

void RingComplete()
{
    if(button_state == MODE_PARTY)//leds.ActivePattern != FADE_INNER_RING)
    {
      pattern randPattern = (pattern)(random(5)+1); // starts with NONE 
      direction dir = (direction)random(2);
      
      RgbColor color1 = leds.Wheel(random(255));
      RgbColor color2 = colorMap[DEVICE_ID];
       
      if(randPattern == RAINBOW_CYCLE) 
      {
        leds.RainbowCycle(LED_MIN_DELAY_MS, dir);
        leds.SetBrightness(LED_BRIGHTNESS_FADE);
        leds2.RainbowCycle(LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS_FADE);
      }
      else if(randPattern == FADE)
      {
        leds.Fade(color2, color1, LED_BRIGHTNESS_FADE*2, LED_MIN_DELAY_MS, dir);
        leds.SetBrightness(LED_BRIGHTNESS_FADE);
        leds2.Fade(color2, color1, LED_BRIGHTNESS_FADE*2, LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS_FADE);
      }
      else if(randPattern == THEATER_CHASE)
      {
        leds.TheaterChase(color1, color2, random(80) + LED_MIN_DELAY_MS, dir);
        leds.SetBrightness(LED_BRIGHTNESS);
        leds2.TheaterChase(color1, color2, random(80) + LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS);
      }
      else if(randPattern == COLOR_WIPE)
      {
        leds.ColorWipe(color1, random(20) + LED_MIN_DELAY_MS, dir);
        leds.SetBrightness(LED_BRIGHTNESS);
        leds2.ColorWipe(color1, random(20) + LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS);
      }
      else if(randPattern == SCANNER)
      {
        leds.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
        leds.SetBrightness(LED_BRIGHTNESS_FADE);
        leds2.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
        leds2.SetBrightness(LED_BRIGHTNESS_FADE);
      }    
      else
      {
        leds.ColorSet(colorMap[DEVICE_ID]);
        leds.SetBrightness(LED_BRIGHTNESS);
        leds2.ColorSet(colorMap[DEVICE_ID]);
        leds2.SetBrightness(LED_BRIGHTNESS);
      }
    }
    if(button_state == MODE_PARTY)
    {
      pattern randPattern = (pattern)(random(5)+1); // starts with NONE 
      direction dir = (direction)random(2);
      
      RgbColor color1 = leds.Wheel(random(255));
      RgbColor color2 = colorMap[DEVICE_ID];
       
      if(randPattern == RAINBOW_CYCLE) 
      {
        leds2.RainbowCycle(LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS_FADE);
      }
      else if(randPattern == FADE)
      {
        leds2.Fade(color2, color1, LED_BRIGHTNESS_FADE*2, LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS_FADE);
      }
      else if(randPattern == THEATER_CHASE)
      {
        leds2.TheaterChase(color1, color2, random(80) + LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS);
      }
      else if(randPattern == COLOR_WIPE)
      {
        leds2.ColorWipe(color1, random(20) + LED_MIN_DELAY_MS, dir);
        leds2.SetBrightness(LED_BRIGHTNESS);
      }
      else if(randPattern == SCANNER)
      {
        leds2.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
        leds2.SetBrightness(LED_BRIGHTNESS_FADE);
      }    
      else
      {
        leds2.ColorSet(colorMap[DEVICE_ID]);
        leds2.SetBrightness(LED_BRIGHTNESS);
      }

      
    }
}

uint8_t getBatPercentage()
{
  float batVolt = 10.0*((float)analogRead(BATTERY_PIN))/4095.0; 
  uint8_t batPerc = (uint8_t)(100.0*(batVolt - 3.7)/(5.0-3.7)); // 5.0 = 100%, 3.7 = 0%
  if(batPerc > 100) batPerc = 100;
  if(batPerc < 0) batPerc = 0;
  Serial.println("batPerc= " + String(batPerc));
  return batPerc;
}

// low battery mode
// flash low brightness red until battery percentage goes back up
// flash twice quickly and then wait 2 seconds
void red_ring()
{
  #ifdef DEBUG_OLED
  display.clear();
  display.display();
  #endif
  
  leds.SetBrightness(1);
  uint8_t batPerc = LOW_BATTERY;
  while(batPerc <= LOW_BATTERY)
  {
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.SetPixelColor(i, RgbColor(255,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.SetPixelColor(i, RgbColor(0,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.SetPixelColor(i, RgbColor(255,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS; i++)
      leds.SetPixelColor(i, RgbColor(0,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
    batPerc = getBatPercentage();    
  }
  Serial.println("red ring!");
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
    LoRa.beginPacket();
    LoRa.write((byte)0xDE); //DEAD is 2-byte start of message
    LoRa.write((byte)0xAD);
    LoRa.write((byte)DEVICE_ID);
    LoRa.write((byte)batPerc); // uint8_t
    LoRa.write((byte)numSat); // uin8_t
    //for(int jj = 0; jj < 2; jj++) {LoRa.write((byte)0xFF);}
    LoRa.write((byte)255);
    LoRa.write((byte)255);
    LoRa.write((byte)255);
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double
    LoRa.write((byte)0xBE); //BEEF is 2-byte end of message
    LoRa.write((byte)0xEF);   
    LoRa.endPacket(true);   
  
    return 255;
  }
  else
  {
    float dlat = (float)lat - MAN_LAT;
    float dlon = (float)lon - MAN_LON;
  
    float m_dx = dlon * METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
    float m_dy = dlat * METERS_PER_DEGREE;
  
    float dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy);
    float bearing = DEG_PER_RAD * atan2(m_dx, m_dy);
  
    float clock_hours = (bearing / 360. * 12. + NORTH);
    uint16_t clock_minutes = (uint16_t)(clock_hours * 60 + .5);
    // Force into the range [0, CLOCK_MINUTES)
    clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES;
  
    uint8_t hour = clock_minutes / 60;
    uint8_t minute = clock_minutes % 60;

    Serial.println("hour=" + String(hour) + " minute=" + String(minute));
    Serial.println(fmtPlayaStr(lat, lon));
  
    if(dist > 65534) dist = 65535;
    
    uint16_t distRounded = (uint16_t)(dist + .5); // meters
    // 11 byte message
    LoRa.beginPacket();
    //Serial.println("after begin pky");
    LoRa.write((byte)0xDE); //DEAD is 2-byte start of message
    LoRa.write((byte)0xAD);
    LoRa.write((byte)DEVICE_ID);
    LoRa.write((byte)batPerc); // uint8_t
    LoRa.write((byte)numSat); // uin8_t
    LoRa.write((byte)hour);
    LoRa.write((byte)minute);
    LoRa.write((byte *) &distRounded, 2);
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double
    LoRa.write((byte)0xBE); //BEEF is 2-byte end of message
    LoRa.write((byte)0xEF); 
    LoRa.endPacket(); 

    return get_pixel_id(hour, minute, dist);
  }
}

void display_oled_header()
{
  #ifdef DEBUG_OLED
  display.clear();
  //display.setCursor(0,0);
  display.drawString(0,0,String(DEVICE_ID) + "v" + String(VERSION,1) + " " + String(millis()/60000) + "m"); 
  //display.println(lat, 6);
  //display.println(lon, 6);
  //display.println(String(spd) + "mph");
  //display.println(String(hour) + ":" + String(minute));
  //float batVolt = 5.0*((float)analogRead(A0))/1023.0; 
  //display.println(String(batVolt)); 
  display.drawString(64, 0, "#s=" + String(numSat) + " %" + String(batPerc)); 
  #endif
}

//uint8_t ringIdx[] =      {   2,   4,   6,    8,   10,  12,     13,  14};
uint8_t chanIdx[] =      {   2,   2,   2,    2,       2,   2,     2,   2};
uint8_t numLedByChan[] = { 27,  24,  20,  16,  12,   8,       5,   3};
uint8_t numRingsTotal = 8;
uint8_t numLedByRing[] = { 27,  24,  20,  16,  12,  8,  5, 3};

uint8_t get_pixel_id(uint8_t hr, uint8_t minute, uint16_t dist)
{
  if(hr == 255 && minute == 255)
    return 255;
    
  int ring_i = getReferenceRing((float)dist);
  if(ring_i > numRingsTotal) ring_i = numRingsTotal;
  ring_i = numRingsTotal- ring_i;

  int offset = 0;
  for(int i = 0; i < ring_i-1; i++)
  {
    offset += numLedByRing[i];
  }

  // 12*60 

  float alpha = (hr*60 + minute)/(12.0*60.0);
  if(alpha > 1.0) alpha = 1.0;

  offset += (int)alpha*numLedByRing[ring_i];

  return offset;

  /*if(refRing < 2) // man to esplanade
  {
    //return 39-(((int)hr%12)*60 + minute)/45; // middle ring (39 to 24)
    return 40+((((int)hr%12)*60 + minute+30)/60+1)%12; // center ring (40 to 51)
  }
  else
  {
    return (((((int)hr%12)*60 + minute+15)/30)+1)%24; // outer ring (0-23)
  }*/
}

