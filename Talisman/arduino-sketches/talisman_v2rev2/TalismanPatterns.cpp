//#include <Adafruit_NeoPixel.h>
#include <NeoPixelBrightnessBus.h>

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, FADE_INNER_RING };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

#define MIDDLE_OFFSET 0

#pragma once

// NeoPattern Class - derived from the Adafruit_NeoPixel class
template<typename T_COLOR_FEATURE, typename T_METHOD> class TalismanPatterns : public NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>
{
public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    RgbColor color1, color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    uint8_t StartIndex; 
    uint8_t EndIndex;
    
    void (*OnComplete)();  // Callback on completion of pattern

    TalismanPatterns(uint16_t countPixels, uint8_t pin, void (*callback)()) :
        NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>(countPixels, pin),
        _brightness(0)
    {
        OnComplete = callback;
    }

    TalismanPatterns(uint16_t countPixels, uint8_t pinClock, uint8_t pinData, void (*callback)()) :
        NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>(countPixels, pinClock, pinData),
        _brightness(0)
    {
        OnComplete = callback;
    }

    TalismanPatterns(uint16_t countPixels, void (*callback)()) :
        NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>(countPixels),
        _brightness(0)
    {
        OnComplete = callback;
    }

    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                case FADE_INNER_RING:
                    FadeInnerRingUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        //Index = 0;
        Index = (dir == FORWARD)? 0 : TotalSteps-1;
        Direction = dir;
        color1 = Wheel(random(255));
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< PixelCount(); i++)
        {
            SetPixelColor(i, Wheel(((i * 256 / PixelCount()) + Index) & 255));
        }
        Show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(RgbColor _color1, RgbColor _color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = PixelCount();
        color1 = _color1;
        color2 = _color2;
        //Index = 0;
        Index = (dir == FORWARD)? 0 : TotalSteps-1;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< PixelCount(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                SetPixelColor(i, color1);
            }
            else
            {
                SetPixelColor(i, color2);
            }
        }
        Show();
        Increment();
    }

    // Initialize for a RgbColorWipe
    void ColorWipe(RgbColor color, uint16_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = PixelCount();
        color1 = color;
        //Index = 0;
        Index = (dir == FORWARD)? 0 : TotalSteps-1;
        Direction = dir;
    }
    
    // Update the RgbColor Wipe Pattern
    void ColorWipeUpdate()
    {
        SetPixelColor(Index, color1);
        Show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(RgbColor _color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (PixelCount() - 1) * 2;
        color1 = _color1;
        Index = 0;
        Direction = FORWARD;
    }

    // Update the Scanner Pattern
    /*void ScannerUpdate()
    { 
        for (int i = 0; i < PixelCount(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 SetPixelColor(i, RgbColor1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 SetPixelColor(i, RgbColor1);
            }
            else // Fading tail
            {
                 SetPixelColor(i, DimRgbColor(getPixelColor(i)));
            }
        }
        Show();
        Increment();
    }*/

    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < PixelCount(); i++)
        {
            int idx = i;
            if (i == Index)  // Scan Pixel to the right
            {
                 SetPixelColor(idx, color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 SetPixelColor(idx, color1);
            }
            else // Fading tail
            {
                 SetPixelColor(idx, DimRgbColor(GetPixelColor(idx)));
            }
        }
        Show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(RgbColor _color1, RgbColor _color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        color1 = _color1;
        color2 = _color2;
        //Index = 0;
        Index = (dir == FORWARD)? 0 : TotalSteps-1;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between RgbColor1 and RgbColor2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(color1) * (TotalSteps - Index)) + (Red(color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(color1) * (TotalSteps - Index)) + (Green(color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(color1) * (TotalSteps - Index)) + (Blue(color2) * Index)) / TotalSteps;
        
        ColorSet(RgbColor(red, green, blue));
        Show();
        Increment();
    }

    // Initialize for a Fade
    void FadeInnerRing(RgbColor _color1, RgbColor _color2, uint16_t steps, uint8_t interval, uint8_t start, uint8_t end, direction dir = FORWARD)
    {
        ActivePattern = FADE_INNER_RING;
        Interval = interval;
        TotalSteps = steps;
        color1 = _color1;
        color2 = _color2;
        //Index = 0;
        Index = (dir == FORWARD)? 0 : TotalSteps-1;
        Direction = dir;
        StartIndex = start;
        EndIndex = end;
    }
    
    // Update the Fade Pattern
    void FadeInnerRingUpdate()
    {
        // Calculate linear interpolation between RgbColor1 and RgbColor2
        // Optimise order of operations to minimize truncation error
        //uint8_t red = ((Red(RgbColor1) * (TotalSteps - Index)) + (Red(RgbColor2) * Index)) / TotalSteps;
        //uint8_t green = ((Green(RgbColor1) * (TotalSteps - Index)) + (Green(RgbColor2) * Index)) / TotalSteps;
        //uint8_t blue = ((Blue(RgbColor1) * (TotalSteps - Index)) + (Blue(RgbColor2) * Index)) / TotalSteps;
        
        uint8_t gray = Index;
        
        ColorSetIndexed(RgbColor(gray, gray, gray), StartIndex, EndIndex);
        Show();
        Increment();
    }


   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    RgbColor DimRgbColor(RgbColor color)
    {
        // Shift R, G and B components one bit to the right
        RgbColor dimRgbColor = RgbColor(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimRgbColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(RgbColor color)
    {
        for (int i = 0; i < PixelCount(); i++)
        {
            SetPixelColor(i, color);
        }
        Show();
    }
    
    // Set all pixels to a color (synchronously)
    void ColorSetIndexed(RgbColor color, uint8_t start, uint8_t end)
    {
        for (int i = start; i < end; i++)
        {
            SetPixelColor(i, color);
        }
        Show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(RgbColor color)
    {
        //return (color >> 16) & 0xFF;
        return color.R;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(RgbColor color)
    {
        //return (color >> 8) & 0xFF;
        return color.G;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(RgbColor color)
    {
        //return color & 0xFF;
        return color.B;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    RgbColor Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return RgbColor(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return RgbColor(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return RgbColor(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
    
    void Show()
    {
        NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>::Show();
    }
    
    uint16_t PixelCount() const
    {
        return NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>::PixelCount();
    }
    
    void SetPixelColor(uint16_t indexPixel, typename T_COLOR_FEATURE::ColorObject color)
    {
        NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>::SetPixelColor(indexPixel, color);
    }
    
    typename T_COLOR_FEATURE::ColorObject GetPixelColor(uint16_t indexPixel) const
    {
        return NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>::GetPixelColor(indexPixel);
    }
    
    void ClearTo(typename T_COLOR_FEATURE::ColorObject color)
    {      
        NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>::ClearTo(color);
    };

protected:
    uint8_t _brightness;
};

void RingComplete();

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
/*NeoPatterns Ring(52, D8, NEO_GRB + NEO_KHZ800, &RingComplete);


// Initialize everything and prepare to start
void setup()
{
  Serial.begin(115200);

   pinMode(D3, INPUT_PULLUP);
   pinMode(D7, INPUT_PULLUP);
    
    // Initialize all the pixelStrips
    Ring.begin();
  
    
    // Kick off a pattern
    Ring.TheaterChase(Ring.RgbColor(255,255,0), Ring.RgbColor(0,0,50), 100);
}

// Main loop
void loop()
{
    // Update the rings.
    Ring.Update(); 
    
    // Switch patterns on a button press:
    if (digitalRead(D7) == LOW) // Button #1 pressed
    {
        // Switch Ring1 to FADE pattern
        Ring.ActivePattern = RAINBOW_CYCLE; //FADE;
        Ring.Interval = 20;
    }
    else if (digitalRead(D3) == LOW) // Button #2 pressed
    {
        // Switch to alternating color wipes on Rings1 and 2
        Ring.ActivePattern = FADE; //COLOR_WIPE;
    }
    else // Back to normal operation
    {
        // Restore all pattern parameters to normal values
        Ring.ActivePattern = NONE; //SCANNER;//THEATER_CHASE;
        Ring.Interval = 100;
    }    

    ///RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE
}*/

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Ring Completion Callback
/*void RingComplete()
{
    if (digitalRead(D3) == LOW)  // Button #2 pressed
    {
        // Alternate color-wipe patterns with Ring2
        Ring.RgbColor1 = Ring.Wheel(random(255));
        Ring.Interval = 20000;
    }
    else  // Retrn to normal
    {
      Ring.Reverse();
    }
}*/
