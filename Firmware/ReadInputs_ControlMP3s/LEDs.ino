int ledShifts = 0; //Used in ledScreenSaver1

//Make the LEDs chase each other with a gap between them
//Start on LED 1 and proceed
//Keep calling this function to advance LEDs
void ledScreenSaver1()
{
  const int ledGap = 5; //Number of dark LEDs between lit LEDs

  //Shift the lower 5 bytes of LEDs to the left.
  for (int ledByte = 0 ; ledByte < 5 ; ledByte++)
  {
    //Check if the highest bit is set, then move to next byte
    if (ledStateA[ledByte] & 0b10000000)
    {
      if (ledByte < 4)
        ledStateA[ledByte + 1] |= 0b1; //Move bit to next byte up
      else
        ledStateB[0] |= 0b1; //Move bit to next board
    }

    //Shift this byte by one
    ledStateA[ledByte] <<= 1;
  }

  //Shift the upper 5 bytes of LEDs to the left.
  for (int ledByte = 0 ; ledByte < 5 ; ledByte++)
  {
    //Check if the highest bit is set, then move to next byte
    if (ledStateB[ledByte] & 0b10000000)
    {
      if (ledByte < 4)
        ledStateB[ledByte + 1] |= 0b1; //Move bit to next byte up
    }

    //Shift this byte by one
    ledStateB[ledByte] <<= 1;
  }

  //Add a bit to the starting byte when needed
  ledShifts++;
  if (ledShifts == ledGap)
  {
    ledShifts = 0;
    ledStateA[0] |= 0b1;
  }
}

//Turn off all leds
void ledOff()
{
  for (int x = 0 ; x < 5 ; x++)
  {
    ledStateA[x] = 0;
    ledStateB[x] = 0;
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
      ledStateA[registerNumber - 5] |= (1 << ledNumber); //Set
    else
      ledStateA[registerNumber - 5] &= (0xFF ^ (1 << ledNumber)); //Clear
  }
}
