int sensorPin = A5;    // select the input pin for the potentiometer
//int sensorValue = 0;  // variable to store the value coming from the sensor
int deltValue = 0;
int sensorVal0 = 0;

bool hugging = false;

int hugPwr = 0;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(LED_BUILTIN, OUTPUT);
  //Serial.begin(9600);
  pinMode(sensorPin, INPUT_PULLUP);

  pinMode(2, OUTPUT);  // channel A  
  pinMode(3, OUTPUT);  // channel B   
  pinMode(4, OUTPUT);  // channel C
  pinMode(5, OUTPUT);  // channel D    
  pinMode(6, OUTPUT);  // channel E
  pinMode(7, OUTPUT);  // channel F
  pinMode(8, OUTPUT);  // channel G
  pinMode(9, OUTPUT);  // channel H

  //delay(1000);
  sensorVal0 = analogRead(sensorPin)/2 + analogRead(sensorPin)/2; // baseline
}

void loop() {
  // read the value from the sensor:
  int x;
  int sensorValue = analogRead(sensorPin);

  deltValue = sensorVal0 - sensorValue; // goes down when touched

  if(deltValue > 0.45*sensorVal0)
  {
    hugging = true;
    hugPwr = 8;
  }
  else if(deltValue > 0.4*sensorVal0)
  {
    hugging = true;
    hugPwr = 7;
  }
  else if(deltValue > 0.35*sensorVal0)
  {
    hugging = true;
    hugPwr = 6;
  }
  else if(deltValue > 0.3*sensorVal0)
  {
    hugging = true;
    hugPwr = 5;
  }
  else if(deltValue > 0.25*sensorVal0)
  {
    hugging = true;
    hugPwr = 4;
  }
  else if(deltValue > 0.2*sensorVal0)
  {
    hugging = true;
    hugPwr = 3;
  }
  else if(deltValue > 0.15*sensorVal0)
  {
    hugging = true;
    hugPwr = 2;
  }
  else if(deltValue > 0.1*sensorVal0)
  {
    hugging = true;
    hugPwr = 1;
  }
  else
  {
    hugging = false;
    hugPwr = 0;
  }

  //if(deltValue > 0.25*sensorValue)
  //{
  //  hugging = false;
  //}

  if(sensorValue > 1000)
    hugging = false;

  if(hugging)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);

  for (x=2; x<=2+hugPwr; x++)
  {
    digitalWrite(x, HIGH);   // turn the EL channel on
  }
  for(x=2+hugPwr; x<=2+9; x++)
  {
    digitalWrite(x, LOW); 
  }
  
  
  // turn the ledPin on
  /*digitalWrite(ledPin, HIGH);
  // stop the program for <sensorValue> milliseconds:
  delay(sensorValue);
  // turn the ledPin off:
  digitalWrite(ledPin, LOW);
  // stop the program for for <sensorValue> milliseconds:
  delay(sensorValue);*/
  //Serial.print(sensorVal0); Serial.print(" "); Serial.println(sensorValue);
  //Serial.println(hugPwr);

  //delay(500);
}
