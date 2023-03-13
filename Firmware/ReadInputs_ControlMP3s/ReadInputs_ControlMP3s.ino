/*
  Control 80 LEDs and 80 inputs through two boards
  If switch is closed, play associated MP3
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 22, 2023
  License: MIT. See license file for more information but you can basically do whatever you want with this code.

  We detect the insertion of cables into the various ports on an old telephone switch board. If a cable is inserted into
  port 1, T001.mp3 is played and the associated LED is lit up. The MP3 will continue to play until the cable is pulled out,
  or a higher number port is plugged.

  If no cables are plugged in, we play T000.mp3 on a loop and light up the LEDs in a chasing pattern.

  The board closest to the ESP32 is the primary board. The numbers on the primary board
  reflect reality (input 1 = input 1). On the 2nd board, the numbers are 1 + 40, i.e. input 1 is
  input 41.

  A array of 5 bytes can read the switches (switchState) and LEDs (ledState) from a given I/O board. 
  The closest board uses arrays 'B', the 2nd board uses arrays 'A'.
*/

//Pins
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const int DATA = 13; //S
const int CLOCK = 16; //C
const int LATCH = 14; //L
const int SWITCH = 32; //I Avoid 36, 39, etc input only as they do not have pull-ups

//int pin_sdcs = 9;
int pin_sdcs = 5; //Onboard SD socket
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//sdFat for SD card
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SdFat.h>         // SDFat Library
SdFat sd; // Create object to handle SD functions

SdFile trackFile; //Global variable pointing at a given track
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//MP3
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <VS1053.h> //https://github.com/baldram/ESP_VS1053_Library

//You must cut the GPIO and MIDI traces on the shield
#define VS1053_CS     17
#define VS1053_DCS    25
#define VS1053_DREQ   4

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

int previousTrack1 = 1; //Used to prevent the playing of sounds twice in a row
int previousTrack2 = 2;

boolean trackStarted = false; //Goes true when track has been started
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Preferences for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Preferences.h>
Preferences systemSettings;

#include "settings.h"
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
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

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println("Switchboard MP3 Player");

  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  digitalWrite(CLOCK, LOW);
  digitalWrite(LATCH, LOW);
  digitalWrite(DATA, LOW);

  loadSettings(); //Load settings from NVM

  Serial.printf("mp3Volume: %d\r\n", settings.mp3Volume);

  initSD(); //Initialize the SD card
  initMP3Player(); //Initialize the MP3 Shield
}

void loop()
{
  //Read the inputs from all plugs. This also does a sendFrame() which will send/update all the LED states.
  //Therefore we do not call sendFrame() here.
  //readSwitches(); 

  //updateBoard(); //If a plug has been inserted, update LEDs and play MP3s

  updateMP3(); //Feed MP3 IC with file data as needed

  if (Serial.available())
    menuMain();
}

const int numberOfPlugs = 80;

//If no plugs are detected, run a screen saver
//If a plug is detected, find the highest plug,
//play that MP3, and light that LED
void updateBoard()
{
  //Check all the switches
  for (int x = numberOfPlugs ; x > 0 ; x--)
  {
    if (checkSwitch(x) == true)
    {
      Serial.printf("Plug found, light LED %d\r\n", x, x);

      ledOff(); //Turn off all LEDs
      setLED(x, HIGH); //Turn on this LED

      char trackName[13];
      sprintf(trackName, "T%03d.MP3", x); //Splice the track number into file name

      Serial.print("Playing: ");
      Serial.println(trackName);

      playTrack(trackName);
    }
  }
}
