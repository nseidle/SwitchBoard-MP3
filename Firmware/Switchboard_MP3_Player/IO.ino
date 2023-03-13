//Returns true if plug is inserted
bool isPlugInserted(int plugNumber)
{
  //The plug jack is wired in a way so that when no plug is inserted, the
  //input is grounded. If a plug is inserted, it pulls the paddle away from the GND connection
  //and the input goes high
  if (getSwitch(plugNumber) == true)
    return (true);

  return (false);
}

//Return true if bit of given switchNumber is set
bool getSwitch(int switchNumber)
{
  if (switchNumber < 1)
  {
    //switchNumber == 0 is a special case
    //This is where we point to if a switch is malfunctioning

    //Serial.println("getSwitch error");
    return (false);
  }

  switchNumber--; //Align 1-80 to 0-79

  int registerNumber = switchNumber / 8;
  switchNumber %= 8;

  if (registerNumber < 5)
    return (switchStateB[registerNumber] & (1 << switchNumber));

  return (switchStateA[registerNumber - 5] & (1 << switchNumber));
}

//Return true if bit of given switchNumber in the Reality arrays is set
bool getSwitchReality(int switchNumber)
{
  if (switchNumber < 1)
  {
    //Serial.println("getSwitchReality error");
    return (false);
  }

  switchNumber--; //Align 1-80 to 0-79

  int registerNumber = switchNumber / 8;
  switchNumber %= 8;

  if (registerNumber < 5)
    return (switchStateBReality[registerNumber] & (1 << switchNumber));

  return (switchStateAReality[registerNumber - 5] & (1 << switchNumber));
}

//Sets the given switch number to true or false
//This is used in conjunction with the reality transform to map the read inputs to the labeled inputs
void setSwitch(int switchNumber, int level)
{
  switchNumber--; //Map 1 to 80 LEDs to 0-79

  int registerNumber = switchNumber / 8;
  switchNumber %= 8;

  if (registerNumber < 5)
  {
    if (level == HIGH)
      switchStateB[registerNumber] |= (1 << switchNumber); //Set
    else
      switchStateB[registerNumber] &= (0xFF ^ (1 << switchNumber)); //Clear
  }
  else
  {
    if (level == HIGH)
      switchStateA[registerNumber - 5] |= (1 << switchNumber); //Set
    else
      switchStateA[registerNumber - 5] &= (0xFF ^ (1 << switchNumber)); //Clear
  }
}

//Repeatedly set the bits in switchControlA to step through all 40 switches
//Load switch values into switchState array
//First we check the switches on the A board (the 2nd board)
//Then we check the switches on the B board (the 1st board)
void readSwitches()
{
  //Check the 2nd board
  for (int registerNumber = 0 ; registerNumber < 5 ; registerNumber++)
  {
    switchStateAReality[registerNumber] = 0; //Reset

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
      sendFrame(); //Send

      updateMP3(); //Feed MP3 IC with file data as needed

      //1 causes the reading of the switch before it
      //10 causes some switches to read as 0
      //100 works well
      delayMicroseconds(50); //Allow switch and signal to propogate
      //delayMicroseconds(100); //Allow switch and signal to propogate

      //Load the value sitting on the SWITCH pin into the switchState
      if (digitalRead(SWITCH) == HIGH)
        switchStateAReality[registerNumber] |= (1 << (7 - channelSelect));
    }
  }

  //Now we check 1st board
  for (int registerNumber = 0 ; registerNumber < 5 ; registerNumber++)
  {
    switchStateBReality[registerNumber] = 0; //Reset

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

      updateMP3(); //Feed MP3 IC with file data as needed

      delayMicroseconds(50); //Allow switch and signal to propogate

      //Load the value sitting on the SWITCH pin into the switchState
      if (digitalRead(SWITCH) == HIGH)
        switchStateBReality[registerNumber] |= (1 << (7 - channelSelect));
    }

    //    Serial.printf("switchState: 0b"BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(switchState[registerNumber]));
    //    Serial.println();
  }

  //Convert the switches we just read from reality back to our internal system
  loadInternalSwitchArray();
}

//Clock out the switchControl and LED array to the sink drivers
//For a dual setup, first A is sent to reach the 2nd board, then B for the first board.
void sendFrame()
{
  //Convert LEDs arrays from our internal system to the hardware reality
  loadRealityLEDArray();

  //Clock out the LED and control array out to the drivers

  //LED 40 is clocked out first
  for (int ledByteNumber = 4 ; ledByteNumber >= 0 ; ledByteNumber--)
  {
    for (int x = 0 ; x < 8 ; x++)
    {
      digitalWrite(CLOCK, LOW);
      digitalWrite(DATA, ledStateAReality[ledByteNumber] & (1 << (7 - x)));
      digitalWrite(CLOCK, HIGH); //Bit transfers to the register on the rising edge of CLOCK
    }
  }

  //INPUT E0 is clocked out last
  for (int x = 0 ; x < 8 ; x++)
  {
    digitalWrite(CLOCK, LOW);
    digitalWrite(DATA, switchControlA & (1 << (7 - x)));
    digitalWrite(CLOCK, HIGH); //Bit transfers to the register on the rising edge of CLOCK
  }

  //LED 40 is clocked out first
  for (int ledByteNumber = 4 ; ledByteNumber >= 0 ; ledByteNumber--)
  {
    for (int x = 0 ; x < 8 ; x++)
    {
      digitalWrite(CLOCK, LOW);
      digitalWrite(DATA, ledStateBReality[ledByteNumber] & (1 << (7 - x)));
      digitalWrite(CLOCK, HIGH); //Bit transfers to the register on the rising edge of CLOCK
    }
  }

  //INPUT E0 is clocked out last
  for (int x = 0 ; x < 8 ; x++)
  {
    digitalWrite(CLOCK, LOW);
    digitalWrite(DATA, switchControlB & (1 << (7 - x)));
    digitalWrite(CLOCK, HIGH); //Bit transfers to the register on the rising edge of CLOCK
  }

  //Latch this data
  digitalWrite(LATCH, LOW);
  digitalWrite(LATCH, HIGH); //Shift register presents its data on the rising edge of LATCH
}

//Transform the ledState and switchState arrays into the locations on the I/O boards
//Step through the LEDstate arrays and convert to reality
void loadRealityLEDArray()
{
  //Get the real location of an LED from the transform
  //Then set the appropriate bit/LED in the reality LED array
  for (int ledNumber = 1 ; ledNumber <= numberOfLEDs ; ledNumber++)
  {
    //Change the location of that bit to reality
    int realLocation = ledTransform[ledNumber];

    //Set bit in reality array
    setRealityLED(realLocation, getLED(ledNumber));
  }
}

//Transform the recently read switchStateXReality back into internal switch numbers
//Step through the switchReality arrays and convert to internal numbers
//For example, switchStateAReality[registerNumber & 1<<5] might be 1, but that is actually the switch labeled '16'
//Once this is complete, we can call getSwitch(16) and correctly check is something is plugged into jack #16
void loadInternalSwitchArray()
{
  //Get the real location of a switch from the transform
  //Then set the appropriate bit in the internal switch array
  for (int switchNumber = 1 ; switchNumber <= numberOfPlugs ; switchNumber++)
  {
    //switchTransform[10] = 61 means that the plug labeled '10' on the front of the switchboard (the user number) is connected to input 61 (Board A, pin 21) on the electronicsvoid loadRealityArraySwitches()

    int hardwarePortNumber = switchTransform[switchNumber]; //switchNumber 10 resolves to port number is 61

    //    Serial.print("switchNumber: ");
    //    Serial.print(switchNumber);
    //    Serial.print(" found at hardwarePortNumber: ");
    //    Serial.print(hardwarePortNumber);
    //    Serial.println();

    //getSwitchReality(61) returns true if the bit is set in the switchStateAReality register (that lines up with hardware port 61)
    bool isRealSwitchPlugged = getSwitchReality(hardwarePortNumber);

    //Using this bit, set our internal array
    setSwitch(switchNumber, isRealSwitchPlugged); //switchNumber 10, set to 1
  }
}

void printLEDArray()
{
  Serial.printf("ledState Array: (A) %02X %02X %02X %02X %02X (B) %02X %02X %02X %02X %02X",
                ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0],
                ledStateB[4], ledStateB[3], ledStateB[2], ledStateB[1], ledStateB[0]
               );
  Serial.println();
}

void printLEDTransformArray()
{
  Serial.print("ledTransform array: ");

  for (int x = 0 ; x < numberOfLEDs ; x++)
    Serial.printf("LED %d is connected to Output port# %d\r\n", x, ledTransform[x]);

  Serial.println();
}

void printSwitchTransformArray()
{
  Serial.print("switchTransform array: ");

  for (int x = 0 ; x < numberOfPlugs ; x++)
    Serial.printf("Switch %d is connected to Input port# %d\r\n", x, switchTransform[x]);

  Serial.println();
}

void printSwitchArray()
{
  Serial.printf("switchState Array: (A) %02X %02X %02X %02X %02X (B) %02X %02X %02X %02X %02X",
                switchStateA[4], switchStateA[3], switchStateA[2], switchStateA[1], switchStateA[0],
                switchStateB[4], switchStateB[3], switchStateB[2], switchStateB[1], switchStateB[0]
               );
  Serial.println();
}

unsigned long plugInsertionTime[numberOfPlugs + 1]; //We count plugs 1 to 80

//Returns the number of the newest plug to be inserted
//If multiple plugs are detected, returns the plug that is most recently inserted
//Returns 0 if no plug is detected
uint8_t getPlugNumber()
{
  static int lastPlugInserted = 0;

  //Read the inputs from all plugs. This also does a sendFrame() which will send/update all the LED states.
  //Therefore we do not call sendFrame() here.
  readSwitches();

  //Update the time table with any new switches
  for (int x = 1 ; x < numberOfPlugs + 1 ; x++)
  {
    if (isPlugInserted(x) == true && plugInsertionTime[x] == 0)
      plugInsertionTime[x] = millis();

    //Erase plug states if the plug is no longer inserted
    else if (isPlugInserted(x) == false && plugInsertionTime[x] > 0)
      plugInsertionTime[x] = 0;
  }

  //Find the largest time
  unsigned long largestTime = 0;
  for (int x = 1 ; x < numberOfPlugs + 1 ; x++)
    if (plugInsertionTime[x] > largestTime) largestTime = plugInsertionTime[x];

  //If no time was found, no plugs are inserted
  if(largestTime == 0) return(0);

  //Find the switch with the largest time and return it
  for (int x = 1 ; x < numberOfPlugs + 1 ; x++)
    if (plugInsertionTime[x] == largestTime) return (x);

  return (0);
}
