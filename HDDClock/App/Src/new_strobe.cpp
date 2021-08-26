#include "new_strobe.h"
#include "common.h"

#define LED_PORT                  GPIOC
#define PIN_STATE(v)              ((GPIO_PinState)v)
#define ALL_LEDS_OFF()            HAL_GPIO_WritePin(LED_PORT, 0xFF, PIN_STATE(0))

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

// Variables used outside this scope
volatile uint16_t fullSpin = 0;
volatile uint32_t maxSector = 0;

// Local stuff
static volatile uint8_t strobe[30] = {0};
static volatile uint8_t sector = 0;
static volatile uint8_t dmaData = 0x03;

void strobeInit()
{
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2); // full spin detection
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);

      fullSpin = __HAL_TIM_GET_COUNTER(&htim2);
      __HAL_TIM_SET_COUNTER(&htim2, 0);

      // trigger interrupt every 15 degrees
      __HAL_TIM_SET_COUNTER(&htim3, 0);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (int)(fullSpin / 24.0));

      ALL_LEDS_OFF();
      sector = 0;
      HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);

    }
  }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      __HAL_TIM_SET_COUNTER(&htim3, 0);
      if (sector == 21)
      {
        // HAL_DMA_Abort(&hdma_memtomem_dma1_channel1);
        HAL_DMA_Start(&hdma_memtomem_dma1_channel1, (uint32_t)&dmaData, (uint32_t)&GPIOC->ODR, 1);
        HAL_DMA_PollForTransfer(&hdma_memtomem_dma1_channel1, HAL_DMA_FULL_TRANSFER, 2000);
      }
      sector++;
      if (maxSector < sector)
        maxSector = sector;
    }
  }
}
