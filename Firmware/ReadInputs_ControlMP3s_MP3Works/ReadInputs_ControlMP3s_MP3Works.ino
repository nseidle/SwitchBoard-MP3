/*
  Control 80 LEDs and 80 inputs through two boards
  If switch is closed, play associated MP3
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 22, 2023
  License: MIT. See license file for more information but you can basically do whatever you want with this code.

  We detect the insertion of cables into the various ports on an old telephone switch board. If a cable is inserted into
  port 1, 1.mp3 is played and the associated LED is lit up. The MP3 will continue to play until the cable is pulled out,
  or a higher number port is plugged.

  If no cables are plugged in, we play 0.mp3 on a loop and light up the LEDs in a chasing pattern.

  The board closest to the ESP32 is the primary board. The numbers on the primary board
  reflect reality (input 1 = input 1). On the 2nd board, the numbers are 1 + 40. Input 1 is
  input 41.
*/

//Pins
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const int DATA = 4; //S
const int CLOCK = 16; //C
const int LATCH = 14; //L
const int SWITCH = 32; //I Avoid 36, 39, etc input only as they do not have pull-ups

//int pin_sdcs = 9;
int pin_sdcs = 5; //Onboard socket
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

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

//#include <avr/wdt.h> //We need watch dog for this program

//sdFat for SD card
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SdFat.h>         // SDFat Library
SdFat sd; // Create object to handle SD functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//MP3
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <VS1053.h> //https://github.com/baldram/ESP_VS1053_Library

//You must cut the GPIO and MIDI traces on the shield
#define VS1053_CS     17
#define VS1053_DCS    25
#define VS1053_DREQ   4

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

#define VOLUME  90 // volume level 0-100

int previousTrack1 = 1; //Used to prevent the playing of sounds twice in a row
int previousTrack2 = 2;

boolean playerStopped = false; //Goes true when track has ended and we need to play SILENCE.MP3

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SdFile trackFile;

void setup()
{
  Serial.begin(115200);
  delay(250);

  Serial.println("Hello!");

  SPI.begin();

  //  pinMode(CLOCK, OUTPUT);
  //  pinMode(LATCH, OUTPUT);
  //  pinMode(DATA, OUTPUT);
  //  pinMode(SWITCH, INPUT_PULLUP);

  pinMode(26, OUTPUT);
  digitalWrite(26, HIGH);

  //  digitalWrite(CLOCK, LOW);
  //  digitalWrite(LATCH, LOW);
  //  digitalWrite(DATA, LOW);

  initSD();  // Initialize the SD card
  //  initMP3Player(); // Initialize the MP3 Shield

  //char trackName[40] = "TRACK001.WAV";
  char trackName[40] = "T001.MP3";
  if (trackFile.open(trackName, O_READ) == false)
  {
    Serial.println("Failed to open file");
    while (1);
  }


  player.begin();
  player.switchToMp3Mode(); // optional, some boards require this
  player.setVolume(VOLUME);




  //printMenu();

  //playRandomTrack();

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

  player.startSong();
}

void loop()
{
  if (player.data_request() == true)
  {
    uint8_t mp3DataBuffer[32];

    // Go out to SD card and try reading some number of bytes
    int bytesToPlay = trackFile.read(mp3DataBuffer, sizeof(mp3DataBuffer));
    if (bytesToPlay <= 0)
    {
      Serial.println("Stop file");

      //No data left in file
      player.stopSong();
      trackFile.close(); // Close out this track
    }
    else
    {
      Serial.printf("Play %d\r\n", bytesToPlay);
      player.playChunk(mp3DataBuffer, bytesToPlay);
    }
  }

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
        unsigned long startTime = micros();

        readSwitches();
        unsigned long stopTime = micros(); //26us

        Serial.printf("Time for readSwitches: %d\r\n", stopTime - startTime);

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

      //Make the LEDs chase each other with a gap between them
      //Start on LED 1 and proceed

#define LED_GAP 5 //Number of dark LEDs between lit LEDs

      int shifts = 0;

      delay(50);
      while (Serial.available()) Serial.read();

      int startingLED = 1;
      int toLight = 1;
      while (Serial.available() == 0)
      {
        //Clear the LED arrays
        for (int x = 0 ; x < 5 ; x++)
        {
          ledStateA[x] = 0;
          ledStateB[x] = 0;
        }

        //Set the appropriate LEDs
        int litLEDs = 0;
        for (int x = 0 ; x < LED_COUNT ; x++)
        {
          if ( (x % startingLED) == 0)
          {
            setLED(x, HIGH);

            litLEDs++;
            if (litLEDs == toLight) break;
          }
        }

        //Move to next starting spot
        startingLED++;
        startingLED %= LED_GAP; //Wrap

        //Make sure we don't get too large
        toLight++;
        toLight %= LED_COUNT;

        //Display output
        Serial.printf("LED Array: ");

        printBinary(ledStateB[4]);
        printBinary(ledStateB[3]);
        printBinary(ledStateB[2]);
        printBinary(ledStateB[1]);
        printBinary(ledStateB[0]);

        Serial.println();

        delay(2000);

        //        for (int ledByte = 0 ; ledByte < 5 ; ledByte++)
        //        {
        //          //Check if the highest bit is set, then move to next byte
        //          if (ledStateA[ledByte] & 0b10000000)
        //          {
        //            if (ledBytes < 5)
        //              ledStatA[ledByte + 1] |= 0b1; //Move bit to next byte up
        //          }
        //
        //          //Shift this byte by one
        //          shifts++;
        //          ledStateA[ledByte] <<= 1;
        //
        //          //Add a bit to the starting byte when needed
        //          if (ledByte == 0 && shifts == LED_GAP)
        //          {
        //            shifts = 0;
        //            ledStateA[0] |= 0b1;
        //          }
        //        }
      }

    }
  }
}
