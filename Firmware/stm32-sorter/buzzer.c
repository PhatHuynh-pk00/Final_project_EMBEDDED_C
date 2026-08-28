#include "buzzer.h"

// Mượn hàm Delay từ main.c
extern void Delay_ms(uint32_t ms); 

void Buzzer_Init(void) {
    // 1. Cấp clock cho PORTB
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    // 2. Cấu hình PB11 làm Output Push-Pull, tốc độ 50MHz
    // PB11 nằm ở các bit từ 12 đến 15 trên thanh ghi CRH
    GPIOB->CRH &= ~(0x0000F000); 
    GPIOB->CRH |=  (0x00003000); 
    
    // 3. Trạng thái ban đầu: Tắt còi (PB11)
    GPIOB->BSRR = GPIO_BSRR_BS11; 
}

void Buzzer_On(void) {
    // Đổi BS11 thành BR11 (Kéo xuống THẤP để còi kêu)
    GPIOB->BSRR = GPIO_BSRR_BR11; 
}

void Buzzer_Off(void) {
    // Đổi BR11 thành BS11 (Kéo lên CAO để tắt còi)
    GPIOB->BSRR = GPIO_BSRR_BS11; 
}

// Hàm tạo tiếng bíp ngắt quãng
void Buzzer_Beep(uint8_t times, uint16_t duration_ms) {
    for (uint8_t i = 0; i < times; i++) {
        Buzzer_On();
        Delay_ms(duration_ms);
        Buzzer_Off();
        
        if (i < (times - 1)) {
            Delay_ms(150); 
        }
    }
}

// Hàm tạo tiếng kêu dài liên tục
void Buzzer_LongBeep(uint16_t duration_ms) {
    Buzzer_On();
    Delay_ms(duration_ms);
    Buzzer_Off();
}