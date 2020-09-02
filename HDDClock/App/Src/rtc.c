#include "rtc.h"
#include <stdio.h>
#include "stm32f1xx_hal.h"

#define RTC_ADDRESS (0x68 << 1)

#define RTC_I2C_HANDLE hi2c1

extern I2C_HandleTypeDef RTC_I2C_HANDLE;


void rtcInit(void)
{
  printf("RTC initialized\n");
}

void rtcRead(void)
{
  uint8_t buf[7];
  HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&RTC_I2C_HANDLE, RTC_ADDRESS, 0x00, 1, buf, 7, 200);

  uint8_t secs = ((buf[0] & 0x7F) >> 4) * 10 + (buf[0] & 0x0F);
  uint8_t mins = (buf[1] >> 4) * 10 + (buf[1] & 0x0F);
  uint8_t hours = ((buf[2] & 0x3F) >> 4) * 10 + (buf[2] & 0x0F);

  uint8_t weekday = buf[3];
  uint8_t day = (buf[4] >> 4) * 10 + (buf[4] & 0x0F);
  uint8_t month = (buf[5] >> 4) * 10 + (buf[5] & 0x0F);
  uint8_t year = (buf[6] >> 4) * 10 + (buf[6] & 0x0F);

  if (status == HAL_OK)
  {
    printf("%02d:%02d:%02d %02d/%02d/%d\n", hours, mins, secs, day, month, year);
  }
  else
  {
    printf("Sraka. Status: %d\n", status);
  }
}
