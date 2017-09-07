int sensorPinLeft = A0;
int sensorPinMid = A3;    // select the input pin for the potentiometer
int sensorPinRight = A5;
int sensorValueLeft = 0;
int sensorValueRight = 0;
int sensorValueMid = 0;
//int sensorValue = 0;  // variable to store the value coming from the sensor
int deltValueLeft = 0;
int deltValueMid = 0;
int deltValueRight = 0;
int sensorVal0Left = 0;
int sensorVal0Right = 0;
int sensorVal0Mid = 0;

bool huggingLeft = false;
bool huggingRight = false;
bool huggingMid = false;

int hugPwrLeft = 0;
int hugPwrMid = 0;
int hugPwrRight = 0;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(LED_BUILTIN, OUTPUT);
  //Serial.begin(9600);
  pinMode(sensorPinLeft, INPUT_PULLUP);
  pinMode(sensorPinMid, INPUT_PULLUP);
  pinMode(sensorPinRight, INPUT_PULLUP);

  pinMode(2, OUTPUT);  // channel A  
  pinMode(3, OUTPUT);  // channel B   
  pinMode(4, OUTPUT);  // channel C
  pinMode(5, OUTPUT);  // channel D    
  pinMode(6, OUTPUT);  // channel E
  pinMode(7, OUTPUT);  // channel F
  pinMode(8, OUTPUT);  // channel G
  pinMode(9, OUTPUT);  // channel H

  delay(1000);
  sensorVal0Left = analogRead(sensorPinLeft)/2 + analogRead(sensorPinLeft)/2; // baseline
  delay(1000);
  sensorVal0Right = analogRead(sensorPinRight)/2 + analogRead(sensorPinRight)/2; // baseline
  delay(1000);
  sensorVal0Mid = analogRead(sensorPinMid)/2 + analogRead(sensorPinMid)/2; // baseline
  delay(1000);
}

void loop() {
  // read the value from the sensor:
  int x;
  sensorValueLeft = analogRead(sensorPinLeft);

  deltValueLeft = sensorVal0Left - sensorValueLeft; // goes down when touched

  if(deltValueLeft > 0.45*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 8;
  }
  else if(deltValueLeft > 0.4*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 7;
  }
  else if(deltValueLeft > 0.35*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 6;
  }
  else if(deltValueLeft > 0.3*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 5;
  }
  else if(deltValueLeft > 0.25*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 4;
  }
  else if(deltValueLeft > 0.2*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 3;
  }
  else if(deltValueLeft > 0.15*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 2;
  }
  else if(deltValueLeft > 0.1*sensorVal0Left)
  {
    huggingLeft = true;
    hugPwrLeft = 1;
  }
  else
  {
    huggingLeft = false;
    hugPwrLeft = 0;
  }

  sensorValueRight = analogRead(sensorPinRight);

  deltValueRight = sensorVal0Right - sensorValueRight; // goes down when touched

  if(deltValueRight > 0.45*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 8;
  }
  else if(deltValueRight > 0.4*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 7;
  }
  else if(deltValueRight > 0.35*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 6;
  }
  else if(deltValueRight > 0.3*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 5;
  }
  else if(deltValueRight > 0.25*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 4;
  }
  else if(deltValueRight > 0.2*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 3;
  }
  else if(deltValueRight > 0.15*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 2;
  }
  else if(deltValueRight > 0.1*sensorVal0Right)
  {
    huggingRight = true;
    hugPwrRight = 1;
  }
  else
  {
    huggingRight = false;
    hugPwrRight = 0;
  }

  sensorValueMid = analogRead(sensorPinMid);

  deltValueMid = sensorVal0Mid - sensorValueMid; // goes down when touched

  if(deltValueMid > 0.45*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 8;
  }
  else if(deltValueMid > 0.4*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 7;
  }
  else if(deltValueMid > 0.35*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 6;
  }
  else if(deltValueMid > 0.3*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 5;
  }
  else if(deltValueMid > 0.25*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 4;
  }
  else if(deltValueMid > 0.2*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 3;
  }
  else if(deltValueMid > 0.15*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 2;
  }
  else if(deltValueMid > 0.1*sensorVal0Mid)
  {
    huggingMid = true;
    hugPwrMid = 1;
  }
  else
  {
    huggingMid = false;
    hugPwrMid = 0;
  }

  //if(deltValue > 0.25*sensorValue)
  //{
  //  hugging = false;
  //}

  //if(sensorValue > 1000)
  //  hugging = false;

  if(huggingLeft || huggingRight || huggingMid)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);

  //for (x=2; x<=2+hugPwr; x++)
  //{
  //  digitalWrite(x, HIGH);   // turn the EL channel on
  //}
  //for(x=2+hugPwr; x<=2+9; x++)
  //{
  //  digitalWrite(x, LOW); 
  //}

  if(hugPwrLeft >= 4)
    digitalWrite(2, HIGH);
  else
    digitalWrite(2, LOW);

  if(hugPwrLeft >= 7)
    digitalWrite(3, HIGH);
  else
    digitalWrite(3, LOW);

  //

  if(hugPwrMid >= 2)
    digitalWrite(4, HIGH);
  else
    digitalWrite(4, LOW);

  if(hugPwrMid >= 4)
    digitalWrite(5, HIGH);
  else
    digitalWrite(5, LOW);

  if(hugPwrMid >= 6)
    digitalWrite(6, HIGH);
  else
    digitalWrite(6, LOW);

  if(hugPwrMid >= 8)
    digitalWrite(7, HIGH);
  else
    digitalWrite(7, LOW);  

  //

  if(hugPwrRight >= 4)
    digitalWrite(8, HIGH);
  else
    digitalWrite(8, LOW);

  if(hugPwrRight >= 7)
    digitalWrite(9, HIGH);
  else
    digitalWrite(9, LOW);
  
  
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
