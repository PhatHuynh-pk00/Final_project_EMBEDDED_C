#ifndef LCD_H_
#define LCD_H_

#include "stm32f103xb.h"

void I2C1_Init(void);
void LCD_Init(void);
void LCD_String(char *str);
void LCD_Set_Cursor(uint8_t row, uint8_t col);
void LCD_PrintNumber(uint32_t num); // Bổ sung hàm in số

#endif /* LCD_H_ */