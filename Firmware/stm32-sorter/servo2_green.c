#include "servo2_green.h"

// Hàm ánh xạ tuyến tính nội bộ
static uint16_t map_green(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Servo2_Green_Init(void) {
    // 1. Cấp clock (Dùng toán tử |= để không ghi đè mất thiết lập của Red)
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    // 2. Cấu hình PA7 làm Alternate Function Push-Pull (AF-PP)
    // Xóa 4 bit của PA7 (bit 28-31) và gán mã 0xB (1011)
    GPIOA->CRL &= ~(0xF0000000); 
    GPIOA->CRL |=  (0xB0000000); 

    // 3. Chu kỳ 20ms cho Servo (Khẳng định lại tham số để an toàn)
    TIM3->PSC = 8 - 1;       
    TIM3->ARR = 20000 - 1;   

    // 4. Cấu hình PWM Mode 1 cho Kênh 2 (TIM3_CH2)
    TIM3->CCMR1 &= ~TIM_CCMR1_CC2S;                     // Đặt làm Output
    TIM3->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1; // PWM mode 1
    TIM3->CCMR1 |= TIM_CCMR1_OC2PE;                     // Preload enable

    // Bật tín hiệu ra kênh 2 và kích hoạt Timer
    TIM3->CCER |= TIM_CCER_CC2E;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void Servo2_Green_SetAngle(uint8_t angle) {
    if (angle > 180) angle = 180;
    // Xuất xung từ 500us (0 độ) đến 2500us (180 độ) vào thanh ghi CCR2
    TIM3->CCR2 = map_green(angle, 0, 180, 500, 2500); 
}