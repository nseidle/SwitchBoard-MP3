void clearSerialBuffer()
{
  delay(20);
  while (Serial.available()) Serial.read();
}

void printBinary(uint8_t value)
{
  for (int x = 0 ; x < 8 ; x++)
  {
    if (value & 0b10000000) Serial.print("1");
    else Serial.print("0");

    value <<= 1;
  }
}

void loadSettings()
{
  if (systemSettings.getBytesLength("settingsStruct") == 0) //Check if the NVM is blank
  {
    recordSettings();
    Serial.println("NVM is blank! Settings are now default"); //Use the default constructors for settings
  }
  else
    systemSettings.getBytes("settingsStruct", &settings, sizeof(Settings)); //Load NVM into the settings struct
}

void recordSettings()
{
  systemSettings.putBytes("settingsStruct", &settings, sizeof(Settings)); //Write current settings to NVM
}

//Get a long int from user, uses sscanf to obtain 64-bit int
//Returns INPUT_RESPONSE_GETNUMBER_EXIT if user presses 'x' or doesn't enter data
//Returns INPUT_RESPONSE_GETNUMBER_TIMEOUT if input times out
long getNumber()
{
  char userEntry[50]; //Allow user to enter more than one char. sscanf will remove extra.
  long userNumber = 0;

  InputResponse response = getString(userEntry, sizeof(userEntry));
  if (response == INPUT_RESPONSE_VALID)
  {
    if (strcmp(userEntry, "x") == 0 || strcmp(userEntry, "X") == 0)
      userNumber = INPUT_RESPONSE_GETNUMBER_EXIT;
    else
      sscanf(userEntry, "%ld", &userNumber);
  }
  else if (response == INPUT_RESPONSE_TIMEOUT)
  {
    Serial.println("\r\nNo user response - Do you have line endings turned on?");
    userNumber = INPUT_RESPONSE_GETNUMBER_TIMEOUT; //Timeout
  }
  else if (response == INPUT_RESPONSE_EMPTY)
  {
    userNumber = INPUT_RESPONSE_GETNUMBER_EXIT; //Empty
  }

  return userNumber;
}

//Option not known
void printUnknown(uint8_t unknownChoice)
{
  Serial.print("Unknown choice: ");
  Serial.write(unknownChoice);
  Serial.println();
}
void printUnknown(int unknownValue)
{
  Serial.print("Unknown value: ");
  Serial.println((uint16_t)unknownValue, DEC);
}

//Gathers raw characters from user until \n or \r is received
//Handles backspace
//Used for raw mixed entry (SSID, pws, etc)
//Used by other menu input methods that use sscanf
//Returns INPUT_RESPONSE_TIMEOUT, INPUT_RESPONSE_OVERFLOW, INPUT_RESPONSE_EMPTY, or INPUT_RESPONSE_VALID
InputResponse getString(char *userString, uint8_t stringSize)
{
  clearSerialBuffer();

  long startTime = millis();
  uint8_t spot = 0;

  while ((millis() - startTime) / 1000 <= menuTimeout)
  {
    delay(1); //Yield to processor

    //Get the next input character
    while (Serial.available() > 0)
    {
      byte incoming = Serial.read();

      if ((incoming == '\r') || (incoming == '\n'))
      {
        Serial.println(); //Echo
        userString[spot] = '\0'; //Null terminate

        if (spot == 0) return INPUT_RESPONSE_EMPTY;

        return INPUT_RESPONSE_VALID;
      }
      else
      {
        Serial.print(incoming); //Echo

        userString[spot++] = incoming;
        if (spot == (stringSize - 1)) //Leave room for termination
          return INPUT_RESPONSE_OVERFLOW;
      }
    }
  }

  return INPUT_RESPONSE_TIMEOUT;
}
