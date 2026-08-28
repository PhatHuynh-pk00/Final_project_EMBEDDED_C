#ifndef TCS3200_H
#define TCS3200_H

#include "stm32f103xb.h"

extern volatile uint32_t TCS3200_red_value;
extern volatile uint32_t TCS3200_green_value;
extern volatile uint32_t TCS3200_blue_value;

void TCS3200_Init(void);
void TCS3200_RunStateMachine(void);
#endif /* TCS3200_H */