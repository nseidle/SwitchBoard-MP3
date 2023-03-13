const uint16_t menuTimeout = 60 * 10; //Menus will exit/timeout after this number of seconds

typedef struct {
  uint8_t mp3Volume = 98; //50 = *very* quiet, 60 = *very* quiet, 70 = medium loud
  uint8_t screenSaverSpeed = 100; //A lower number makes screen saver act faster
  uint32_t interactions = 0;
} Settings;
Settings settings;

//Return values for getString()
typedef enum
{
  INPUT_RESPONSE_GETNUMBER_EXIT = -9999999, //Less than min ECEF. User may be prompted for number but wants to exit without entering data
  INPUT_RESPONSE_GETNUMBER_TIMEOUT = -9999998,
  INPUT_RESPONSE_GETCHARACTERNUMBER_TIMEOUT = 255,
  INPUT_RESPONSE_GETCHARACTERNUMBER_EMPTY = 254,
  INPUT_RESPONSE_TIMEOUT = -3,
  INPUT_RESPONSE_OVERFLOW = -2,
  INPUT_RESPONSE_EMPTY = -1,
  INPUT_RESPONSE_VALID = 1,
} InputResponse;
