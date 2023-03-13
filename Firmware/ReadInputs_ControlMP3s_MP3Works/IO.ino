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
