#include <WiFi.h>
#include <NeoPixelBus.h>

//const char* ssid = "FutureRobots";
//const char* password = "benbenben";
const char* ssid = "Bud Hole";
const char* password = "allonewordnocaps";
//const char* ssid = "camp2";
//const char* password = "ForTheKids";

//MDNSResponder mdns;
// Actual name will be "espopc.local"
//const char myDNSName[] = "espopc";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(7890);

#define OSCDEBUG  1

const int PixelCount = 120;

#define LED_PIN1 2
#define LED_PIN2 14

NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0800KbpsMethod> leds1(PixelCount, LED_PIN1);
NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> leds2(PixelCount, LED_PIN2);

// Gamma correction 2.2 look up table
uint8_t GammaLUT[256];

void fillGammaLUT(float gamma)
{
  int i;

  for (i = 0; i < 256; i++) {
    float intensity = (float)i / 255.0;
    GammaLUT[i] = (uint8_t)(pow(intensity, gamma) * 255.0);
    //Serial.printf("GammaLUT[%d] = %u\r\n", i, GammaLUT[i]);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // this resets all the neopixels to an off state
  leds1.Begin(); leds2.Begin();
  leds1.Show(); leds2.Show();

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Set up mDNS responder:
 /* if (!mdns.begin(myDNSName, WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
  }
  else {
    Serial.println("mDNS responder started");
    Serial.printf("My name is [%s]\r\n", myDNSName);
  }*/

  // Start the server listening for incoming client connections
  server.begin();
  Serial.println("Server listening on port 7890");

  fillGammaLUT(2.2);
}

WiFiClient client;

#define minsize(x,y) (((x)<(y))?(x):(y))

void clientEvent()
{
  static int packetParse = 0;
  static uint8_t pktChannel, pktCommand;
  static uint16_t pktLength, pktLengthAdjusted, bytesIn;
  //static uint8_t pktData[PixelCount*3];
  static uint8_t pktData[1500*3];
  uint16_t bytesRead;
  size_t frame_count = 0, frame_discard = 0;

  if (!client) {
    // Check if a client has connected
    client = server.available();
    if (!client) {
      return;
    }
    Serial.println("new OPC client");
  }

  if (!client.connected()) {
    Serial.println("OPC client disconnected");
    client = server.available();
    if (!client) {
      return;
    }
  }

  //should this be while client.connected()? and then if .available()?
  while (client.available()) {
    switch (packetParse) {
      case 0: // Get pktChannel
        pktChannel = client.read();
        packetParse++;
#if OSCDEBUG
        Serial.printf("pktChannel %u\r\n", pktChannel);
#endif
        break;
      case 1: // Get pktCommand
        pktCommand = client.read();
        packetParse++;
#if OSCDEBUG
        Serial.printf("pktCommand %u\r\n", pktCommand);
#endif
        break;
      case 2: // Get pktLength (high byte)
        pktLength = client.read() << 8;
        packetParse++;
#if OSCDEBUG
        Serial.printf("pktLength high byte %u\r\n", pktLength);
#endif
        break;
      case 3: // Get pktLength (low byte)
        pktLength = pktLength | client.read();
        packetParse++;
        bytesIn = 0;
#if OSCDEBUG
        Serial.printf("pktLength %u\r\n", pktLength);
#endif
        if (pktLength > sizeof(pktData)) {
          Serial.println("Packet length exceeds size of buffer! Data discarded");
          pktLengthAdjusted = sizeof(pktData);
        }
        else {
          pktLengthAdjusted = pktLength;
        }
        break;
      case 4: // Read pktLengthAdjusted bytes into pktData
        bytesRead = client.read(&pktData[bytesIn],
            minsize(sizeof(pktData), pktLengthAdjusted) - bytesIn);
        bytesIn += bytesRead;
        if (bytesIn >= pktLengthAdjusted) {
          if ((pktCommand == 0) && (pktChannel <= 2)) {
            int i;
            uint8_t *pixrgb;
            pixrgb = pktData;
            for (i = 0; i < minsize((pktLengthAdjusted / 3), PixelCount); i++) {
              
              if(pktChannel==1)
              {
                leds1.SetPixelColor(i,
                  RgbColor(GammaLUT[*pixrgb++],
                           GammaLUT[*pixrgb++],
                           GammaLUT[*pixrgb++]));
              }
              if(pktChannel==2)
              {
                leds2.SetPixelColor(i,
                  RgbColor(GammaLUT[*pixrgb++],
                           GammaLUT[*pixrgb++],
                           GammaLUT[*pixrgb++]));
              }               
            }
            // Display only the first frame in this cycle. Buffered frames
            // are discarded.
            if (frame_count == 0) {
#if OSCDEBUG
              Serial.print("=");
              unsigned long startMicros = micros();
#endif
              leds2.Show(); leds2.Show();
#if OSCDEBUG
              Serial.printf("%lu\r\n", micros() - startMicros);
#endif
            }
            else {
              frame_discard++;
            }
            frame_count++;
          }
          if (pktLength == pktLengthAdjusted)
            packetParse = 0;
          else
            packetParse++;
        }
        break;
      default:  // Discard data that does not fit in pktData
        bytesRead = client.read(pktData, pktLength - bytesIn);
        bytesIn += bytesRead;
        if (bytesIn >= pktLength) {
          packetParse = 0;
        }
        break;
    }
  }
#if OSCDEBUG
  if (frame_discard) {
    Serial.printf("discard %u\r\n", frame_discard);
  }
#endif
}

void loop() {
  clientEvent();
}
