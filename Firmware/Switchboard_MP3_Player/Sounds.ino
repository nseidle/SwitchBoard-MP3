//Plays a given track name
bool playTrack(const char* trackName)
{
  //char trackName[40] = "T001.MP3";
  if (trackFile.open(trackName, O_READ) == true)
  {
    player.startSong();
    isPlaying = true;  
    return true;
  }
  else
  {
    Serial.print("Failed to open file: ");
    Serial.println(trackName);
    isPlaying = false;
    return false;
  }
}

void stopTrack()
{
  if (isPlaying == true)
  {
    isPlaying = false;
    player.stopSong();
  }
}

//If the MP3 shield requests more data, feed from MP3 file
void updateMP3()
{
  if (isPlaying == true && player.data_request() == true)
  {
    // Go out to SD card and try reading some number of bytes
    uint8_t mp3DataBuffer[32];
    int bytesToPlay = trackFile.read(mp3DataBuffer, sizeof(mp3DataBuffer));

    if (bytesToPlay <= 0)
    {
      //No data left in file
      player.stopSong();
      trackFile.close(); // Close out this track

      isPlaying = false;
      Serial.println("Song finished playing");
    }
    else
    {
      //Feed MP3 IC
      player.playChunk(mp3DataBuffer, bytesToPlay);
    }
  }
}

//Plays a random track
//Then turns off audio to reduce hissing
void playRandomTrack()
{
  char trackName[13];

  //Don't play the same track as the last one
  byte trackNumber = previousTrack1;
  while (trackNumber == previousTrack1 || trackNumber == previousTrack2)
    trackNumber = random(1, 4); //(inclusive min, exclusive max)

  sprintf(trackName, "T%03d.MP3", trackNumber); //Splice the track number into file name

  Serial.print("Playing random track: ");
  Serial.println(trackName);

  if (isPlaying) stopTrack(); //Stop any previous track

  playTrack(trackName);

  //Update the previous variables
  previousTrack2 = previousTrack1;
  previousTrack1 = trackNumber;
}

//Then turns off audio to reduce hissing
void playSilence()
{
  //  if (MP3player.isPlaying()) MP3player.stopTrack(); //Stop any previous track
  //
  //  MP3player.playMP3((char*)"SILENCE.MP3");

  Serial.println("Playing silence");
}

//Imitialize the SD card
void initSD()
{
  //Initialize the SdCard.
  if (sd.begin(SdSpiConfig(pin_sdcs, SHARED_SPI, SD_SCK_MHZ(4))) == false)
    //  if (sd.begin(pin_sdcs, SPI_HALF_SPEED) == false)
    Serial.println("SD begin error");

  if (!sd.chdir("/"))
    Serial.println("SD root directory error");
}

//Starts MP3 library and sets volume level
void initMP3Player()
{
  //Make sure the MP3 shield is not in reset
  pinMode(pin_mp3Reset, OUTPUT);
  digitalWrite(pin_mp3Reset, HIGH);

  player.begin();
  player.setVolume(settings.mp3Volume);

  playRandomTrack(); //Make some noise so a human knows we are alive
}
