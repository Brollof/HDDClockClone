#include "common.h"
#include "rtc.h"

#define RTC_ADDRESS             0xD0 // 0x68 << 1
#define RTC_REG_BASE            0x00
#define RTC_I2C_TIMEOUT         200 // ms
#define RTC_INIT_RETRIES        3
#define RTC_I2C_HANDLE          hi2c1

// Converts single char digit into integer using ASCII table offset
#define CH_TO_NUM(ch)           ((uint8_t)(ch) - 48)

// Timekeeper registers offset
enum
{
  SECONDS_IDX = 0,
  MINUTES_IDX,
  HOURS_IDX,
  WEEKDAY_IDX,
  DAY_IDX,
  MONTH_IDX,
  YEAR_IDX,
  TIMEKEEPER_IDX_MAX,
};

extern I2C_HandleTypeDef RTC_I2C_HANDLE;
static DateTime_t dt = {0};

static HAL_StatusTypeDef rtcWrite(uint8_t rtcRegAddr, uint8_t *buf, uint8_t size)
{
  return HAL_I2C_Mem_Write(&RTC_I2C_HANDLE, RTC_ADDRESS, rtcRegAddr, sizeof(uint8_t), buf, size, RTC_I2C_TIMEOUT);
}

static HAL_StatusTypeDef rtcRead(uint8_t rtcRegAddr, uint8_t *buf, uint8_t size)
{
  return HAL_I2C_Mem_Read(&RTC_I2C_HANDLE, RTC_ADDRESS, rtcRegAddr, sizeof(uint8_t), buf, size, RTC_I2C_TIMEOUT);
}

static void rtcReinit(void)
{
  printf("RTC re-init...\n");
  if (HAL_I2C_DeInit(&RTC_I2C_HANDLE) != HAL_OK)
  {
    printf("deinit failed!\n");
  }

  HAL_Delay(50);

  if (HAL_I2C_Init(&RTC_I2C_HANDLE) != HAL_OK)
  {
    printf("init failed!\n");
  }
}

void rtcInit(void)
{
  uint8_t retries = RTC_INIT_RETRIES;

  while (retries)
  {
    retries--;
    if (rtcUpdateDateTime() == true)
    {
      printf("RTC initialized\n");
      return;
    }
    else
    {
      printf("RTC initialization failed!\n");
      rtcReinit();
    }
    HAL_Delay(50);
  }
  printf("RTC ERROR, not initialized after %d retries!\n", RTC_INIT_RETRIES);
}

void rtcPrintDateTime(void)
{
  printf("%d%d:%d%d:%d%d %d%d/%d%d/%d%d\n",
         dt.time.hoursH,     dt.time.hoursL,
         dt.time.minutesH,   dt.time.minutesL,
         dt.time.secondsH,   dt.time.secondsL,
         dt.date.dayH,       dt.date.dayL,
         dt.date.monthH,     dt.date.monthL,
         dt.date.yearH,      dt.date.yearL);
}

bool rtcUpdateDateTime(void)
{
  uint8_t buf[TIMEKEEPER_IDX_MAX];

  HAL_StatusTypeDef status = rtcRead(RTC_REG_BASE, buf, sizeof(buf));
  if (status != HAL_OK)
  {
    printf("RTC Read failed, status: %d\n", status);
    return false;
  }

  // printf("RTC read ok\n");

  dt.time.secondsH = (buf[SECONDS_IDX] & 0x7F) >> 4;
  dt.time.secondsL = buf[SECONDS_IDX] & 0x0F;
  dt.time.minutesH = buf[MINUTES_IDX] >> 4;
  dt.time.minutesL = buf[MINUTES_IDX] & 0x0F;
  dt.time.hoursH = (buf[HOURS_IDX] & 0x3F) >> 4;
  dt.time.hoursL = buf[HOURS_IDX] & 0x0F;

  dt.date.dayH = buf[DAY_IDX] >> 4;
  dt.date.dayL = buf[DAY_IDX] & 0x0F;
  dt.date.monthH = buf[MONTH_IDX] >> 4;
  dt.date.monthL = buf[MONTH_IDX] & 0x0F;
  dt.date.yearH = buf[YEAR_IDX] >> 4;
  dt.date.yearL = buf[YEAR_IDX] & 0x0F;

  return true;
}

// Time string example: 09:35:00
void rtcSetTimeFromString(const std::string& sTime)
{
  if (sTime.size() != 8)
  {
    printf("Invalid data length!\n");
    return;
  }

  // Validate input
  for (uint8_t i = 0; i < sTime.size(); i++)
  {
    if (i == 2 || i == 5)
    {
      continue;
    }

    if (isdigit(sTime[i]) == false)
    {
      printf("Incorrect time digits given!\n");
      return;
    }
  }

  const Time_t time =
  {
    .secondsH = CH_TO_NUM(sTime[6]),
    .secondsL = CH_TO_NUM(sTime[7]),
    .minutesH = CH_TO_NUM(sTime[3]),
    .minutesL = CH_TO_NUM(sTime[4]),
    .hoursH = CH_TO_NUM(sTime[0]),
    .hoursL = CH_TO_NUM(sTime[1])
  };

  rtcSetTime(&time);
}

// Date string example: 03/12/20
void rtcSetDateFromString(const std::string& sDate)
{
  if (sDate.size() != 8)
  {
    printf("Invalid data length!\n");
    return;
  }

  // Validate input
  for (uint8_t i = 0; i < sDate.size(); i++)
  {
    if (i == 2 || i == 5)
    {
      continue;
    }

    if (isdigit(sDate[i]) == false)
    {
      printf("Incorrect date digits given!\n");
      return;
    }
  }

  const Date_t date =
  {
    .dayH = CH_TO_NUM(sDate[0]),
    .dayL = CH_TO_NUM(sDate[1]),
    // index 2 skipped - slash character
    .monthH = CH_TO_NUM(sDate[3]),
    .monthL = CH_TO_NUM(sDate[4]),
    // index 5 skipped - slash character
    .yearH = CH_TO_NUM(sDate[6]),
    .yearL = CH_TO_NUM(sDate[7])
  };

  rtcSetDate(&date);
}

void rtcSetTime(const Time_t *time)
{
  uint8_t buf[3];
  buf[0] = (time->secondsH << 4) | time->secondsL;
  buf[1] = (time->minutesH << 4) | time->minutesL;
  buf[2] = (time->hoursH << 4) | time->hoursL;

  HAL_StatusTypeDef status = rtcWrite(RTC_REG_BASE, buf, sizeof(buf));
  if (status == HAL_OK)
  {
    printf("RTC write ok\n");
    // update global date-time structure
    memcpy(&dt.time, time, sizeof(dt.time));
  }
  else
  {
    printf("RTC write failed, status: %d\n", status);
  }
}

void rtcSetDate(const Date_t *date)
{
  uint8_t buf[3];
  buf[0] = (date->dayH << 4) | date->dayL;
  buf[1] = (date->monthH << 4) | date->monthL;
  buf[2] = (date->yearH << 4) | date->yearL;

  HAL_StatusTypeDef status = rtcWrite(RTC_REG_BASE + DAY_IDX, buf, sizeof(buf));
  if (status == HAL_OK)
  {
    printf("RTC write ok\n");
    // update global date-time structure
    memcpy(&dt.date, date, sizeof(dt.date));
  }
  else
  {
    printf("RTC write failed, status: %d\n", status);
  }
}

DateTime_t *rtcGetDateTime(void)
{
  return &dt;
}
