#ifndef UART_H_
#define UART_H_

#include "stm32f103xb.h"

void UART1_Init(void);
void UART_SendChar(char c);
void UART_SendString(char *str);
void UART_SendNumber(uint32_t num);

#endif /* UART_H_ */