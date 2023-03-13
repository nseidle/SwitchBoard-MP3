void printMainMenu()
{
  Serial.println("Switchboard MP3 Player");
  Serial.printf("Number of interactions to date: %d\r\n", settings.interactions);
  Serial.println("s) Print switch transform");
  Serial.println("l) Print LED transform");
  Serial.println("p) Play random track");
  Serial.println("w) Check switches");
  Serial.println("b) Blink LEDs");
  Serial.printf("v) Set MP3 volume: %d\r\n", settings.mp3Volume);
  Serial.printf("t) Set screen saver speed: %d\r\n", settings.screenSaverSpeed);
  Serial.println("e) Erase NVM - reset all settings to default");
  Serial.println("r) Reset");
}

void menuMain()
{
  printMainMenu();

  byte incoming = Serial.read();
  if (incoming == 'r')
  {
    ESP.restart();
  }
  else if (incoming == 's')
  {
    printSwitchTransformArray();
  }
  else if (incoming == 'l')
  {
    printLEDTransformArray();
  }

  else if (incoming == 'p')
  {
    playRandomTrack(); //Make some noise so a human knows we are alive
  }
  else if (incoming == 'w')
  {
    switchTest();
  }
  else if (incoming == 'b')
  {
    blinkTest();
  }

  else if (incoming == 'v')
  {
    Serial.print("Enter MP3 volume (1 to 100): ");
    int volume = getNumber(); //Returns EXIT, TIMEOUT, or long
    if ((volume != INPUT_RESPONSE_GETNUMBER_EXIT) && (volume != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
    {
      if (volume < 1 || volume > 100)
        Serial.println("Error: Volume out of range");
      else
      {
        settings.mp3Volume = volume;
        recordSettings();
      }
    }
  }
  else if (incoming == 't')
  {
    Serial.print("Enter screenSaverSpeed in ms (100 is good): ");
    int screenSaverSpeed = getNumber(); //Returns EXIT, TIMEOUT, or long
    if ((screenSaverSpeed != INPUT_RESPONSE_GETNUMBER_EXIT) && (screenSaverSpeed != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
    {
      if (screenSaverSpeed < 1 || screenSaverSpeed > 10000)
        Serial.println("Error: screenSaverSpeed out of range");
      else
      {
        settings.screenSaverSpeed = screenSaverSpeed;
        recordSettings();
      }
    }
  }
  else if (incoming == 'e')
  {
    Serial.print("Are you sure you want to erase everything? ('5' to continue): ");
    int choice = getNumber(); //Returns EXIT, TIMEOUT, or long
    if ((choice != INPUT_RESPONSE_GETNUMBER_EXIT) && (choice != INPUT_RESPONSE_GETNUMBER_TIMEOUT))
    {
      if (choice == 5)
      {
        systemSettings.clear(); //Erase all NVM settings
        Serial.println("Settings erased.");
        
        Settings defaultSettings;
        settings = defaultSettings;
        recordSettings();
        Serial.println("Reset to default.");
      }
      else
      {
        Serial.println("Aborted!");
      }
    }
  }
}

//Chase the LEDs until a button is pressed
void blinkTest()
{
  clearSerialBuffer();

  loadRealityArrayLEDs(); //Get transform from NVM
  loadRealityArraySwitches(); //Get transform from NVM

  Serial.println("Blinking. Press any key to stop.");

  while (Serial.available() == 0)
  {
    for (int x = 60 ; x >= 1 ; x--)
    {
      ledOff(); //Clear all LEDs
      setLED(x, HIGH);

      Serial.printf("ledState: (A) %02X %02X %02X %02X %02X (B) %02X %02X %02X %02X %02X ",
                    ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0],
                    ledStateB[4], ledStateB[3], ledStateB[2], ledStateB[1], ledStateB[0]
                   );

      Serial.printf("ledStateReality: (A) %02X %02X %02X %02X %02X (B) %02X %02X %02X %02X %02X",
                    ledStateAReality[4], ledStateAReality[3], ledStateAReality[2], ledStateAReality[1], ledStateAReality[0],
                    ledStateBReality[4], ledStateBReality[3], ledStateBReality[2], ledStateBReality[1], ledStateBReality[0]
                   );

      Serial.println();

      sendFrame();
      delay(50);
    }
  }
}
void switchTest()
{
  clearSerialBuffer();

  Serial.println("Checking switches");

  while (Serial.available() == 0)
  {
    unsigned long startTime = micros();
    readSwitches();
    unsigned long stopTime = micros(); //26us, 3947us with 2nd board and full LED load

    //Serial.printf("Time for readSwitches: %d\r\n", stopTime - startTime);

    for (int x = 1 ; x <= 60 + 7 + 7 ; x++) //Include the lower area as well
    {
      if (isPlugInserted(x) == true)
        Serial.printf("Switch %d\n\r", x);
    }

        Serial.printf("switchStateReality: (A) %02X %02X %02X %02X %02X (B) %02X %02X %02X %02X %02X ",
                      switchStateAReality[4], switchStateAReality[3], switchStateAReality[2], switchStateAReality[1], switchStateAReality[0],
                      switchStateBReality[4], switchStateBReality[3], switchStateBReality[2], switchStateBReality[1], switchStateBReality[0]
                     );
    
        Serial.printf("switchState: (A) %02X %02X %02X %02X %02X (B) %02X %02X %02X %02X %02X",
                      switchStateA[4], switchStateA[3], switchStateA[2], switchStateA[1], switchStateA[0],
                      switchStateB[4], switchStateB[3], switchStateB[2], switchStateB[1], switchStateB[0]
                     );
    
        Serial.println();

    delay(100);
  }
}
