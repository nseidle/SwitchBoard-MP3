/*
  ESP32 Preferences
*/

typedef struct {
  char myName[40] = "TestName";
  uint8_t myValue = 16;
} Settings;
Settings settings;

//Preferences for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Preferences.h>
Preferences systemSettings;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println("ESP32 Preferences Demo");

  systemSettings.begin("deviceSettings"); //We will be working within a namespace called deviceSettings

  if (systemSettings.getBytesLength("settingsStruct") == 0) //Check if the NVM is blank
  {
    systemSettings.putBytes("settingsStruct", &settings, sizeof(Settings)); //Write default settings to NVM
    Serial.println("NVM is blank! Settings are now default"); //Use the default constructors for settings
  }
  else
    systemSettings.getBytes("settingsStruct", &settings, sizeof(Settings)); //Load NVM into the settings struct

  settings.myValue++;
  systemSettings.putBytes("settingsStruct", &settings, sizeof(Settings)); //Write current settings to NVM

  Serial.printf("myName: %s\r\n", settings.myName);
  Serial.printf("myValue: %d\r\n", settings.myValue);
  Serial.println("Press r to reset");
  Serial.println("Press d to delete NVM");
  Serial.println("Press + to increase value");
}

void loop()
{
  if (Serial.available())
  {
    byte incoming = Serial.read();

    if (incoming == 'r')
    {
      ESP.restart();
    }
    else if (incoming == 'd')
    {
      systemSettings.clear(); //Erase all NVM settings
      Serial.println("Settings erased");
    }
    else if (incoming == '+')
    {
      settings.myValue++;
      systemSettings.putBytes("settingsStruct", &settings, sizeof(Settings)); //Write current settings to NVM
      Serial.printf("myValue: %d\r\n", settings.myValue);
    }
  }
}
