#include "common.h"
#include "console.h"
#include "libUart.h"
#include "rtc.h"
#include <string>

#define BUFFER_SIZE 64
#define BACKSPACE   127
#define IS_ENTER(ch) ((ch) == '\r' || (ch) == '\n')

enum DateTime {DATE, TIME};



extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;
static volatile uint8_t dmaData = 0xFF;

extern TIM_HandleTypeDef htim3;
extern uint32_t maxSector;

static bool inProgress = false;
static char command = 0;

static uint8_t key = 0;
static std::string data;

extern uint16_t fullSpin;

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

// Sets date or time based on 'dt' parameter
static void setDateTime(uint8_t key, DateTime dt)
{
  if (inProgress)
  {
    if (IS_ENTER(key))
    {
      printf("\n");
      if (dt == DateTime::TIME)
        rtcSetTimeFromString(data);
      else
        rtcSetDateFromString(data);

      inProgress = false;
      clearBuffer();
    }
    else
    {
      printf("%c", key);
      // Skip backspace character and remove last char
      if ((uint8_t)key == BACKSPACE)
      {
        if (data.size() > 0)
          data.pop_back();
      }
      else
      {
        data += key;
      }
    }
  }
  else
  {
    if (dt == DateTime::TIME)
      printf("Enter new time (hh:mm:ss):\n");
    else
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
    setDateTime(key, DateTime::TIME);
    break;

  case 'D':
    setDateTime(key, DateTime::DATE);
    break;

  case 'f':
    printf("full spin: %u\n", fullSpin);
    break;

  case 'd':
  {
    printf("Aborting dma...\n");
    HAL_StatusTypeDef status = HAL_DMA_Abort(&hdma_memtomem_dma1_channel1);
    printf("status: %d\n", status);

    printf("DMA Start with data: %d\n", dmaData);
    status = HAL_DMA_Start(&hdma_memtomem_dma1_channel1, (uint32_t)&dmaData, (uint32_t)&GPIOC->ODR, 1);
    printf("status: %d\n", status);

    //printf("Polling for DMA completion...\n");
    //status = HAL_DMA_PollForTransfer(&hdma_memtomem_dma1_channel1, HAL_DMA_FULL_TRANSFER, 2000);
    //printf("status: %d\n", status);

    break;
  }

  case 'r':
    dmaData ^= 0xFF;
    printf("Current DMA data: 0x%02x\n", dmaData);
    break;

  case 's':
  {
    int temp = fullSpin / 24.0;
    printf("TIM3 Compare set to: %d\n", temp);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, temp);
    break;
  }

  case 'm':
    printf("Max sector: %d\n", (int)maxSector);
    break;

  default:
    printf("key: %d (0x%02x)\n", key, key);
    break;
  }

  key = 0;
}
