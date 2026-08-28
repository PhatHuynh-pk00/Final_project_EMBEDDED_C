#include "stm32f103xb.h"

/* --- BIẾN TOÀN CỤC --- */
typedef enum { COLOR_RED = 0, COLOR_GREEN = 1, COLOR_BLUE = 2 } ColorState_t;
volatile ColorState_t currentColor = COLOR_RED;

volatile uint32_t red_value = 0;
volatile uint32_t green_value = 0;
volatile uint32_t blue_value = 0;
volatile uint32_t pulse_count = 0;

/* --- CÁC HÀM TRẠNG THÁI (STATE FUNCTIONS) --- */
void process_red_state(void) {
    red_value = pulse_count; pulse_count = 0;
    currentColor = COLOR_GREEN;
    // Đổi S2 = 1, S3 = 1 (Xanh lá) thông qua thanh ghi BSRR
    GPIOA->BSRR = GPIO_BSRR_BS2 | GPIO_BSRR_BS3; 
}

void process_green_state(void) {
    green_value = pulse_count; pulse_count = 0;
    currentColor = COLOR_BLUE;
    // Đổi S2 = 0, S3 = 1 (Xanh dương)
    GPIOA->BSRR = GPIO_BSRR_BR2 | GPIO_BSRR_BS3; 
}

void process_blue_state(void) {
    blue_value = pulse_count; pulse_count = 0;
    currentColor = COLOR_RED;
    // Đổi S2 = 0, S3 = 0 (Đỏ)
    GPIOA->BSRR = GPIO_BSRR_BR2 | GPIO_BSRR_BR3; 
}

void (*color_state_machine[3])(void) = { process_red_state, process_green_state, process_blue_state };

/* --- KHỞI TẠO PHẦN CỨNG BAREMETAL --- */
void GPIO_Init(void) {
    // 1. Cấp xung nhịp (clock) cho PORT A và AFIO
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    // 2. Cấu hình PA0-PA3 là Output Push-Pull tốc độ 50MHz (Mode = 11, CNF = 00)
    GPIOA->CRL &= ~0x0000FFFF;  // Xóa cấu hình cũ của 4 chân đầu
    GPIOA->CRL |=  0x00003333;  // Cài đặt cấu hình mới

    // 3. Cấu hình PA4 là Input Pull-down (Mode = 00, CNF = 10)
    GPIOA->CRL &= ~0x000F0000;
    GPIOA->CRL |=  0x00080000;
    GPIOA->BSRR = GPIO_BSRR_BR4; // Đưa ODR xuống 0 để kích hoạt Pull-down

    // 4. Cấu hình ngắt ngoài EXTI4 cho chân PA4
    AFIO->EXTICR[1] &= ~AFIO_EXTICR2_EXTI4; // Liên kết EXTI4 với PORT A
    EXTI->IMR |= EXTI_IMR_MR4;              // Bỏ mặt nạ ngắt (Unmask)
    EXTI->RTSR |= EXTI_RTSR_TR4;            // Kích hoạt ngắt sườn lên
    EXTI->FTSR &= ~EXTI_FTSR_TR4;           // Tắt ngắt sườn xuống

    // Cấu hình độ ưu tiên và bật ngắt trong lõi NVIC
    NVIC_SetPriority(EXTI4_IRQn, 1);
    NVIC_EnableIRQ(EXTI4_IRQn);
}

void TIM2_Init(void) {
    // Cấp xung nhịp cho Timer 2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 

    // Tính toán theo Clock hệ thống 72MHz
    TIM2->PSC = 7199;             // Bộ chia: 72MHz / 7200 = 10kHz
    TIM2->ARR = 199;              // Chu kỳ: 200 * 0.1ms = 20ms
    TIM2->DIER |= TIM_DIER_UIE;   // Bật ngắt tràn (Update Interrupt)

    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);

    // Kích hoạt Timer 2
    TIM2->CR1 |= TIM_CR1_CEN; 
}

/* --- TRÌNH PHỤC VỤ NGẮT (ISR) --- */
void EXTI4_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR4) {   // Kiểm tra cờ ngắt có được dựng lên không
        pulse_count++;
        EXTI->PR = EXTI_PR_PR4;     // Xóa cờ ngắt bằng cách ghi giá trị 1
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {    // Kiểm tra cờ tràn Timer
        color_state_machine[currentColor]();
        TIM2->SR &= ~TIM_SR_UIF;    // Xóa cờ tràn
    }
}