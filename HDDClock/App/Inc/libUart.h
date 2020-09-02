#ifndef __LIB_UART_H
#define __LIB_UART_H

#include <stdint.h>

void setRxBuffer(uint8_t* newBuf);
void startConsoleRx(void);

#endif
