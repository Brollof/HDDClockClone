#include "common.h"
#include "console.h"
#include "libUart.h"
#include "rtc.h"
#include <string>

#define BUFFER_SIZE 64
#define IS_ENTER(ch) ((ch) == '\r' || (ch) == '\n')

static bool inProgress = false;
static char command = 0;

static uint8_t key = 0;
static std::string data;

extern uint16_t fullSpin;
extern int32_t offsets[];

void initConsole(void)
{
  setbuf(stdout, NULL); // Disable printf buffering
  setRxBuffer(&key);
  startConsoleRx();
}

static void clearBuffer(void)
{
	data.clear();
}

static void changeOffset(uint8_t n, int32_t val)
{
  offsets[n - 1] += val;
  printf("offset%d: %d\n", n, offsets[n - 1]);
}

// TODO: refactor console interaction
static void setTime(uint8_t key)
{
  if (inProgress)
  {
    if (IS_ENTER(key))
    {
      printf("\n");
      rtcSetTimeFromString(data);

      inProgress = false;
      clearBuffer();
    }
    else
    {
      printf("%c", key);
      // Skip backspace character and remove last char
      if ((uint8_t)key == 127)
        data.pop_back();
      else
        data += key;
    }
  }
  else
  {
    printf("Enter new time (hh:mm:ss):\n");
    inProgress = true;
  }
}

static void setDate(uint8_t key)
{
  if (inProgress)
  {
    if (IS_ENTER(key))
    {
      printf("\n");
      rtcSetDateFromString(data);

      inProgress = false;
      clearBuffer();
    }
    else
    {
      printf("%c", key);
      // Skip backspace character and remove last char
      if ((uint8_t)key == 127)
        data.pop_back();
      else
        data += key;
    }
  }
  else
  {
    printf("Enter new date (dd/mm/yy):\n");
    inProgress = true;
  }
}

void processConsoleInput(void)
{
  // Check if there is new data
  if (!key)
    return;

  // Don't interpret new data as command if current command is in progress
  if (!inProgress)
    command = key;

  switch (command)
  {

  case 'p':
    rtcPrintDateTime();
    break;

  case 'u':
    rtcUpdateDateTime();
    break;

  case 'T':
    setTime(key);
    break;

  case 'D':
    setDate(key);
    break;

  case 'f':
    printf("full spin: %u\n", fullSpin);
    break;

  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
    changeOffset(key - '0', 1);
    break;

  case '!':
    changeOffset(1, -1);
    break;

  case '@':
    changeOffset(2, -1);
    break;

  case '#':
    changeOffset(3, -1);
    break;

  case '$':
    changeOffset(4, -1);
    break;

  case '%':
    changeOffset(5, -1);
    break;

  case '^':
    changeOffset(6, -1);
    break;

  default:
    printf("key: %d (0x%02x)\n", key, key);
    break;
  }

  key = 0;
}
