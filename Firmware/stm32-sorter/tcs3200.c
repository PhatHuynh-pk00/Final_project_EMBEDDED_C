#include "tcs3200.h"

typedef enum { COLOR_RED = 0, COLOR_GREEN = 1, COLOR_BLUE = 2 } TCS3200_ColorState_t;
static volatile TCS3200_ColorState_t TCS3200_currentColor = COLOR_RED;
static volatile uint32_t TCS3200_pulse_count = 0; 

volatile uint32_t TCS3200_red_value = 0;
volatile uint32_t TCS3200_green_value = 0;
volatile uint32_t TCS3200_blue_value = 0;

static void TCS3200_ProcessRedState(void) {
    TCS3200_red_value = TCS3200_pulse_count; 
    TCS3200_pulse_count = 0;
    TCS3200_currentColor = COLOR_GREEN;
    GPIOA->BSRR = GPIO_BSRR_BS2 | GPIO_BSRR_BS3; 
}
static void TCS3200_ProcessGreenState(void) {
    TCS3200_green_value = TCS3200_pulse_count; 
    TCS3200_pulse_count = 0;
    TCS3200_currentColor = COLOR_BLUE;
    GPIOA->BSRR = GPIO_BSRR_BR2 | GPIO_BSRR_BS3; 
}
static void TCS3200_ProcessBlueState(void) {
    TCS3200_blue_value = TCS3200_pulse_count; 
    TCS3200_pulse_count = 0;
    TCS3200_currentColor = COLOR_RED;
    GPIOA->BSRR = GPIO_BSRR_BR2 | GPIO_BSRR_BR3; 
}

static void (*TCS3200_color_state_machine[3])(void) = { 
    TCS3200_ProcessRedState, TCS3200_ProcessGreenState, TCS3200_ProcessBlueState 
};

void TCS3200_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    // Chỉ dùng PA2, PA3 làm Output cho chân S2, S3
    GPIOA->CRL &= ~0x0000FF00;  
    GPIOA->CRL |=  0x00003300;  
    
    // Chân OUT (PA4) thiết lập Input và ngắt EXTI4
    GPIOA->CRL &= ~0x000F0000;
    GPIOA->CRL |=  0x00080000;
    GPIOA->BSRR = GPIO_BSRR_BR4; 

    AFIO->EXTICR[1] &= ~AFIO_EXTICR2_EXTI4; 
    EXTI->IMR |= EXTI_IMR_MR4;              
    EXTI->RTSR |= EXTI_RTSR_TR4;            
    EXTI->FTSR &= ~EXTI_FTSR_TR4;           
    
    NVIC_SetPriority(EXTI4_IRQn, 1);
    NVIC_EnableIRQ(EXTI4_IRQn);

    GPIOA->BSRR = GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
    TCS3200_currentColor = COLOR_RED;
}

void EXTI4_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR4) {   
        TCS3200_pulse_count++;
        EXTI->PR = EXTI_PR_PR4;     
    }
}

// Chuyển ngắt thành hàm gọi phần mềm do SysTick điều phối
void TCS3200_RunStateMachine(void) {
    TCS3200_color_state_machine[TCS3200_currentColor]();
}