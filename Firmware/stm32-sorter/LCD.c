#include "LCD.h"

extern void Delay_ms(uint32_t ms);

// Địa chỉ I2C của màn hình
#define LCD_ADDR 0x4E 

void I2C1_Init(void) {
    // 1. Cấp clock cho I2C1, GPIOB và AFIO
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // 2. Cấu hình PB6 (SCL) và PB7 (SDA) là Alternate Function Open-Drain (AF-OD), 50MHz
    GPIOB->CRL &= ~(0xFF000000);
    GPIOB->CRL |=  (0xFF000000); 

    // 3. Reset ngoại vi I2C1 để đảm bảo trạng thái sạch
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    // 4. Cấu hình tần số I2C (Giả sử APB1 = 8MHz)
    I2C1->CR2 = 8;             
    I2C1->CCR = 40;             // Tốc độ 100 kHz (Standard Mode)
    I2C1->TRISE = 9;            // Thời gian cạnh lên tối đa

    // 5. Bật I2C1
    I2C1->CR1 |= I2C_CR1_PE;
}

/* 
 * Thêm chữ "static" vào 3 hàm nội bộ này.
 * Mục đích: Chặn không cho các file khác gọi nhầm, tránh xung đột tên 
 * khi gộp code với các thành viên khác trong nhóm.
 */
static void I2C_WriteByte(uint8_t data) {
    while(I2C1->SR2 & I2C_SR2_BUSY);
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = LCD_ADDR;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1; (void)I2C1->SR2;
    I2C1->DR = data;
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

static void LCD_Send_Cmd(char cmd) {
    char data_u = (cmd & 0xf0), data_l = ((cmd << 4) & 0xf0);
    uint8_t data_t[4] = {data_u | 0x0C, data_u | 0x08, data_l | 0x0C, data_l | 0x08};
    for(int i=0; i<4; i++) I2C_WriteByte(data_t[i]);
}

static void LCD_Send_Data(char data) {
    char data_u = (data & 0xf0), data_l = ((data << 4) & 0xf0);
    uint8_t data_t[4] = {data_u | 0x0D, data_u | 0x09, data_l | 0x0D, data_l | 0x09};
    for(int i=0; i<4; i++) I2C_WriteByte(data_t[i]);
}

/* 
 * 3 hàm bên dưới không có chữ static, đóng vai trò như các "nút bấm" 
 * để file main.c có thể điều khiển màn hình.
 */
void LCD_Init(void) {
    Delay_ms(50);
    LCD_Send_Cmd(0x30); Delay_ms(5);
    LCD_Send_Cmd(0x30); Delay_ms(1);
    LCD_Send_Cmd(0x30); Delay_ms(10);
    LCD_Send_Cmd(0x20); Delay_ms(10);  
    LCD_Send_Cmd(0x28); Delay_ms(1);   
    LCD_Send_Cmd(0x0C); Delay_ms(1);   
    LCD_Send_Cmd(0x01); Delay_ms(5);   
    LCD_Send_Cmd(0x06); Delay_ms(1);   
}

void LCD_String(char *str) { 
    while (*str) LCD_Send_Data(*str++); 
}

void LCD_Set_Cursor(uint8_t row, uint8_t col) { 
    LCD_Send_Cmd((row == 0) ? (0x80 + col) : (0xC0 + col)); 
}

void LCD_PrintNumber(uint32_t num) {
    char buffer[10];
    int i = 0;
    if (num == 0) { LCD_Send_Data('0'); return; }
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        LCD_Send_Data(buffer[--i]);
    }
}