/*
  setLED(6, LOW);

  Done:
  if(checkSwitch(3) == LOW)
  readButtons();
  sendFrame();

*/

const int DATA = 23; //S
const int CLOCK = 18; //C
const int LATCH = 26; //L
const int SWITCH = 36; //I

uint8_t ledState[5];
// Byte 0 - Byte 1 - Byte 2 ... Byte n
// 76543210
// LED87654321 LED16151413121110 LED2423222120191817

uint8_t switchControl = 0;
// 8 bits
// INPUT E0 to E4, S0 to S2

uint8_t switchState[5];
// Byte 0 - Byte 1 - Byte 2 ... Byte n
// 76543210
// SW87654321 LED16151413121110 LED2423222120191817

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void setup()
{
  Serial.begin(115200);
  delay(250);

  Serial.println("Hello!");

  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  digitalWrite(CLOCK, LOW);
  digitalWrite(LATCH, LOW);
  digitalWrite(DATA, LOW);

}

void loop()
{
  if (Serial.available())
  {
    byte incoming = Serial.read();
    if (incoming == 'r')
    {
      ESP.restart();
    }
    else if (incoming == 's')
    {
      unsigned long startTime = micros();
      sendFrame();
      unsigned long stopTime = micros(); //26us

      Serial.printf("Time for sendFrame: %d\r\n", stopTime - startTime);
    }
    else if (incoming == 'b')
    {
      unsigned long startTime = micros();
      readButtons();
      unsigned long stopTime = micros(); //687us

      Serial.printf("Time for readButtons: %d\r\n", stopTime - startTime);

      if (checkSwitch(3) == LOW)
        Serial.println("Button 3 low");
      else
        Serial.println("Button 3 high");
    }
    else if (incoming == 't')
    {
      switchControl = 0b00000001;

      //INPUT E0 is clocked out last
      for (int x = 0 ; x < 8 ; x++)
      {
        digitalWrite(CLOCK, LOW);
        digitalWrite(DATA, switchControl & (1 << (7 - x)));
        digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
      }
      digitalWrite(LATCH, LOW);
      digitalWrite(LATCH, HIGH); //Data is presented on the rising edge of RCK

      Serial.println("INPUT E0 should now be low, E1 should be high");
    }
    else if (incoming == 'w')
    {
      Serial.println("Checking switch 3");
      Serial.println("Run readButtons before this");

      if (checkSwitch(3) == LOW)
        Serial.println("Button 3 low");
      else
        Serial.println("Button 3 high");
    }
    else if (incoming == 'z')
    {
      delay(50);
      while (Serial.available()) Serial.read();

      Serial.println("Blinking");

      while (Serial.available() == 0)
      {
        digitalWrite(CLOCK, HIGH);
        digitalWrite(DATA, LOW);
        digitalWrite(LATCH, HIGH);
        delay(2000);
        digitalWrite(CLOCK, LOW);
        digitalWrite(DATA, HIGH);
        digitalWrite(LATCH, LOW);
        delay(2000);
      }
    }
    else if (incoming == '1')
    {
      delay(50);
      while (Serial.available()) Serial.read();

      Serial.println("Blinking");

      while (Serial.available() == 0)
      {
        setLED(1, HIGH);
        setLED(2, LOW);

        Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledState[4], ledState[3], ledState[2], ledState[1], ledState[0]);
        Serial.println();
        sendFrame();
        delay(2000);

        setLED(1, LOW);
        setLED(2, HIGH);

        Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledState[4], ledState[3], ledState[2], ledState[1], ledState[0]);
        Serial.println();
        sendFrame();
        delay(2000);
      }
    }
  }
}

//Set the correct bit within the LED array
void setLED(int ledNumber, int level)
{
  ledNumber--; //Map 1 to 40 LEDs to 0-39

  int registerNumber = ledNumber / 8;
  ledNumber %= 8;

  if (level == HIGH)
    ledState[registerNumber] |= (1 << ledNumber); //Set
  else
    ledState[registerNumber] &= (0xFF ^ (1 << ledNumber)); //Clear
}

//Return the value of a given bit in the switchState array
int checkSwitch(int switchNumber)
{
  int registerNumber = switchNumber / 8;
  switchNumber %= 8;
  return (switchState[registerNumber] & (1 << switchNumber));
}

//Repeatedly set the bits in switchControl to step through all 40 switches
//Load switch values into switchState array
void readButtons()
{
  for (int registerNumber = 0 ; registerNumber < 5 ; registerNumber++)
  {
    for (int channelSelect = 7 ; channelSelect > 0 ; channelSelect--)
    {
      //Select the register we need
      switchControl = (1 << registerNumber);

      //Drive the pin low to clear INPUT_S0
      //111 = 000 Y0
      //110 = 001 Y1
      //101 = 010 Y2
      //100 = 011 Y3
      switchControl |= (channelSelect << 5);

      sendFrame();

      //Load the value sitting on the SWITCH pin into the switchState
      switchState[registerNumber] |= digitalRead(SWITCH) << channelSelect;
    }
    //    Serial.printf("switchControl: 0b"BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(switchControl));
    //    Serial.println();
  }
}

//Clock out the switchControl and LED array to the sink drivers
void sendFrame()
{
  //Clock out the LED and control array out to the drivers

  //LED 40 is clocked out first
  for (int ledByteNumber = 4 ; ledByteNumber >= 0 ; ledByteNumber--)
  {
    for (int x = 0 ; x < 8 ; x++)
    {
      digitalWrite(CLOCK, LOW);
      digitalWrite(DATA, ledState[ledByteNumber] & (1 << (7 - x)));
      digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
    }
  }

  //INPUT E0 is clocked out last
  for (int x = 0 ; x < 8 ; x++)
  {
    digitalWrite(CLOCK, LOW);
    digitalWrite(DATA, switchControl & (1 << (7 - x)));
    digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
  }

  //Latch this data
  digitalWrite(LATCH, LOW);
  digitalWrite(LATCH, HIGH); //Data is presented on the rising edge of RCK
}
