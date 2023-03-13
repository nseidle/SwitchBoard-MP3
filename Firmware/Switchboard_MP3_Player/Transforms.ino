//This is the hardcoded transform of LEDs. It's faster just to manually set it.
//Decoder ring between the I/O board port # and the physical LED location.
//If ledTransform[3] = 17, then the LED location 3 connects with I/O port 17.
void loadRealityArrayLEDs()
{
  ledTransform[60] = 1; //ledTransform[60] = 1, then the LED location 60 connects with I/O port 1 of board B.
  ledTransform[59] = 2;
  ledTransform[58] = 3;
  ledTransform[57] = 4;
  ledTransform[56] = 5;
  ledTransform[55] = 6;
  ledTransform[54] = 7;
  ledTransform[53] = 8;
  ledTransform[52] = 9;
  ledTransform[51] = 10;

  ledTransform[50] = 41; //ledTransform[50] = 41, then the LED location 50 connects with I/O port 1 of board A.
  ledTransform[49] = 42;
  ledTransform[48] = 43;
  ledTransform[47] = 44;
  ledTransform[46] = 45;
  ledTransform[45] = 46;
  ledTransform[44] = 47;
  ledTransform[43] = 48;
  ledTransform[42] = 49;
  ledTransform[41] = 50;

  ledTransform[40] = 11; //ledTransform[40] = 11, then the LED location 40 connects with I/O port 11 of board B.
  ledTransform[39] = 12;
  ledTransform[38] = 13;
  ledTransform[37] = 14;
  ledTransform[36] = 15;
  ledTransform[35] = 16;
  ledTransform[34] = 17;
  ledTransform[33] = 19; //Swap
  ledTransform[32] = 18; //Swap
  ledTransform[31] = 20;

  ledTransform[30] = 51; //ledTransform[30] = 51, then the LED location 30 connects with I/O port 51 of board A.
  ledTransform[29] = 52;
  ledTransform[28] = 53;
  ledTransform[27] = 54;
  ledTransform[26] = 55;
  ledTransform[25] = 56;
  ledTransform[24] = 57;
  ledTransform[23] = 58;
  ledTransform[22] = 59;
  ledTransform[21] = 60;

  ledTransform[20] = 21; //ledTransform[20] = 21, then the LED location 20 connects with I/O port 21 of board B.
  ledTransform[19] = 22;
  ledTransform[18] = 23;
  ledTransform[17] = 24;
  ledTransform[16] = 25;
  ledTransform[15] = 26;
  ledTransform[14] = 27;
  ledTransform[13] = 28;
  ledTransform[12] = 29;
  ledTransform[11] = 30;

  ledTransform[10] = 61; //ledTransform[10] = 61, then the LED location 10 connects with I/O port 61 of board A.
  ledTransform[9] = 62;
  ledTransform[8] = 63;
  ledTransform[7] = 64;
  ledTransform[6] = 65;
  ledTransform[5] = 66;
  ledTransform[4] = 67;
  ledTransform[3] = 68;
  ledTransform[2] = 69;
  ledTransform[1] = 70;  //ledTransform[1] = 70, then the LED location 1 connects with I/O port 70 of board A.
}

//This is the hardcoded transform of switches. It's faster just to manually set it.
//Decoder ring between the I/O board port # and the physical switch location.
//switchTransform[10] = 61 means that the plug labeled '10' on the front of the switchboard (the user number) is connected to input 61 (Board A, pin 21) on the electronics
void loadRealityArraySwitches()
{
  switchTransform[60] = 1; //switchTransform[60] = 1, then the switch in location 60 connects with input port 1 of board B.
  switchTransform[59] = 2;
  switchTransform[58] = 3;
  switchTransform[57] = 4;
  switchTransform[56] = 5;
  switchTransform[55] = 6;
  switchTransform[54] = 7;
  switchTransform[53] = 8;
  switchTransform[52] = 9;
  switchTransform[51] = 10;

  switchTransform[50] = 41; //switchTransform[50] = 41, then the switch in location 50 connects with input port 1 of board A.
  switchTransform[49] = 42;
  switchTransform[48] = 43;
  switchTransform[47] = 44;
  switchTransform[46] = 45;
  switchTransform[45] = 46;
  switchTransform[44] = 47;
  switchTransform[43] = 48;
  switchTransform[42] = 49;
  switchTransform[41] = 50;

  switchTransform[40] = 11; //switchTransform[40] = 11, then the switch in location 40 connects with input port 11 of board B.
  switchTransform[39] = 12;
  switchTransform[38] = 13;
  switchTransform[37] = 14;
  switchTransform[36] = 15;
  switchTransform[35] = 16;
  switchTransform[34] = 17;
  switchTransform[33] = 19; //Swap
  switchTransform[32] = 18; //Swap
  switchTransform[31] = 20;

  switchTransform[30] = 51; //switchTransform[30] = 51, then the switch in location 30 connects with input port 51 of board A.
  switchTransform[29] = 52;
  switchTransform[28] = 53;
  switchTransform[27] = 54;
  switchTransform[26] = 55;
  switchTransform[25] = 56;
  switchTransform[24] = 57;
  switchTransform[23] = 58;
  switchTransform[22] = 59;
  switchTransform[21] = 60;

  switchTransform[20] = 21; //switchTransform[20] = 21, then the switch in location 20 connects with input port 21 of board B.
  switchTransform[19] = 22;
  switchTransform[18] = 23;
  switchTransform[17] = 24;
  switchTransform[16] = 25;
  switchTransform[15] = 26;
  switchTransform[14] = 27;
  switchTransform[13] = 28;
  switchTransform[12] = 29;
  switchTransform[11] = 30;

  switchTransform[10] = 61; //switchTransform[10] = 61, then the switch in location 10 connects with input port 61 of board A.
  switchTransform[9] = 62;
  switchTransform[8] = 63;
  switchTransform[7] = 64;
  switchTransform[6] = 65;
  switchTransform[5] = 66;
  switchTransform[4] = 67;
//  switchTransform[3] = 0; //switchTransform[3] = 0, then the switch in location 3 connects to nothing. 
  switchTransform[3] = 68; //Repaired 
  switchTransform[2] = 69;
  switchTransform[1] = 70; //switchTransform[1] = 70, then the switch in location 1 connects with input port 70 of board A.  
}
