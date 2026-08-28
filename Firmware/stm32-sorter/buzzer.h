#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f103xb.h"
#include <stdint.h>

// Khởi tạo chân cắm cho Buzzer (PB12)
void Buzzer_Init(void);

// Bật / Tắt thủ công
void Buzzer_On(void);
void Buzzer_Off(void);

// Các hiệu ứng âm thanh dựng sẵn
void Buzzer_Beep(uint8_t times, uint16_t duration_ms);
void Buzzer_LongBeep(uint16_t duration_ms);

#endif /* BUZZER_H */