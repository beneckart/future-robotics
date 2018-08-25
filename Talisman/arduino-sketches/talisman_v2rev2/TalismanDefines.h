#define VERSION 2.2

//UNCOMMENT OUT IF AT BM!!
#define PRODUCTION

#define DEVICE_ID 0 //later, replace this with chipid

#define NUM_TALISMAN 6

/*
 * Update Rates for Modes
 */
 
// delays
#define LED_MAP_UPDATE_DELAY_MS 250

#define BATTERY_CHECK_DELAY_MS 60*1000*5 // check battery every 5 minutes

#define DIAGNOSTIC_UPDATE_DELAY_MS 500

#define STALE_MAP_POINT_SEC 300 // 300 = 5 minutes until point drops from map

// how quick we allow the animations to be in party mode
#define LED_MIN_DELAY_MS 15

/*
 *  Thresholds and Stuff
 */

// low battery percentage
#define LOW_BATTERY -1

// led brightness
#define LED_BRIGHTNESS 4 // 0-255 (12=%5)
#define LED_BRIGHTNESS_FADE 32 // needs to be at least 32 for smoother fades

#define BUTTON_DEBOUNCE 40

/*
 * Main compile/debugging options
 */

#define LOGGING

//this is the minimum, it is dynamic based on speed/displacement
#define LOG_UPDATE_DELAY_MS 1000 

// if wanting to read gps log off serial line
//#define READ_LOG_THEN_QUIT 

#define LOG_DUMP_BAUD 115200 // too fast or just right?

#define DEBUG_OLED

#define OLED_UPDATE_DELAY_MS 500


/*
 * LoRa Constants 
 */

#define RECEIVE_ONLY 0 // for OLED trackers

// min time between transmissions
#define LORA_SEND_DELAY_MS 500

#define BAND    868E6 //
#define SPREADING_FACTOR 9
// 1-20, default in library is 17 db
// latest github version supports 20db (test this?) but max before that was 17db
#define TX_PWR 17 

//7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, and 250E3
//125E3 is library default, check and make sure 500e3 works (latest github version has this I think)
#define LORA_BW 250E3 

#define LORA_CODING 5 //between 5..8

#define LORA_PKT_SIZE 11

