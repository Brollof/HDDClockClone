#ifndef __RTC_H
#define __RTC_H

#include <string>

typedef struct
{
  uint8_t secondsH;
  uint8_t secondsL;
  uint8_t minutesH;
  uint8_t minutesL;
  uint8_t hoursH;
  uint8_t hoursL;
} Time_t;

typedef struct
{
  uint8_t dayH;
  uint8_t dayL;
  uint8_t monthH;
  uint8_t monthL;
  uint8_t yearH;
  uint8_t yearL;
} Date_t;

typedef struct
{
  Time_t time;
  Date_t date;
} DateTime_t;

void rtcInit(void);
bool rtcUpdateDateTime(void);
void rtcSetTimeFromString(const std::string& sTime);
void rtcSetDateFromString(const std::string& sDate);
void rtcSetTime(const Time_t *time);
void rtcSetDate(const Date_t *date);
void rtcPrintDateTime(void);
DateTime_t *rtcGetDateTime(void);

#endif
