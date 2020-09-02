#include "libUart.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdbool.h>

#define CONSOLE_UART huart2

// Console uart handle
extern UART_HandleTypeDef CONSOLE_UART;

// Tx semaphore
static volatile bool TxTransferCompleted = false;

// Rx buffer pointer
static uint8_t* RxByte = NULL;

static void sendChar(char c)
{
  HAL_UART_Transmit_IT(&CONSOLE_UART, (uint8_t *)&c, 1);
  while (TxTransferCompleted == false);
  TxTransferCompleted = false;
}

#ifdef __GNUC__
int __io_putchar(int c)
#else
int fputc(int c, FILE *f)
#endif
{
  if (c == '\n') sendChar('\r');
  sendChar(c);
  return c;
}

void setRxBuffer(uint8_t* buf)
{
  RxByte = buf;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &CONSOLE_UART)
  {
    startConsoleRx();
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &CONSOLE_UART)
  {
    TxTransferCompleted = true;
  }
}

void startConsoleRx(void)
{
  HAL_UART_Receive_IT(&CONSOLE_UART, RxByte, 1);
}
