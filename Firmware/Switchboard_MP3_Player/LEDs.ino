//Turn off all leds
void ledOff()
{
  //Serial.println("LED off");

  for (int x = 0 ; x < 5 ; x++)
  {
    ledStateA[x] = 0;
    ledStateB[x] = 0;
    ledStateAReality[x] = 0;
    ledStateBReality[x] = 0;
  }
}

//Set the correct bit within the LED array
void setLED(int ledNumber, int level)
{
  //Serial.printf("setLed: %d\r\n", ledNumber);

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
      ledStateA[registerNumber - 5] |= (1 << ledNumber); //Set
    else
      ledStateA[registerNumber - 5] &= (0xFF ^ (1 << ledNumber)); //Clear
  }
}

//Set the correct bit within the LED Reality array
void setRealityLED(int ledNumber, int level)
{
  //Serial.printf("setRealityLED: %d\r\n", ledNumber);

  ledNumber--; //Map 1 to 80 LEDs to 0-79

  int registerNumber = ledNumber / 8;
  ledNumber %= 8;

  if (registerNumber < 5)
  {
    if (level == HIGH)
      ledStateBReality[registerNumber] |= (1 << ledNumber); //Set
    else
      ledStateBReality[registerNumber] &= (0xFF ^ (1 << ledNumber)); //Clear
  }
  else
  {
    if (level == HIGH)
      ledStateAReality[registerNumber - 5] |= (1 << ledNumber); //Set
    else
      ledStateAReality[registerNumber - 5] &= (0xFF ^ (1 << ledNumber)); //Clear
  }
}

//Return the value of a given LED number
bool getLED(int ledNumber)
{
  //Serial.printf("getLED: %d\r\n", ledNumber);

  ledNumber--; //Map 1 to 80 LEDs to 0-79

  int registerNumber = ledNumber / 8;
  ledNumber %= 8;

  if (registerNumber < 5)
  {
    if (ledStateB[registerNumber] & (1 << ledNumber)) return true;
    return false;
  }
  else
  {
    if (ledStateA[registerNumber - 5] & (1 << ledNumber)) return true;
    return false;
  }
}

//Chase one LED back and forth
void ledScreenSaverChase()
{
  const int ledsOnBoard = 60; //Play with LEDs 1 to 60
  if (millis() - lastScreenSaverUpdate > settings.screenSaverSpeed)
  {
    lastScreenSaverUpdate = millis();

    ledOff(); //Clear all LEDs
    setLED(leaderLED, HIGH);

    leaderLED += ledDirection;
    
    if (leaderLED == ledsOnBoard + 1) //Beyond last LED
    {
      leaderLED = ledsOnBoard;
      ledDirection = -1;
    }
    else if(leaderLED == 0)
    {
      leaderLED = 1;
      ledDirection = 1;
    }
  }
}

//Make the LEDs chase each other with a gap between them
//Start on LED 1 and proceed
//Keep calling this function to advance LEDs
void ledScreenSaverLotsDots()
{
  if (millis() - lastScreenSaverUpdate > settings.screenSaverSpeed)
  {
    lastScreenSaverUpdate = millis();

    int tempLeader = leaderLED;
    for (int x = numberOfLEDs ; x > 0 ; x--)
    {
      setLED(x, false);
      if (x == tempLeader) setLED(x, true);
      if (x == (tempLeader - (ledGap + 1)))
      {
        tempLeader = tempLeader - (ledGap + 1);
        setLED(x, true);
      }
    }

    leaderLED++;
    if (leaderLED == (numberOfLEDs + 1)) leaderLED -= (ledGap + 1);
  }
}
