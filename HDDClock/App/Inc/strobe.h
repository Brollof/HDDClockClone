#ifndef __STROBE_H
#define __STROBE_H

#define DIGITS_ALL 0x3F

enum
{
  H10_IDX = 0,
  H1_IDX,
  M10_IDX,
  M1_IDX,
  S10_IDX,
  S1_IDX,
  DIGITS_NUM
};

// 0011 1111 - 0x3F
typedef struct
{
  union
  {
    uint8_t digits[6];
    struct
    {
      uint8_t d1;
      uint8_t d2;
      uint8_t d3;
      uint8_t d4;
      uint8_t d5;
      uint8_t d6;
    };
  };
  union
  {
    uint8_t all;
    struct
    {
      uint8_t d1 : 1;
      uint8_t d2 : 1;
      uint8_t d3 : 1;
      uint8_t d4 : 1;
      uint8_t d5 : 1;
      uint8_t d6 : 1;
      uint8_t unused : 2;
    };
  } digitsEnabled;
  bool colonHH;
  bool colonMM;
} StrobeCfg_t;

void strobeInit(void);
bool isZeroDetected(void);
void strobe(const StrobeCfg_t *cfg);
void setStrobeCfg(const StrobeCfg_t *newCfg);

#endif
