#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
//#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include "TalismanPatterns.cpp"
#include <TinyGPS++.h>
#include "FS.h"
#include "SPIFFS.h"
#include "Switch.h"
#include "SSD1306.h" 

//2.3: 915E3, 125e3, SF11, Tx17 (camp to A)
//2.4: 915E3,62.5E3, SF12, Tx20 (too long? no recv)
//2.5: 915E3, 125, 12, 20
#define VERSION 2.5

//UNCOMMENT OUT IF AT BM!!
#define PRODUCTION

#define SERIAL_BAUD 9600

// 0-5 Dad
// *6 - Jimmy (was JP on Wednesday?) -- gave to him
// *7 - Luca -- gave to him
// *8 - JP (was Ben on Wednesday?) -- left by bed
// *9(2) - Elliot
// 10 - Shervin
// 11 - Ben

// NEED 11, 10 -- and need to charge them! (Elliot/Shervin?

// made an 11(2), 9(2), but 11(2) has bad usb port so ditching it

// gave Talisman 6 to Jimmy (6) -- are the channels right?
// flashed 11(2) for me (Ben)
// wrote JP on 8
// gave 7 to Luca

#define DEVICE_ID 9 //later, replace this with chipid

#define NUM_TALISMAN 12

//#define MMM 
#define MMM_PIN 2

// user set defines
#include "TalismanDefines.h" 

// other defines
#include "HardwareDefines.h"
#include "PlayaDefines.h"

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
 * Main talisman objects (gps and leds)
 */
 
uint64_t chipid;  
 
TinyGPSPlus gps;

Switch button = Switch(BUTTON_PIN, INPUT_PULLUP, LOW, 40, 400, 250);

#ifdef MMM
int PixelCount = 20;
TalismanPatterns<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> leds2(PixelCount, 14, RingComplete);
TalismanPatterns<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> leds(PixelCount, MMM_PIN, RingComplete);
NeoPixelAnimator animations(1);
uint16_t effectState = 0;  // general purpose variable used to store effect state

// what is stored for state is specific to the need, in this case, the colors.
// basically what ever you need inside the animation update function
struct MyAnimationState
{
    RgbColor StartingColor;
    RgbColor EndingColor;
};

// one entry per pixel to match the animation timing manager
MyAnimationState animationState[1];



// simple blend function
void BlendAnimUpdate(const AnimationParam& param)
{
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        param.progress);

    // apply the color to the strip
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        //strip.SetPixelColor(pixel, updatedColor);
        leds.SetPixelColor(pixel, updatedColor);
        //strip.SetPixelColor(pixel, colorGamma.Correct(updatedColor));
    }
}

void FadeInFadeOutRinseRepeat(float luminance)
{
    if (effectState == 0)
    {
        // Fade upto a random color
        // we use HslColor object as it allows us to easily pick a hue
        // with the same saturation and luminance so the colors picked
        // will have similiar overall brightness
        RgbColor target = HslColor(random(360) / 360.0f, 1.0f, luminance);
        uint16_t time = random(800*5, 2000*5);

        animationState[0].StartingColor = leds.GetPixelColor(0);
        animationState[0].EndingColor = target;

        animations.StartAnimation(0, time, BlendAnimUpdate);
    }
    else if (effectState == 1)
    {
        // fade to black
        RgbColor target = HslColor(random(360) / 360.0f, 1.0f, luminance);
        uint16_t time = random(800*5, 2000*5);

        animationState[0].StartingColor = leds.GetPixelColor(0);
        animationState[0].EndingColor = target;

        animations.StartAnimation(0, time, BlendAnimUpdate);
    }

    // toggle to the next effect state
    effectState = (effectState + 1) % 2;
}
#else
TalismanPatterns<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod> leds(NUM_NEOPIXELS_STRAND1, LED_PIN1, RingComplete);
TalismanPatterns<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> leds2(NUM_NEOPIXELS_STRAND2, LED_PIN2, RingComplete);
#endif //MMM endif

enum BUTTON_STATE { MODE_PARTY, MODE_PARTY2, MODE_MAP, MODE_DIAGNOSTIC, MODE_TRACKER_ONLY };

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


const uint8_t num_rings = 15;

uint8_t num1ByRing[] = {29, 0, 25,  0, 22,  0, 18,  0, 14, 0, 10,  0,  0,    0,  0};
uint8_t num2ByRing[] = {0, 27,  0, 24,  0, 20,  0, 16,  0, 12, 0,  9,  6,    4,  2};


uint8_t chanIdx[] =    {1,  2,  1, 2,  1,   2, 1,   2,  1, 2,  1,  2,  2,    2,  2};  
uint16_t ch1_ringStartIdx[num_rings]; 
uint16_t ch2_ringStartIdx[num_rings]; 

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
uint16_t prevPixelId[NUM_TALISMAN];
uint16_t pixelId[NUM_TALISMAN];
unsigned long updateTimes[NUM_TALISMAN];

unsigned char lora_buffer[LORA_PKT_SIZE]; // circular buffer
unsigned int pos = 0; // current position in lora buffer

uint8_t batPerc = 99; //getBatPercentage();  

double lat = 0;
double lon = 0;
double prevLat = 0;
double prevLon = 0;
double displacement = 0.0;

uint8_t numSat = 0; // (uint8_t) gps.satellites.value();
float spd = 0.0;
uint8_t month;// = gps.date.month();
uint8_t day;// = gps.date.day();
uint8_t hour;// = gps.time.hour();
uint8_t minute;// = gps.time.minute();
uint8_t second = 255;// dummy val to start
uint8_t centisecond = 255;
uint32_t millisecond = 1000;

bool prev_update_sent = true;
bool blinkState = true;

void setup() {
 
#ifdef DEBUG_OLED
  init_oled();
#endif

  Serial.begin(SERIAL_BAUD);//, SERIAL_8N1); 
  
  chipid = ESP.getEfuseMac();
  Serial.printf("DevId: %02d, ESP32 Chip ID = %04X", DEVICE_ID, (uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

  // GPS, hardcoded for T-Beam
  Serial1.begin(9600, SERIAL_8N1, 12, 15); 
  
  
#ifdef LOGGING
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
  }
  // uncomment for reading back log file
  #ifdef READ_LOG_THEN_QUIT
  read_log_then_quit();
  #endif

  open_log();  
#endif // end logging code

  // set initial pixel mappings to dummy vals
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    pixelId[i] = INVALID_PIXEL_ID;
    prevPixelId[i] = INVALID_PIXEL_ID;
  }

  init_lora();
  init_leds();
  
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
  clear_leds();

  delay(250);
  
  //randomSeed(LoRa.random());
  SetRandomSeed();

  ////leds.TheaterChase(RgbColor(255,255,0), RgbColor(0,0,50), 100);
  ////leds.Fade(RgbColor(255,0,0), RgbColor(0,255,0), 255, 20, FORWARD);

#ifdef MMM
  leds.Begin();
  leds.Show();

  SetRandomSeed();
#else
  //leds.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
  //leds2.Scanner(colorMap[DEVICE_ID], LED_MIN_DELAY_MS);
  RingComplete();
#endif


#ifdef DEBUG_OLED
  display.println("fin setup!");
  display.display();
#endif

  Serial.println("fin setup!");
  uint16_t idx1_so_far = 0;
  uint16_t idx2_so_far = 0;
  for (int i = 0; i < num_rings; ++i) {
    ch1_ringStartIdx[i] = idx1_so_far;
    ch2_ringStartIdx[i] = idx2_so_far;
    idx1_so_far += num1ByRing[i];
    idx2_so_far += num2ByRing[i];
  }
  for(int i = 0; i < NUM_TALISMAN; i++)
  {
    updateTimes[i] = 0UL;
  }

  delay(200);
}

void loop() {

  //wdt_disable();
  check_gps();
  //wdt_enable(5000);

  yield();

  unsigned long currTime = millis();

  check_lora_send(currTime);

  yield();

  check_lora_recv();

  yield();

  check_map_update(currTime);

  yield();

  check_diagnostic_update(currTime);

  yield();
  
  check_battery(currTime);

  yield();

#ifdef LOGGING
  check_logging(currTime);
#endif

  yield();

#ifdef DEBUG_OLED
  if(currTime - last_oled >= OLED_UPDATE_DELAY_MS)
  {
    //display_oled_header();
    //display.drawString(0,26, fmtPlayaStr(lat, lon));
    //display.display();
    last_oled = millis();
  }
#endif

  yield();

  if(button_state == MODE_PARTY || button_state == MODE_PARTY2) // &&  currTime - last_led >= LED_PARTY_UPDATE_DELAY_MS)
  {
    #ifdef MMM
    //Serial.println("party  update");
    if (animations.IsAnimating())
    {
        // the normal loop just needs these two to run the active animations
        animations.UpdateAnimations();
        leds.Show();
    }
    else
    {
        // no animation runnning, start some 
        //
        FadeInFadeOutRinseRepeat(0.2f); // 0.0 = black, 0.25 is normal, 0.5 is bright
        //Serial.println("TEST");
    }
    #else //NORMAL TALISMAN OPERATION
    leds.Update(); 
    leds2.Update();
    leds2.Show(); leds.Show();
    #endif
  }

  //leds2.Show(); leds.Show();

  button.poll();

  if(button.pushed())
  {
    //Serial.println("button pushed!");
    switch(button_state)
    {
      case MODE_PARTY:
          button_state = MODE_PARTY2;
          leds.ActivePattern = RAINBOW_CYCLE;
          leds2.ActivePattern = RAINBOW_CYCLE;
          leds.RainbowCycle(30);
          //leds.SetBrightness(50);
          leds2.RainbowCycle(30);
          //leds2.SetBrightness(50);
          leds.SetBrightness(LED_BRIGHTNESS_FADE);
          leds2.SetBrightness(LED_BRIGHTNESS_FADE);
          break;
      case MODE_PARTY2:
          button_state = MODE_MAP;
          leds.ActivePattern = NONE;
          leds2.ActivePattern = NONE;

          ////leds.ActivePattern = FADE_INNER_RING;
          ////leds.Interval = 100;
          ////leds.TotalSteps = LED_BRIGHTNESS_FADE;
          leds.SetBrightness(50);
          leds2.SetBrightness(50);
          ////color1 = RgbColor(0,0,0);
          ////color2 = RgbColor(LED_BRIGHTNESS_FADE,LED_BRIGHTNESS_FADE,LED_BRIGHTNESS_FADE);
          ////leds.StartIndex = 24;
          ////leds.EndIndex = 24+16;
          leds.ClearTo(RgbColor(0));
          leds.Show(); 
          leds2.ClearTo(RgbColor(0));
          leds2.Show(); 
          //Serial.println("map mode");
          break;
      case MODE_MAP:
          button_state = MODE_DIAGNOSTIC;
          leds.ActivePattern = NONE;
          leds.SetBrightness(LED_BRIGHTNESS_FADE);
          leds2.SetBrightness(LED_BRIGHTNESS_FADE);
          leds.ClearTo(RgbColor(0));
          leds.Show(); 
          leds2.ActivePattern = NONE;
          leds2.ClearTo(RgbColor(0));
          leds2.Show(); 
          //Serial.println("diagnostic mode");
          break;
      case MODE_DIAGNOSTIC:
          button_state = MODE_TRACKER_ONLY;
          leds.ActivePattern = NONE;
          leds.ClearTo(RgbColor(0));
          leds.Show(); 
          leds2.ActivePattern = NONE;
          leds2.ClearTo(RgbColor(0));
          leds2.Show(); 
          //Serial.println("tracker mode");
          break;
      case MODE_TRACKER_ONLY:
          button_state = MODE_PARTY;
          leds.SetBrightness(LED_BRIGHTNESS);
          leds2.SetBrightness(LED_BRIGHTNESS);
          RingComplete();
          //Serial.println("party mode");
          break;
      default:
          break;
    } //end switch    

  }
  //leds2.Show(); leds.Show();
}

void clear_leds()
{
  leds.ClearTo(RgbColor(0,0,0));
  leds2.ClearTo(RgbColor(0,0,0));
  //leds.Show();
  //leds2.Show();
}


void check_gps()
{
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
  } // end gps read
}

void RingComplete()
{
  #ifndef MMM
    if(button_state == MODE_PARTY)//leds.ActivePattern != FADE_INNER_RING)
    {
      pattern randPattern = (pattern)(random(5)+1); // starts with NONE 
      //pattern randPattern = (pattern)(LoRa.random()%5+1); // starts with NONE 
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
      //pattern randPattern = (pattern)(random(5)+1); // starts with NONE 
      //pattern randPattern = (pattern)(LoRa.random()%5+1); // starts with NONE 
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
    //Serial.println("OLD PARTY MODE");
    #endif
}

uint8_t getBatPercentage()
{
  float batVolt = 10.0*((float)analogRead(BATTERY_PIN))/(4095.0); 
  uint8_t batPerc = (uint8_t)(100.0*(batVolt - 3.3)/(5.0-3.3)); // 5.0 = 100%, 3.7 = 0%
  if(batPerc > 100) batPerc = 100;
  if(batPerc < 0) batPerc = 10;
  //Serial.println("batPerc= " + String(batPerc));
  return batPerc;
}

// low battery mode
// flash low brightness red until battery percentage goes back up
// flash twice quickly and then wait 2 seconds
void red_ring()
{
  /*#ifdef DEBUG_OLED
  display.clear();
  display.display();
  #endif
  
  leds.SetBrightness(1);
  uint8_t batPerc = LOW_BATTERY;
  while(batPerc <= LOW_BATTERY)
  {
    for(int i = 0; i < NUM_NEOPIXELS_STRAND1; i++)
      leds.SetPixelColor(i, RgbColor(255,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS_STRAND1; i++)
      leds.SetPixelColor(i, RgbColor(0,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS_STRAND1; i++)
      leds.SetPixelColor(i, RgbColor(255,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    for(int i = 0; i < NUM_NEOPIXELS_STRAND1; i++)
      leds.SetPixelColor(i, RgbColor(0,0,0));
    leds.Show();
    //digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
    batPerc = getBatPercentage();    
  }*/
  //Serial.println("red ring!");
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
    /*LoRa.write((byte)255);
    LoRa.write((byte)255);
    LoRa.write((byte)255);
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double*/
    Serial.write((byte)10);
    Serial.write((byte)45);
    Serial.write((byte)0);
    Serial.write((byte)0); 
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double    
    LoRa.write((byte)0xBE); //BEEF is 2-byte end of message
    LoRa.write((byte)0xEF);   
    LoRa.endPacket(true);   

#ifdef MMM
    /*Serial.write((byte)0xDE); //DEAD is 2-byte start of message
    Serial.write((byte)0xAD);
    Serial.write((byte)DEVICE_ID);
    Serial.write((byte)batPerc); // uint8_t
    Serial.write((byte)numSat); // uin8_t
    Serial.write((byte)10);
    Serial.write((byte)45);
    Serial.write((byte)0);
    Serial.write((byte)0); 
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double
    Serial.write((byte)0xBE); //BEEF is 2-byte end of message
    Serial.write((byte)0xEF); */
    Serial.println(String(DEVICE_ID) + " " + String(hour) + " " + String(minute) + " " + String(0));      //note iff MMM tracker, then this will never happen
#endif
  
    return 255;
  }
  else
  {
    float dlat = (float)lat - MAN_LAT;
    float dlon = (float)lon - MAN_LON;
  
    float m_dx = dlon * METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
    float m_dy = dlat * METERS_PER_DEGREE;    

    float dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy);
    float bearing = (DEG_PER_RAD * atan2(m_dx, m_dy));
    if(bearing >= 360) bearing -= 360;
    if(bearing < 0) bearing += 360;
  
    float clock_hours = (bearing / 360. * 12. + NORTH);
    if(clock_hours >= 12) clock_hours -= 12;
    int clock_minutes = (int)(clock_hours * 60 + .5);
  
    uint8_t hour = clock_minutes / 60;
    uint8_t minute = clock_minutes % 60;

    //Serial.println("hour=" + String(hour) + " minute=" + String(minute));
    //Serial.println(fmtPlayaStr(lat, lon));
  
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

#ifdef MMM
    /*Serial.write((byte)0xDE); //DEAD is 2-byte start of message
    Serial.write((byte)0xAD);
    Serial.write((byte)DEVICE_ID);
    Serial.write((byte)batPerc); // uint8_t
    Serial.write((byte)numSat); // uin8_t
    Serial.write((byte)hour);
    Serial.write((byte)minute);
    Serial.write((byte *) &distRounded, 2);
    //Serial.write((byte *) &lat, sizeof(lat)); // 8 bytes in a double
    //Serial.write((byte *) &lon, sizeof(lon)); // 8 bytes in a double
    Serial.write((byte)0xBE); //BEEF is 2-byte end of message
    Serial.write((byte)0xEF);  */
    Serial.println(String(DEVICE_ID) + " " + String(hour) + " " + String(minute) + " " + String(distRounded));  
#endif

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

uint8_t get_pixel_id(uint8_t hr, uint8_t minte, uint16_t dist)
{
  if(hr == 255 && minte == 255)
    return 255;
    
  int ring_i = (int)(((float)dist)/ringRadius(16)*num_rings);
  if(ring_i > num_rings) ring_i = num_rings-1;
  ring_i = num_rings - ring_i - 1;

  float alpha = (((float)hr)*60.0 + (float)minte)/(12.0*60.0);
  
  int chan = chanIdx[ring_i];
  
  int offset;
  if(chan == 1)
  {
    offset = ch1_ringStartIdx[ring_i]; //118
    offset += (int)(alpha*(num1ByRing[ring_i] + num2ByRing[ring_i])+0.5)+1;  
    //Serial.println("CHANNEL 1: ch1_ringstart=" + String(ch1_ringStartIdx[ring_i]) + ", num1ByRing=" + String(num1ByRing[ring_i]) + ", offset addition=" + String(alpha*num1ByRing[ring_i]));   
  }
  else
  {
    offset = ch2_ringStartIdx[ring_i];
    offset += (int)((alpha*(num1ByRing[ring_i] + num2ByRing[ring_i])) - num1ByRing[ring_i]+0.5)+1;
    //Serial.println(String((int)(alpha*num2ByRing[ring_i])) + " num2byring[14]=" + String(num2ByRing[ring_i]) + " ring start=" + String(ch2_ringStartIdx[ring_i])); 
  }
  return NUM_NEOPIXELS_STRAND1*(chan-1) + offset;
}

#ifdef DEBUG_OLED
void init_oled()
{
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  pinMode(OLED_PIN,OUTPUT);
  digitalWrite(OLED_PIN, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_PIN, HIGH); // while OLED is running, must set GPIO16 in high、
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
}
#endif

void init_lora()
{
  //initialize LoRa
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
  //LoRa.receive(); // I think only for continuous receive mode
  LoRa.setSpreadingFactor(SPREADING_FACTOR);   
  //LoRa.setSyncWord(0xBE); //ignore everything but BE, defaults to 0x12
  LoRa.setTxPower(TX_PWR); //affects battery life how?
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CODING); //between 5..8
  //preamble length in symbols, defaults to 8. Supported values are between 6 and 65535
  //LoRa.setPreambleLength(preambleLength);
  //LoRa.enableCrc();
  //LoRa.disableCrc(); //default
  
  Serial.println("LoRa init ok");
}

void read_log_then_quit()
{
  logfile = SPIFFS.open("/gps_log.txt", "r");
  #ifdef DEBUG_OLED  
    display.clear(); 
    display.drawString(0,0,"ID:" + String(DEVICE_ID)); 
    display.drawString(0,16,"Logf dump! Press btn to begin");
    display.display();
  #endif
  while(digitalRead(BUTTON_PIN)){delay(50);}
  #ifdef DEBUG_OLED  
    display.drawString(0,32,"dumping...");
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
    display.clear();
    display.drawString(0,0,"End dump");
    display.drawString(0,16,"Reset me");
    display.display();
  #endif  
  while(true){delay(100);}
  
  // WARNING: uncomment to delete log!!!
  /*if (SPIFFS.exists("/gps_log.txt")){ SPIFFS.remove("/gps_log.txt");}
  delay(1000000);*/

}

void open_log()
{
    // this opens the file in append mode
  logfile = SPIFFS.open("/gps_log.txt", "a");
  logfile.println("\nlog start");
  
  if (!logfile) { 
    #ifdef DEBUG_OLED  
    display.clear();
    display.drawString(0,0,"Logf fail!");
    display.display();
    #endif
  }
  else {
    #ifdef DEBUG_OLED  
    display.clear();
    display.drawString(0,0,"Logf appnd");
    display.display();
    #endif    
  }

}

void check_lora_send(unsigned long currTime)
{
  if(!RECEIVE_ONLY && ((millisecond+currTime-gps_update_time)/LORA_SEND_DELAY_MS)%NUM_TALISMAN == DEVICE_ID && currTime - last_send >= LORA_SEND_DELAY_MS*(NUM_TALISMAN-1))
  {
    //Serial.println("LoRa sent " + String((millisecond+currTime-gps_update_time)/LORA_SEND_DELAY_MS) + " " + String(currTime - last_send));
    uint8_t selfPixelId = sendPkt(lat, lon, batPerc, numSat);
    //Serial.println(playaStr(lat, lon));
    pixelId[DEVICE_ID] = selfPixelId;
    
    last_send = currTime;
    updateTimes[DEVICE_ID] = last_send;

  } // end lora send  
}

void init_leds()
{
  leds.Begin(); leds2.Begin();
  leds.SetBrightness(LED_BRIGHTNESS);
  leds.ClearTo(RgbColor(0));
  leds2.SetBrightness(LED_BRIGHTNESS);
  leds2.ClearTo(RgbColor(0));
  leds.Show(); leds2.Show();
  
}

void check_lora_recv()
{
  // something on the LoRa line?
  //for(int i = 0; i < 9; i++)
  //  pixelId[i] = get_pixel_id(2 + i, 0, ringRadius(1 + i));
  
  /*animState += 1;
  if(animState > ringRadius('N'-'A'+2))
    animState = 0;*/
  //Serial.println("pixelId[0]=" + String(pixelId[0]));
  /*  
  if(LoRa.available())
  {
    lora_buffer[pos%LORA_PKT_SIZE] = (char) LoRa.read();
#ifdef MMM
    Serial.print(lora_buffer[pos%LORA_PKT_SIZE]);
#endif
    if(lora_buffer[pos%LORA_PKT_SIZE] == 0xEF && lora_buffer[(pos-1)%LORA_PKT_SIZE] == 0xBE && lora_buffer[(pos+1)%LORA_PKT_SIZE] == 0xDE && lora_buffer[(pos+2)%LORA_PKT_SIZE] == 0xAD)
    {      
      int devId = (int)lora_buffer[(pos+3)%LORA_PKT_SIZE];
      batPercOthers[devId] = lora_buffer[(pos+4)%LORA_PKT_SIZE];
      numSatOthers[devId] = lora_buffer[(pos+5)%LORA_PKT_SIZE];
      hourOthers[devId] = lora_buffer[(pos+6)%LORA_PKT_SIZE];
      minuteOthers[devId] = lora_buffer[(pos+7)%LORA_PKT_SIZE];
      *((unsigned char *)&distOthers + 2*devId) = lora_buffer[(pos+8)%LORA_PKT_SIZE];
      *((unsigned char *)&distOthers + 2*devId+1) = lora_buffer[(pos+9)%LORA_PKT_SIZE];

      //Serial.print("id=" + String(devId) + ": ");
      //Serial.println(playaStrFromPkt(hourOthers[devId], minuteOthers[devId], distOthers[devId]));

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
    pos++;
  }*/
    /*while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("Snr: " + String(LoRa.packetSnr()));
    Serial.println();    
    */

  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    // TODO: is this a better construction?

    for(int ii = 0; ii < packetSize; ii++)
    {
    lora_buffer[pos%LORA_PKT_SIZE] = (unsigned char) LoRa.read();
    
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

        //Serial.print("id=" + String(devId) + ": ");
        //Serial.println(playaStrFromPkt(hourOthers[devId], minuteOthers[devId], distOthers[devId]));

        #ifdef DEBUG_OLED
        display_oled_header();
        display.drawString(0,14,"id=" + String(devId) + ": "); 
        display.drawString(0,26,playaStrFromPkt(hourOthers[devId], minuteOthers[devId], distOthers[devId]));
        display.display();
        #endif

        last_recv = millis();

        pixelId[devId] = get_pixel_id(hourOthers[devId], minuteOthers[devId], distOthers[devId]);
        updateTimes[devId] = last_recv;

        #ifdef MMM
        Serial.println(String(devId) + " " + String(hourOthers[devId]) + " " + String(minuteOthers[devId]) + " " + String(distOthers[devId]) + " 0");
        #endif
      }
    }
    pos++;
    }
  }
}

void SetPixelColor(uint16_t idx, RgbColor c)
{
   if(idx >= NUM_NEOPIXELS_TOTAL)
      return;
   if(idx / NUM_NEOPIXELS_STRAND1 == 0)
        leds.SetPixelColor(idx % NUM_NEOPIXELS_STRAND1, c);
    else
        leds2.SetPixelColor(idx % NUM_NEOPIXELS_STRAND1, c); 
}

void check_map_update(unsigned long currTime)
{
  if(button_state == MODE_MAP && currTime - last_led >= LED_MAP_UPDATE_DELAY_MS)
  {
    //Serial.println("map update start");
    uint8_t currDevId = device_ctr%NUM_TALISMAN;
    uint16_t currPixelId = pixelId[currDevId];
    if(currTime - updateTimes[currDevId] > STALE_MAP_POINT_SEC*1000)
    {
      // point is stale, so remove it
      SetPixelColor(currPixelId, 0);
      pixelId[currDevId] = INVALID_PIXEL_ID;
      prevPixelId[currDevId] = INVALID_PIXEL_ID;
    }  
    uint16_t prevPixelId_tmp = prevPixelId[currDevId];

    SetPixelColor(prevPixelId_tmp, 0);
    SetPixelColor(currPixelId, colorMap[currDevId]);
    
    prevPixelId[currDevId] = currPixelId;

    leds.Show(); leds2.Show();  
    device_ctr++;
    last_led = millis();
    //Serial.println("map update");
  }
}

void check_battery(unsigned long currTime)
{
  if(currTime - last_bat_check > BATTERY_CHECK_DELAY_MS)
  {
    batPerc = getBatPercentage();

    if(batPerc < LOW_BATTERY)
    {
      red_ring(); // low battery mode
    }
    last_bat_check = millis();
  }

}

void check_logging(unsigned long currTime)
{
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
}

void check_diagnostic_update(unsigned long currTime)
{
  if(button_state == MODE_DIAGNOSTIC && currTime - last_diagnostic >= DIAGNOSTIC_UPDATE_DELAY_MS)
  {
    //Serial.println("diagnostic update start");
    leds.ClearTo(RgbColor(0)); leds2.ClearTo(RgbColor(0));
    unsigned long stopLed = num1ByRing[0]*batPerc/100;
    //Serial.println(String(stopLed));
    for(int i = 0; i < stopLed; i++)
    {
        leds.SetPixelColor(i, RgbColor(0,255,0));
        //leds2.SetPixelColor(i, RgbColor(0,50,0));
    }
    
    stopLed = num1ByRing[2]*(currTime - last_send)/(LORA_SEND_DELAY_MS*NUM_TALISMAN);
    stopLed = stopLed > num1ByRing[2]? num1ByRing[2] : stopLed;
    //Serial.println(String(stopLed));
    for(int i = 0; i < stopLed; i++)
    {
      leds.SetPixelColor(num1ByRing[0]+i, RgbColor(0,0,255));
      //leds2.SetPixelColor(num1ByRing[0]+num1ByRing[2]+i, RgbColor(0,0,50));
    }

    stopLed = numSat < num1ByRing[4]? numSat: num1ByRing[4];
    for(int i = 0; i < stopLed; i++)
    {
        //leds.SetPixelColor(i+24+16, colorMap[DEVICE_ID]);
        leds.SetPixelColor(num1ByRing[0]+num1ByRing[2]+num1ByRing[4]+i, RgbColor(255,0,0)); //colorMap[DEVICE_ID]);

    }

    for(int i = 0; i < NUM_TALISMAN; i++)
    {
      if(pixelId[i] < NUM_NEOPIXELS_TOTAL)
      {
        //fresh data
        leds.SetPixelColor(i+num1ByRing[0]+num1ByRing[2]+num1ByRing[4]+num1ByRing[6], colorMap[i]);
      }
      if(i == DEVICE_ID && blinkState == false)
      {
        leds.SetPixelColor(i+num1ByRing[0]+num1ByRing[2]+num1ByRing[4]+num1ByRing[6], RgbColor(0));
        blinkState = true;
      }
      else if(i == DEVICE_ID && blinkState == true)
      {
        blinkState = false;
        leds.SetPixelColor(i+num1ByRing[0]+num1ByRing[2]+num1ByRing[4]+num1ByRing[6], colorMap[i]);
      }
      
    }
    
    leds.Show(); leds2.Show();
    last_diagnostic = millis();
    //Serial.println("diagnostic update");
  }  
}

void SetRandomSeed()
{
    uint32_t seed;

    // random works best with a seed that can use 31 bits
    // analogRead on a unconnected pin tends toward less than four bits
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    randomSeed(seed);
}
