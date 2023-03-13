void printMainMenu()
{
  Serial.println("Switchboard MP3 Player");
  Serial.println("r) Reset");
  Serial.println("p) Play random track");
  Serial.println("!) Erase NVM");
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
    unsigned long startTime = micros();
    sendFrame();
    unsigned long stopTime = micros(); //26us

    Serial.printf("Time for sendFrame: %d\r\n", stopTime - startTime);
  }
  else if (incoming == 'b')
  {
    unsigned long startTime = micros();
    readSwitches();
    unsigned long stopTime = micros(); //687us

    Serial.printf("Time for readSwitches: %d\r\n", stopTime - startTime);

    if (checkSwitch(3) == LOW)
      Serial.println("Button 3 low");
    else
      Serial.println("Button 3 high");
  }
  else if (incoming == 'w')
  {
    Serial.println("Checking switches");

    delay(50);
    while (Serial.available()) Serial.read();

    while (Serial.available() == 0)
    {
      unsigned long startTime = micros();

      readSwitches();
      unsigned long stopTime = micros(); //26us

      Serial.printf("Time for readSwitches: %d\r\n", stopTime - startTime);

      for (int x = 1 ; x <= 80 ; x++)
      {
        if (checkSwitch(x) == false)
          Serial.printf("Switch %d\n\r", x);
      }

      Serial.printf("switchState Array: 0x%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                    switchStateA[4], switchStateA[3], switchStateA[2], switchStateA[1], switchStateA[0],
                    switchStateB[4], switchStateB[3], switchStateB[2], switchStateB[1], switchStateB[0]
                   );
      Serial.println();

      delay(100);
    }
  }

  else if (incoming == '1')
  {
    delay(50);
    while (Serial.available()) Serial.read();

    Serial.println("Blinking");

    while (Serial.available() == 0)
    {
      setLED(1, HIGH);
      setLED(2, LOW);

      Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0]);
      Serial.println();
      sendFrame();
      delay(2000);

      setLED(1, LOW);
      setLED(2, HIGH);

      Serial.printf("LED Array: 0x%02X %02X %02X %02X %02X", ledStateA[4], ledStateA[3], ledStateA[2], ledStateA[1], ledStateA[0]);
      Serial.println();
      sendFrame();
      delay(2000);
    }
  }

#define LED_COUNT 80

  else if (incoming == '2')
  {
    Serial.println("LED display 1");

    delay(50);
    while (Serial.available()) Serial.read();

    ledOff(); //Turn off all LEDs

    while (Serial.available() == 0)
    {
      ledScreenSaver1();

      //Display output
      Serial.printf(" LED Array: ");

      printBinary(ledStateB[4]);
      printBinary(ledStateB[3]);
      printBinary(ledStateB[2]);
      printBinary(ledStateB[1]);
      printBinary(ledStateB[0]);

      printBinary(ledStateA[4]);
      printBinary(ledStateA[3]);
      printBinary(ledStateA[2]);
      printBinary(ledStateA[1]);
      printBinary(ledStateA[0]);

      Serial.println();

      delay(50);
    }
  }
  else if (incoming == '!')
  {
    eraseSettings();
    Serial.println("Settings erased");
  }
}
