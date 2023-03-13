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

  Hardware:

  The plugs are 1/4" and are inserted into a jack from the front. The jack has two paddles that are NC
  connected to ground. When inserted, the paddles are separated, and the system sees a high input.

  The LEDs are bussed with all 3.3V on the high side, and individually sunk through sink drivers. The
  LEDs are inserted into the front panel with long leg oriented towards the top of the hole.
  Reasoning: Long leg is on the round side, or negative.
*/

const int numberOfPlugs = 80;
const int numberOfLEDs = 80;

//Pins
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const int DATA = 13; //S
const int CLOCK = 16; //C
const int LATCH = 14; //L
const int SWITCH = 32; //I Avoid 36, 39, etc input only as they do not have pull-ups

//int pin_sdcs = 9;
int pin_sdcs = 5; //Onboard SD socket

int pin_mp3Reset = 26;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "settings.h"

//Preferences for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Preferences.h>
Preferences systemSettings;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

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

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
uint8_t ledStateA[5]; //The higher numbered LEDs 41-80
uint8_t ledStateB[5];
uint8_t ledStateAReality[5]; //The higher numbered LEDs 41-80
uint8_t ledStateBReality[5]; //Loader LEDs 1-40
uint8_t ledTransform[80 + 1]; //Extra because we call the LEDs '1' to '80'
// Byte 0 - Byte 1 - Byte 2 ... Byte n
// 76543210
// LED87654321 LED16151413121110 LED2423222120191817

int leaderLED = 1; //Used during the screen saver to control the front LED

uint8_t switchControlA = 0;
uint8_t switchControlB = 0;
// 8 bits
// INPUT E0 to E4, S0 to S2

uint8_t switchStateA[5]; //The higher numbered switches 41-80
uint8_t switchStateB[5];
uint8_t switchStateAReality[5]; //These are loaded after the transform is applied
uint8_t switchStateBReality[5];
uint8_t switchTransform[80 + 1];
//switchTransform[10] = 61 means that the plug labeled '10' on the front of the switchboard (the user number) is connected to input 61 (Board A, pin 21) on the electronics

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

int ledGap = 10; //Number of LEDs off between lit LEDs during screen saver

int lastPlug = 0; //Remember the switch we're playing so that we don't keep starting the MP3

unsigned long lastInteraction = 0; //Used to turn on screen saver when no interaction occurs

bool isPlaying = false; //Tracks when we start/stop a track

bool screenSaverRunning = false; //Cause LEDs to chase if no interactions are detected
unsigned long lastScreenSaverUpdate = 0; //Used to prevent the screen saver from running too fast
int ledDirection = 1;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup()
{
  Serial.begin(115200);
  delay(250);

  Serial.println("Switchboard MP3 Player");

  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  digitalWrite(CLOCK, LOW);
  digitalWrite(LATCH, LOW);
  digitalWrite(DATA, LOW);

  //Before anything, turn off LEDs
  ledOff(); //By default, the LEDs are activated. We don't want that much current.
  sendFrame(); //Push data to the LEDs

  systemSettings.begin("settings"); //Start the preferences library, working within a namespace called settings
  loadSettings();

  initSD(); //Initialize the SD card
  initMP3Player(); //Initialize the MP3 Shield

  loadRealityArrayLEDs(); //Get transform from NVM
  loadRealityArraySwitches(); //Get transform from NVM

  printMainMenu();

  //blinkTest();
  //switchTest();
}

void loop()
{
  updateBoard(); //If a plug has been inserted, update LEDs and play MP3s. Calls readSwitches().

  updateMP3(); //Feed MP3 IC with file data as needed

  if (Serial.available())
    menuMain();
}

//If no plugs are detected, run a screen saver
//If a plug is detected, find the highest plug,
//play that MP3, and light that LED
void updateBoard()
{
  //Get the newest plug to be inserted. Returns 0 if no plugs are detected.
  //Calls sendFrame() and readSwitches().
  int plugNumber = getPlugNumber(); 

  //If a plug is inserted, light LED and begin playing MP3
  if (plugNumber > 0 && lastPlug != plugNumber)
  {
    Serial.printf("Plug found, light LED %d\r\n", plugNumber);

    lastInteraction = millis();
    screenSaverRunning = false;

    //Record this interaction to NVM
    settings.interactions++;
    recordSettings();

    ledOff(); //Turn off all LEDs
    setLED(plugNumber, HIGH); //Turn on this LED

    lastPlug = plugNumber; //Remeber what we've started

    char trackName[sizeof("T001.mp3_plusExtra")];
    sprintf(trackName, "T%03d.MP3", plugNumber); //Splice the track number into file name

    Serial.print("Playing file: ");
    Serial.println(trackName);

    playTrack(trackName);
  }

  //If MP3 is finished playing, continue to light LED but start interaction timer
  //  else if(isPlaying == false)
  //  {
  //    Serial.println("MP3 is finished");
  //  }

  //If plug is removed, stop MP3 and turn off LED
  else if (plugNumber == 0 && lastPlug > 0)
  {
    lastInteraction = millis();
    screenSaverRunning = false;

    lastPlug = 0;
    ledOff(); //Turn off all LEDs
    stopTrack();
  }

  else if (millis() - lastInteraction > 10000 && screenSaverRunning == false)
  {
    screenSaverRunning = true;
    
    //Make screen saver start from last plug
    leaderLED = lastPlug;
    ledDirection = 1;

    lastInteraction = millis();
    Serial.println("Play screen saver");
  }

//  if (screenSaverRunning == true) ledScreenSaverLotsDots();
  if (screenSaverRunning == true) ledScreenSaverChase();
}
