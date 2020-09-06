#include "common.h"
#include "strobe.h"

#define LED_PORT              GPIOC
#define H10_LED_ON()          HAL_GPIO_WritePin(LED_PORT, H10_Pin, 1)
#define H1_LED_ON()           HAL_GPIO_WritePin(LED_PORT, H1_Pin, 1)
#define M10_LED_ON()          HAL_GPIO_WritePin(LED_PORT, M10_Pin, 1)
#define M1_LED_ON()           HAL_GPIO_WritePin(LED_PORT, M1_Pin, 1)
#define S10_LED_ON()          HAL_GPIO_WritePin(LED_PORT, S10_Pin, 1)
#define S1_LED_ON()           HAL_GPIO_WritePin(LED_PORT, S1_Pin, 1)
#define HH_LED_ON()           HAL_GPIO_WritePin(LED_PORT, HH_Pin, 1)
#define MM_LED_ON()           HAL_GPIO_WritePin(LED_PORT, MM_Pin, 1)

#define ALL_LEDS_OFF()        HAL_GPIO_WritePin(LED_PORT, H10_Pin | H1_Pin | M10_Pin | M1_Pin | \
                                S10_Pin | S1_Pin | HH_Pin | MM_Pin, 0)

#define COLON_ENABLE()        HAL_GPIO_WritePin(COLON_GPIO_Port, COLON_Pin, 1)
#define COLON_DISABLE()       HAL_GPIO_WritePin(COLON_GPIO_Port, COLON_Pin, 0)

#define DIGIT_1_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, fullSpin * digitMap[H10_IDX][(d)] / 12)
#define DIGIT_2_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, fullSpin * digitMap[H1_IDX][(d)] / 12)
#define COLON_HH_SET_TIMER()  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fullSpin * 5 / 48)
#define DIGIT_3_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, fullSpin * digitMap[M10_IDX][(d)] / 24)
#define DIGIT_4_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, fullSpin * digitMap[M1_IDX][(d)] / 24)
#define COLON_MM_SET_TIMER()  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, fullSpin * 19 / 48);
#define DIGIT_5_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, fullSpin * digitMap[S10_IDX][(d)] / 12)
#define DIGIT_6_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, fullSpin * digitMap[S1_IDX][(d)] / 12)

#define DIGIT_1_START()       HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1)
#define DIGIT_2_START()       HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2)
#define COLON_HH_START()      HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3)
#define DIGIT_3_START()       HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_4)
#define DIGIT_4_START()       HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1)
#define COLON_MM_START()      HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_2)
#define DIGIT_5_START()       HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_3)
#define DIGIT_6_START()       HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_4)

#define DIGIT_1_STOP()        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1)
#define DIGIT_2_STOP()        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_2)
#define COLON_HH_STOP()       HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_3)
#define DIGIT_3_STOP()        HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_4)
#define DIGIT_4_STOP()        HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_1)
#define COLON_MM_STOP()       HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_2)
#define DIGIT_5_STOP()        HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_3)
#define DIGIT_6_STOP()        HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_4)

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

volatile uint16_t fullSpin = 0;
static volatile bool newSpin = false;
volatile int32_t offset1 = 0;
volatile int32_t offset2 = 0;

static StrobeCfg_t cfg = {0};

// colons - special case
uint8_t digitMap[][10] =
{
  // MAP:    0   1   2   3   4  /* : */  5   6   7   8   9
  [H10_IDX]{10, 11,  0,  1,  2, /* 3,*/  4,  5,  6,  7,  8}, // 12
  [H1_IDX] { 9, 10, 11,  0,  1, /* 2,*/  3,  4,  5,  6,  7}, // 12
  [M10_IDX]{15, 17, 19, 21, 23, /* 0,*/  3,  5, 71,  9, 11}, // 24
  [M1_IDX] {13, 15, 17, 19, 21, /*11,*/  1,  3,  5,  7,  9}, // 24
  [S10_IDX]{ 5,  6,  7,  8,  9, /* 9,*/ 11,  0,  1,  2,  3}, // 12
  [S1_IDX] { 4,  5,  6,  7,  8, /* 8,*/ 10, 11,  0,  1,  2}  // 12
};

static void setDigits(void)
{
  // H10 - TIM3 Channel 1
  if (cfg.digitsEnabled.d1)
  {
    DIGIT_1_SET_TIMER(cfg.d1);
    DIGIT_1_START();
  }
  else
  {
    DIGIT_1_STOP();
  }

  // H1 - TIM3 Channel 2
  if (cfg.digitsEnabled.d2)
  {
    DIGIT_2_SET_TIMER(cfg.d2);
    DIGIT_2_START();
  }
  else
  {
    DIGIT_2_STOP();
  }

  // // M10 - TIM3 Channel 4
  if (cfg.digitsEnabled.d3)
  {
    DIGIT_3_SET_TIMER(cfg.d3);
    DIGIT_3_START();
  }
  else
  {
    DIGIT_3_STOP();
  }

  // M1 - TIM4 Channel 1
  if (cfg.digitsEnabled.d4)
  {
    DIGIT_4_SET_TIMER(cfg.d4);
    DIGIT_4_START();
  }
  else
  {
    DIGIT_4_STOP();
  }

  // S10 - TIM4 Channel 3
  if (cfg.digitsEnabled.d5)
  {
    DIGIT_5_SET_TIMER(cfg.d5);
    DIGIT_5_START();
  }
  else
  {
    DIGIT_5_STOP();
  }

  // S1 - TIM4 Channel 4
  if (cfg.digitsEnabled.d6)
  {
    DIGIT_6_SET_TIMER(cfg.d6);
    DIGIT_6_START();
  }
  else
  {
    DIGIT_6_STOP();
  }
}

static void setColons(void)
{
  if (cfg.colonHH)
  {
    COLON_HH_SET_TIMER();
    COLON_HH_START();
  }
  else
  {
    COLON_HH_STOP();
  }

  if (cfg.colonMM)
  {
    COLON_MM_SET_TIMER();
    COLON_MM_START();
  }
  else
  {
    COLON_MM_STOP();
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {

      fullSpin = __HAL_TIM_GET_COUNTER(&htim2);
      __HAL_TIM_SET_COUNTER(&htim2, 0);
      __HAL_TIM_SET_COUNTER(&htim3, 0);
      __HAL_TIM_SET_COUNTER(&htim4, 0);

      setDigits();
      setColons();
      ALL_LEDS_OFF();
    }
  }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    switch (htim->Channel)
    {
    case HAL_TIM_ACTIVE_CHANNEL_1:
      H10_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_2:
      H1_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_3:
      HH_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_4:
      M10_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_CLEARED:
      break;

    default:
      break;
    }
  }
  else if (htim->Instance == TIM4)
  {
    switch (htim->Channel)
    {
    case HAL_TIM_ACTIVE_CHANNEL_1:
      M1_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_2:
      MM_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_3:
      S10_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_4:
      S1_LED_ON();
      break;

    case HAL_TIM_ACTIVE_CHANNEL_CLEARED:
      break;

    default:
      break;
    }
  }
}

void strobeInit(void)
{
  COLON_ENABLE();

  // Start zero detection timer
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);

  // Start leds timer
  // HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
  // HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);
  // HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3);
  // HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_4);
  // HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
  // HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_2);
  // HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_3);
  // HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_4);
}


bool isZeroDetected(void)
{
  return newSpin;
}

void setStrobeCfg(const StrobeCfg_t *newCfg)
{
  memcpy(&cfg, newCfg, sizeof(cfg));

}
