#include "common.h"
#include "console.h"
#include "libUart.h"
#include "rtc.h"


#define BUFFER_SIZE 64
#define IS_ENTER(ch) ((ch) == '\r' || (ch) == '\n')

static bool inProgress = false;
static char command = 0;

static uint8_t key = 0;
static uint8_t data[BUFFER_SIZE] = {0};
static uint32_t pos = 0;


extern int cnt;
extern TIM_HandleTypeDef htim3;
extern uint16_t fullSpin;
extern uint16_t lastCounterValue;
extern int32_t offset1;
extern int32_t offset2;

void initConsole(void)
{
  setbuf(stdout, NULL); // Disable printf buffering
  setRxBuffer(&key);
  startConsoleRx();
}

static void clearBuffer(void)
{
  memset(data, 0, sizeof(data));
  pos = 0;
}

static void numberTest(uint8_t key)
{
  if (inProgress)
  {
    printf("%c", key);
    data[pos++] = key;

    if (IS_ENTER(key))
    {
      int num = atoi((const char *)data);
      printf("Number: %d\n", num);
      inProgress = false;
      clearBuffer();
    }
  }
  else
  {
    printf("Enter number:\n");
    inProgress = true;
  }
}

// TODO: refactor console interaction
static void setTime(uint8_t key)
{
  if (inProgress)
  {
    printf("%c", key);
    // Skip backspace character and remove last char
    if ((uint8_t)key == 127)
      pos--;
    else
      data[pos++] = key;

    if (IS_ENTER(key))
    {
      printf("\n");
      rtcSetTimeFromString((const char *)data, 8);

      inProgress = false;
      clearBuffer();
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
    printf("%c", key);
    // Skip backspace character and remove last char
    if ((uint8_t)key == 127)
      pos--;
    else
      data[pos++] = key;

    if (IS_ENTER(key))
    {
      printf("\n");
      rtcSetDateFromString((const char *)data, 8);

      inProgress = false;
      clearBuffer();
    }
  }
  else
  {
    printf("Enter new date (dd/mm/yy):\n");
    inProgress = true;
  }
}

static uint16_t compare = 0;

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
  case 'q':
    printf("Console test\n");
    break;

  case 'w':
    numberTest(key);
    break;

  case 'p':
    rtcPrintDateTime();
    break;

  case 'u':
    rtcUpdateDateTime();
    break;

  case 't':
    setTime(key);
    break;

  case 'd':
    setDate(key);
    break;

  case 'z':
    break;

  case ',':
    offset1--;
    printf("offset1: %d\n", offset1);
    break;

  case '.':
    offset1++;
    printf("offset1: %d\n", offset1);
    break;

  case 'k':
    offset2--;
    printf("offset2: %d\n", offset2);
    break;

  case 'l':
    offset2++;
    printf("offset2: %d\n", offset2);
    break;

  case 'f':
    printf("full spin: %u\n", fullSpin);

    break;

  default:
    break;
  }

  key = 0;
}
