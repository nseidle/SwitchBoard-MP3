/*
  Control 80 LEDs and 80 inputs through two boards
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 22, 2023
  
  License: MIT. See license file for more information but you can basically do whatever you want with this code.

  The board closest to the ESP32 is the primary board. The numbers on the primary board
  reflect reality (input 1 = input 1). On the 2nd board, the numbers are 1 + 40. Input 1 is
  input 41.
*/

const int DATA = 4; //S
const int CLOCK = 16; //C
const int LATCH = 14; //L
const int SWITCH = 32; //I Avoid 36, 39, etc input only as they do not have pull-ups

uint8_t ledStateA[5];
uint8_t ledStateB[5];
// Byte 0 - Byte 1 - Byte 2 ... Byte n
// 76543210
// LED87654321 LED16151413121110 LED2423222120191817


uint8_t switchControlA = 0;
uint8_t switchControlB = 0;
// 8 bits
// INPUT E0 to E4, S0 to S2

uint8_t switchStateA[5];
uint8_t switchStateB[5];
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

  //    switchControlA = 0b11100001; //Select channel Y0, on register 0
  //    sendFrame();
  //
  //    delay(50);
  //    while (Serial.available()) Serial.read();
  //
  //    while (Serial.available() == 0)
  //    {
  //      int value = analogRead(SWITCH);
  //      Serial.printf("switch: %d\n\r", value);
  //      //    if (digitalRead(SWITCH) == HIGH)
  //      //      Serial.println("High");
  //      //    else
  //      //      Serial.println("Low");
  //      delay(500);
  //    }

  //  delay(50);
  //  while (Serial.available()) Serial.read();
  //
  //  while (Serial.available() == 0)
  //  {
  //    readSwitches();
  //
  //    for(int x = 1 ; x <= 40 ; x++)
  //    {
  //      if(checkSwitch(x) == false)
  //      {
  //        Serial.printf("switch %d\n\r", x);
  //      }
  //    }
  //
  //    delay(100);
  //  }

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
      readSwitches();
      unsigned long stopTime = micros(); //687us

      Serial.printf("Time for readSwitches: %d\r\n", stopTime - startTime);

      if (checkSwitch(3) == LOW)
        Serial.println("Button 3 low");
      else
        Serial.println("Button 3 high");
    }
    else if (incoming == 'w')
    {
      Serial.println("Checking switches");

      delay(50);
      while (Serial.available()) Serial.read();

      while (Serial.available() == 0)
      {
        readSwitches();

        for (int x = 1 ; x <= 80 ; x++)
        {
          if (checkSwitch(x) == false)
            Serial.printf("Switch %d\n\r", x);
        }

        Serial.printf("switchState Array: 0x%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                      switchStateA[4], switchStateA[3], switchStateA[2], switchStateA[1], switchStateA[0],
                      switchStateB[4], switchStateB[3], switchStateB[2], switchStateB[1], switchStateB[0]
                     );
        Serial.println();

        delay(100);
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

        Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0]);
        Serial.println();
        sendFrame();
        delay(2000);

        setLED(1, LOW);
        setLED(2, HIGH);

        Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0]);
        Serial.println();
        sendFrame();
        delay(2000);
      }
    }

#define LED_COUNT 80

    else if (incoming == '2')
    {
      Serial.println("LED display 1");

      delay(50);
      while (Serial.available()) Serial.read();

      //Pick random number between 1-5
      randomSeed(millis());
      int startingLED = random(1, 6); //Inclusive, exclusive

      //Clear the LED array
      for (int x = 0 ; x < 5 ; x++)
        ledStateA[x] = 0;

      //      if (startingLED == 1)
      //      {
      //        for (int ledByte = 0 ; ledByte < 5 ; ledByte++)
      //        {
      //          for(int ledBit = 0 ; ledBit < 8 ; ledBit++)
      //          {
      //            count++;
      //
      //            if(count == startingLED)
      //            {
      //              count = 0;
      //              ledStateA[ledByte] |= (1<<ledBit);
      //            }
      //          }
      //        }
      //      }

      while (Serial.available() == 0)
      {
        //80 LEDs
        //Every 5 LEDs
        //Set then advance
        setLED(1, HIGH);
        setLED(2, LOW);

        Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0]);
        Serial.println();
        sendFrame();
        delay(2000);

        setLED(1, LOW);
        setLED(2, HIGH);

        Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0]);
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
  ledNumber--; //Map 1 to 80 LEDs to 0-79

  int registerNumber = ledNumber / 8;
  ledNumber %= 8;

  if (registerNumber < 5)
  {
    if (level == HIGH)
      ledStateB[registerNumber] |= (1 << ledNumber); //Set
    else
      ledStateB[registerNumber] &= (0xFF ^ (1 << ledNumber)); //Clear
  }
  else
  {
    if (level == HIGH)
      ledStateA[registerNumber] |= (1 << ledNumber); //Set
    else
      ledStateA[registerNumber] &= (0xFF ^ (1 << ledNumber)); //Clear
  }
}

//Return the value of a given bit in the switchState array
int checkSwitch(int switchNumber)
{
  switchNumber--; //Align 1-80 to 0-79

  int registerNumber = switchNumber / 8;
  switchNumber %= 8;

  if (registerNumber < 5)
    return (switchStateB[registerNumber] & (1 << switchNumber));

  return (switchStateA[registerNumber - 5] & (1 << switchNumber));
}

//Repeatedly set the bits in switchControlA to step through all 40 switches
//Load switch values into switchState array
//First we check the switches on the A board (the 2nd board)
//Then we check the switches on the B board (the 1st board)
void readSwitches()
{
  for (int registerNumber = 0 ; registerNumber < 5 ; registerNumber++)
  {
    switchStateA[registerNumber] = 0; //Reset

    for (int channelSelect = 7 ; channelSelect >= 0 ; channelSelect--)
    {
      //Select the register we need
      switchControlA = (1 << registerNumber);

      //Drive the pin low to clear INPUT_S0
      //111 = 000 Y0
      //110 = 001 Y1
      //...
      //000 = 111 Y7
      switchControlA |= (channelSelect << 5);

      switchControlB = 0; //While we check A, leave B alone
      sendFrame();

      delayMicroseconds(10); //Allow switch and signal to propogate

      //Load the value sitting on the SWITCH pin into the switchState
      if (digitalRead(SWITCH) == HIGH)
        switchStateA[registerNumber] |= (1 << (7 - channelSelect));
    }

    //    Serial.printf("switchState: 0b"BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(switchState[registerNumber]));
    //    Serial.println();
  }

  //Now we check 1st board
  for (int registerNumber = 0 ; registerNumber < 5 ; registerNumber++)
  {
    switchStateB[registerNumber] = 0; //Reset

    for (int channelSelect = 7 ; channelSelect >= 0 ; channelSelect--)
    {
      //Select the register we need
      switchControlB = (1 << registerNumber);

      //Drive the pin low to clear INPUT_S0
      //111 = 000 Y0
      //110 = 001 Y1
      //...
      //000 = 111 Y7
      switchControlB |= (channelSelect << 5);

      switchControlA = 0; //While we check B, leave A alone
      sendFrame();

      delayMicroseconds(10); //Allow switch and signal to propogate

      //Load the value sitting on the SWITCH pin into the switchState
      if (digitalRead(SWITCH) == HIGH)
        switchStateB[registerNumber] |= (1 << (7 - channelSelect));
    }

    //    Serial.printf("switchState: 0b"BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(switchState[registerNumber]));
    //    Serial.println();
  }

}

//Clock out the switchControl and LED array to the sink drivers
//For a dual setup, first A is sent to reach the 2nd board, then B for the first board.
void sendFrame()
{
  //Clock out the LED and control array out to the drivers

  //LED 40 is clocked out first
  for (int ledByteNumber = 4 ; ledByteNumber >= 0 ; ledByteNumber--)
  {
    for (int x = 0 ; x < 8 ; x++)
    {
      digitalWrite(CLOCK, LOW);
      digitalWrite(DATA, ledStateA[ledByteNumber] & (1 << (7 - x)));
      digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
    }
  }

  //INPUT E0 is clocked out last
  for (int x = 0 ; x < 8 ; x++)
  {
    digitalWrite(CLOCK, LOW);
    digitalWrite(DATA, switchControlA & (1 << (7 - x)));
    digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
  }

  //LED 40 is clocked out first
  for (int ledByteNumber = 4 ; ledByteNumber >= 0 ; ledByteNumber--)
  {
    for (int x = 0 ; x < 8 ; x++)
    {
      digitalWrite(CLOCK, LOW);
      digitalWrite(DATA, ledStateB[ledByteNumber] & (1 << (7 - x)));
      digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
    }
  }

  //INPUT E0 is clocked out last
  for (int x = 0 ; x < 8 ; x++)
  {
    digitalWrite(CLOCK, LOW);
    digitalWrite(DATA, switchControlB & (1 << (7 - x)));
    digitalWrite(CLOCK, HIGH); //Data transfers to the register on the rising edge of SRCK
  }

  //Latch this data
  digitalWrite(LATCH, LOW);
  digitalWrite(LATCH, HIGH); //Data is presented on the rising edge of RCK
}
