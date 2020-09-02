#include "console.h"
#include "libUart.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 64
#define IS_ENTER(ch) ((ch) == '\r' || (ch) == '\n')

static bool inProgress = false;
static char command = 0;

static uint8_t key = 0;
static uint8_t data[BUFFER_SIZE] = {0};
static uint32_t pos = 0;

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

  default:
    break;
  }

  key = 0;
}
