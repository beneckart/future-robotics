
/*
 * Hardware-Specific Program Constants (do not change!)
 */

#define NUM_NEOPIXELS 120

// LoRa stuff
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET (!!!!!)
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

// Note that the board has RST as 23? is this is a mistake?

// neopixel pins
#define LED_PIN1 14 // this is also LED_BUILTIN 
#define LED_PIN2 2

#define BUTTON_PIN 39
#define BATTERY_PIN 35 

#define OLED_PIN 16