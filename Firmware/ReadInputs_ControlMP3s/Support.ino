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
  systemSettings.begin("deviceSettings"); //We will be working within a namespace called deviceSettings

  if (systemSettings.getBytesLength("settingsStruct") == 0) //Check if the NVM key 'settingsStruct' is blank
  {
    systemSettings.putBytes("settingsStruct", &settings, sizeof(Settings)); //Write current settings to NVM
    Serial.println("NVM is blank! Settings are now default."); //Use the default constructors for settings
  }
  else
    systemSettings.getBytes("settingsStruct", &settings, sizeof(Settings)); //Load NVM into the settings struct
}

void recordSettings()
{
  systemSettings.putBytes("settingsStruct", &settings, sizeof(Settings)); //Write current settings to NVM
}

void eraseSettings()
{
  systemSettings.clear(); //Erase all NVM settings
}
