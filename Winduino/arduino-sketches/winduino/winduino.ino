#include <Wire.h>
#include <Adafruit_GFX.h>

#include "NeoPatterns.cpp"

NeoPatterns leds(500, A0, NEO_GRB + NEO_KHZ800, &RingComplete);


// integer variable to hold current counter value
int count = 0;
int s1 = 0;
int s2 = 0;
int s3 = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(A1, INPUT_PULLUP);
  
  leds.setBrightness(255);
  leds.begin();

  //leds.TheaterChase(leds.Color(255,0,0), leds.Color(0,0,0), 300);
  //leds.RainbowCycle(300);
  uint32_t color1 = leds.Wheel(random(255));
  leds.Scanner(color1, 300);
  leds.show();

}

void loop()
{

  s2 = analogRead(A1);
  int interval = max(1, 150 - s2/2);

  leds.Interval = interval;
  
  leds.Update(); 
  
  if(s1 < 500 || s2 < 500 || s3 < 500)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // increment the counter by 1
  count++;

  yield();

}

void RingComplete()
{
    uint32_t color1 = leds.Wheel(random(255));
    leds.Scanner(color1, 300);
    //leds.TheaterChase(leds.Color(255,0,0), leds.Color(0,0,0), 300);
    //leds.RainbowCycle(300);
}
