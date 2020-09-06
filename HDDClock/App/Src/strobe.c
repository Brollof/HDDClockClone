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

#define GET_CNT_VAL(idx, digit) (fullSpin * factors[idx][digit] + offsets[idx])

#define DIGIT_1_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, GET_CNT_VAL(H10_IDX, d))
#define DIGIT_2_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, GET_CNT_VAL(H1_IDX, d))
#define COLON_HH_SET_TIMER()  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fullSpin * 5 / 48)
#define DIGIT_3_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, GET_CNT_VAL(M10_IDX, d))
#define DIGIT_4_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, GET_CNT_VAL(M1_IDX, d))
#define COLON_MM_SET_TIMER()  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, fullSpin * 19 / 48)
#define DIGIT_5_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, GET_CNT_VAL(S10_IDX, d))
#define DIGIT_6_SET_TIMER(d)  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, GET_CNT_VAL(S1_IDX, d))

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

static StrobeCfg_t cfg = {0};

// Offsets for 6 digits
int32_t offsets[6] = {0};

static const float factors[][10] =
{
  //          0      1       2     3      4      5     6      7     8      9
  [H10_IDX]{10/12.0, 11/12.0,  0/12.0,  1/12.0,  2/12.0,  4/12.0,  5/12.0,  6/12.0, 7/12.0,  8/12.0},
  [H1_IDX] { 9/12.0, 10/12.0, 11/12.0,  0/12.0,  1/12.0,  3/12.0,  4/12.0,  5/12.0, 6/12.0,  7/12.0},
  [M10_IDX]{15/24.0, 17/24.0, 19/24.0, 21/24.0, 23/24.0,  3/24.0,  5/24.0, 71/24.0, 9/24.0, 11/24.0},
  [M1_IDX] {13/24.0, 15/24.0, 17/24.0, 19/24.0, 21/24.0,  1/24.0,  3/24.0,  5/24.0, 7/24.0,  9/24.0},
  [S10_IDX]{ 5/12.0,  6/12.0,  7/12.0,  8/12.0,  9/12.0, 11/12.0,  0/12.0,  1/12.0, 2/12.0,  3/12.0},
  [S1_IDX] { 4/12.0,  5/12.0,  6/12.0,  7/12.0,  8/12.0, 10/12.0, 11/12.0,  0/12.0, 1/12.0,  2/12.0}
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
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
}


bool isZeroDetected(void)
{
  return newSpin;
}

void setStrobeCfg(const StrobeCfg_t *newCfg)
{
  memcpy(&cfg, newCfg, sizeof(cfg));

}
