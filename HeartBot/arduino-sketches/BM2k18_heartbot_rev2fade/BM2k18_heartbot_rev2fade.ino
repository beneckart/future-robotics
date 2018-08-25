// NeoPixelFunFadeInOut
// This example will randomly pick a color and fade all pixels to that color, then
// it will fade them to black and restart over
// 
// This example demonstrates the use of a single animation channel to animate all
// the pixels at once.
//
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

#define FADE_TYPE 0

const int OUTPUT_TYPE = SERIAL_PLOTTER;

const int PULSE_INPUT = A2;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED for lilypad
//const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

PulseSensorPlayground pulseSensor;

const uint16_t PixelCount = 16; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = A4;  // make sure to set this to the correct pin, ignored for Esp8266
const uint8_t AnimationChannels = 1; // we only need one as all the pixels are animated at once

NeoPixelBus<NeoGrbFeature, Neo400KbpsMethod> strip(PixelCount, PixelPin);
// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
//
// NeoEsp8266Uart800KbpsMethod uses GPI02 instead

NeoGamma<NeoGammaTableMethod> colorGamma; // for any fade animations, best to correct gamma

NeoPixelAnimator animations(AnimationChannels); // NeoPixel animation management object

uint16_t effectState = 0;  // general purpose variable used to store effect state


// what is stored for state is specific to the need, in this case, the colors.
// basically what ever you need inside the animation update function
struct MyAnimationState
{
    RgbColor StartingColor;
    RgbColor EndingColor;
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

// one entry per pixel to match the animation timing manager
MyAnimationState animationState[AnimationChannels];

int myBPM = 60;

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

// simple blend function
void BlendAnimUpdate(const AnimationParam& param)
{
    float progress = animationState[param.index].Easeing(param.progress);
    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbColor updatedColor = RgbColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        progress);

    // apply the color to the strip
    for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
    {
        //strip.SetPixelColor(pixel, updatedColor);
        strip.SetPixelColor(pixel, updatedColor);
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
        uint16_t time = 1000*myBPM/60;//random(800, 2000);

        animationState[0].StartingColor = strip.GetPixelColor(0);
        animationState[0].EndingColor = target;

        animations.StartAnimation(0, time, BlendAnimUpdate);
    }
    else if (effectState == 1)
    {
        // fade to black
        uint16_t time = random(60, 70);

        animationState[0].StartingColor = strip.GetPixelColor(0);
        animationState[0].EndingColor = RgbColor(0);

        animations.StartAnimation(0, time, BlendAnimUpdate);
    }

    // toggle to the next effect state
    effectState = (effectState + 1) % 2;
}

void setup()
{
    Serial.begin(115200);
    strip.Begin();
    strip.Show();

    SetRandomSeed();
    
    // Configure the PulseSensor manager.
    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.blinkOnPulse(PULSE_BLINK);
  
    pulseSensor.setSerial(Serial);
    pulseSensor.setOutputType(OUTPUT_TYPE);
    pulseSensor.setThreshold(THRESHOLD);

    //FadeInFadeOutRinseRepeat(0.2f); // 0.0 = black, 0.25 is normal, 0.5 is bright

    // Now that everything is ready, start reading the PulseSensor signal.
    if (!pulseSensor.begin()) {
      /*
         PulseSensor initialization failed,
         likely because our particular Arduino platform interrupts
         aren't supported yet.
  
         If your Sketch hangs here, try changing USE_ARDUINO_INTERRUPTS to false.
         which doesn't use interrupts.
      */
      for(;;) {
        // Flash the led to show things didn't work.
        digitalWrite(PULSE_BLINK, LOW);
        delay(50);
        digitalWrite(PULSE_BLINK, HIGH);
        delay(50);
      }
    }
}


void loop()
{
    myBPM = pulseSensor.getBeatsPerMinute();
    
    if (pulseSensor.sawStartOfBeat()) 
    {
      animations.StopAnimation(0);
      pulseSensor.outputBeat();
 
      //strip.ClearTo(RgbColor(0));
      for(int i = 0; i < AnimationChannels; i++) 
      {
        //animations.RestartAnimation(i);
        RgbColor target = HslColor(random(360) / 360.0f, 1.0f, 0.2);

        if(FADE_TYPE == 1)
        {
          animationState[0].StartingColor = strip.GetPixelColor(0);
          animationState[0].EndingColor = target;
        }
        else if(FADE_TYPE == 0)
        {
          animationState[0].StartingColor = RgbColor(0);
          animationState[0].EndingColor = target;          
        }
        uint16_t time = 17*myBPM;//random(800, 2000);

        AnimEaseFunction easing;

        easing = NeoEase::CubicOut;

        //cubic in starts slow and then accelerates (so seems less responsive)

        //ExponentialInOut, Gamma

        /*switch (random(3))
        {
        case 0:
            easing = NeoEase::CubicIn;
            break;
        case 1:
            easing = NeoEase::CubicOut;
            break;
        case 2:
            easing = NeoEase::QuadraticInOut;
            break;
        }*/
        animationState[0].Easeing = easing;
        
        animations.StartAnimation(0, time, BlendAnimUpdate);
      }

      //FadeInFadeOutRinseRepeat(0.2f);
   
      //Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".
      //Serial.print("BPM: ");                        // Print phrase "BPM: " 
      //Serial.println(myBPM);
    }  

 
    animations.UpdateAnimations();
    strip.Show();
 
 
}



