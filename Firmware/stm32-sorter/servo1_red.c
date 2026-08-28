#include "servo1_red.h"

// Hàm ánh xạ tuyến tính nội bộ (static để không xung đột với file khác)
static uint16_t map_red(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Servo1_Red_Init(void) {
    // 1. Cấp clock cho TIM3 và GPIOA
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    // 2. Cấu hình PA6 làm Alternate Function Push-Pull (AF-PP)
    // Xóa 4 bit của PA6 (bit 24-27) và gán mã 0xB (1011)
    GPIOA->CRL &= ~(0x0F000000); 
    GPIOA->CRL |=  (0x0B000000); 

    // 3. Chu kỳ 20ms cho Servo
    TIM3->PSC = 8 - 1;       
    TIM3->ARR = 20000 - 1;   

    // 4. Cấu hình PWM Mode 1 cho Kênh 1 (TIM3_CH1)
    TIM3->CCMR1 &= ~TIM_CCMR1_CC1S;                     // Đặt làm Output
    TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // PWM mode 1
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;                     // Preload enable

    // Bật tín hiệu ra kênh 1 và kích hoạt Timer
    TIM3->CCER |= TIM_CCER_CC1E;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void Servo1_Red_SetAngle(uint8_t angle) {
    if (angle > 180) angle = 180;
    // Xuất xung từ 500us (0 độ) đến 2500us (180 độ) vào thanh ghi CCR1
    TIM3->CCR1 = map_red(angle, 0, 180, 500, 2500); 
}