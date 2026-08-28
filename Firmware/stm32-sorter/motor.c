#include "motor.h"

volatile uint32_t encoder_pulse_count = 0;
volatile float current_rpm = 0.0;
float Kp = 0.0, Ki = 0.0, Kd = 0.0;
float target_rpm = 0.0;
float e_integral = 0.0;
float e_prev = 0.0;
uint32_t motor_ppr = 234;

void Motor_And_Encoder_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Bật TIM2 cho Encoder

    // 1. IN1, IN2 ra L298N (PB0, PB1 - Output)
    GPIOB->CRL &= ~(0x000000FF);
    GPIOB->CRL |=  (0x00000033);

    // 2. Chân Encoder Pha A (PA0) và Pha B (PA1) - Input Floating
    GPIOA->CRL &= ~(0x000000FF);
    GPIOA->CRL |=  (0x00000044);

    // 3. Khởi tạo TIM2 chế độ Hardware Encoder (Mode 3 - Đọc cả 2 kênh)
    TIM2->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;
    TIM2->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
    TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P); // Bắt sườn lên
    TIM2->ARR = 0xFFFF; // Đặt giá trị tràn tối đa
    TIM2->CR1 |= TIM_CR1_CEN;
    
    // (ĐÃ XÓA NGẮT EXTI0 BỊ DỞ)
}

void Motor_PWM_Init(void) {
    // Chân ENA ở PA11 thuộc Kênh 4 của Timer 1
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    // Cấu hình PA11 là Alternate Function Push-Pull
    GPIOA->CRH &= ~(0x0000F000);
    GPIOA->CRH |=  (0x0000B000);

    TIM1->PSC = 80 - 1;
    TIM1->ARR = 100 - 1;

    // Bật PWM Mode 1 trên Kênh 4 (CH4)
    TIM1->CCMR2 &= ~(TIM_CCMR2_OC4M);
    TIM1->CCMR2 |= (0x6 << 12);
    TIM1->CCMR2 |= TIM_CCMR2_OC4PE;

    TIM1->CCER |= TIM_CCER_CC4E;
    TIM1->BDTR |= TIM_BDTR_MOE; // Main Output Enable (Cực kỳ quan trọng với TIM1)
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM1->CCR4 = 0; 
}

void Motor_SetSpeed(uint8_t speed_percent) {
    if (speed_percent > 100) speed_percent = 100;
    TIM1->CCR4 = speed_percent;
}

void Motor_Run_Forward(void)  { GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BR1; }
void Motor_Run_Backward(void) { GPIOB->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BS1; }
void Motor_Stop(void)         { GPIOB->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1; }

void Motor_PID_Init(float p, float i, float d, uint32_t ppr) {
    Kp = p; Ki = i; Kd = d;
    motor_ppr = ppr;
}

void Motor_SetTargetRPM(float rpm) {
    target_rpm = rpm;
    if (rpm == 0) e_integral = 0;
}

void Motor_PID_Compute(void) {
    // Lấy số xung trực tiếp từ thanh ghi đếm của TIM2
    int16_t pulses = (int16_t)TIM2->CNT; 
    TIM2->CNT = 0; // Đọc xong reset ngay
    
    // Trị tuyệt đối để chống lỗi nếu motor giật lùi
    if (pulses < 0) pulses = -pulses;
    
    encoder_pulse_count += pulses; 

    // Tính RPM. Tự động chia 4 vì Hardware Encoder đã nhân 4 độ phân giải
    current_rpm = ((float)pulses * 600.0) / ((float)motor_ppr * 4.0);

    float error = target_rpm - current_rpm;
    e_integral += error;
    if (e_integral > 2000.0) e_integral = 2000.0;
    if (e_integral < -2000.0) e_integral = -2000.0;

    float derivative = error - e_prev;
    e_prev = error;

    float u = (Kp * error) + (Ki * e_integral) + (Kd * derivative);
    if (u > 0.5) {
        u += 15.0; 
    }
    if (u > 100.0) u = 100.0;
    if (u < 0.0) u = 0.0;

    if (target_rpm > 0.1) Motor_SetSpeed((uint8_t)u);
    else Motor_SetSpeed(0);
}