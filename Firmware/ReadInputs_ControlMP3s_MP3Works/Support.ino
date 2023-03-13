void printBinary(uint8_t value)
{
  for (int x = 0 ; x < 8 ; x++)
  {
    if (value & 0b10000000) Serial.print("1");
    else Serial.print("0");

    value <<= 1;
  }
}
