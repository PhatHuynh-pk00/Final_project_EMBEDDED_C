#include "stm32f103xb.h"
#include "tcs3200.h"
#include "servo1_red.h"
#include "servo2_green.h"
#include "buzzer.h"
#include "UART.h"
#include "LCD.h"
#include "motor.h"
#include <stdio.h>

#define SENSITIVITY_OFFSET 15

volatile uint16_t target_red = 5;
volatile uint16_t target_green = 5;
volatile uint16_t target_blue = 5;

// --- HỆ ĐIỀU HÀNH THU NHỎ SYSTICK ---
volatile uint32_t ms_ticks = 0;
extern volatile uint8_t system_start;

void SysTick_Init(void) {
    SysTick->LOAD = 8000 - 1; // 1ms với HSI 8MHz
    SysTick->VAL = 0;
    SysTick->CTRL = 7; 
}

void Delay_ms(uint32_t ms) {
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms);
}

void SysTick_Handler(void) {
    ms_ticks++;

    // Mỗi 20ms cập nhật màu
    if (ms_ticks % 20 == 0) {
        TCS3200_RunStateMachine();
    }
    
    // Mỗi 100ms chạy thuật toán PID
     if (ms_ticks % 100 == 0) {
         if (system_start == 1) {
             Motor_PID_Compute();
         }
    }

// MỖI 100ms ĐỌC ENCODER ĐỂ ĐO TỐC ĐỘ (KHÔNG ĐIỀU KHIỂN MOTOR)
    // if (ms_ticks % 100 == 0) {
    //     if (system_start == 1) {
    //         // 1. Đọc số xung Encoder từ thanh ghi TIM2
    //         int16_t pulses = (int16_t)TIM2->CNT; 
    //         TIM2->CNT = 0; // Đọc xong reset ngay lập tức
            
    //         // 2. Chống lỗi số âm nếu motor bị giật lùi
    //         if (pulses < 0) pulses = -pulses;
            
    //         // 3. Tính RPM (234 là độ phân giải xung, nhân 4 vì xài Hardware Encoder)
    //         current_rpm = ((float)pulses * 600.0) / (234.0 * 4.0);
    //     }
    // }
}
// ------------------------------------

void LED_PC13_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0x00F00000);
    GPIOC->CRH |=  (0x00300000);
    GPIOC->BSRR = GPIO_BSRR_BS13;
}

void Relay_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRH &= ~(0x00000F00);
    GPIOB->CRH |=  (0x00000300);
    GPIOB->BSRR = GPIO_BSRR_BR10; 
}

void Update_LCD_Display(uint32_t red, uint32_t green, uint32_t blue) {
    uint32_t total = red + green + blue;
    LCD_Set_Cursor(0, 0);
    LCD_String("R:"); LCD_PrintNumber(red);
    LCD_String(" G:"); LCD_PrintNumber(green);
    LCD_String(" B:"); LCD_PrintNumber(blue);
    LCD_String("      ");
    LCD_Set_Cursor(1, 0);
    LCD_String("TOTAL: "); LCD_PrintNumber(total);
    LCD_String("       ");
}

int main(void) {
    SysTick_Init();
    Buzzer_Init(); 
    LED_PC13_Init();
    Relay_Init();
    UART1_Init();
    I2C1_Init();
    LCD_Init();
    TCS3200_Init();
    Servo1_Red_Init();
    Servo2_Green_Init();
    Motor_And_Encoder_Init();
    Motor_PWM_Init();

    Motor_PID_Init(0.3, 0.02, 0.01, 234);

    Servo1_Red_SetAngle(180); 
    Servo2_Green_SetAngle(180);


    uint16_t debounce_count = 0;
    uint8_t is_object_present = 0;
    uint32_t total_red = 0, total_green = 0, total_blue = 0;
    uint32_t base_r = 0, base_g = 0, base_b = 0;
    
    while (system_start == 0) {
        Delay_ms(100); // Đứng im lìm ở đây chờ bạn nhập dữ liệu và ấn 'S'
    }
    
    LCD_Set_Cursor(0, 0);
    LCD_String("  H.THONG SAN  ");
    LCD_Set_Cursor(1, 0);
    LCD_String(" SANG HOAT DONG ");

    // while (TCS3200_red_value == 0 || TCS3200_green_value == 0 || TCS3200_blue_value == 0) {
    //     Delay_ms(10);
    // }
    // Delay_ms(200);
    base_r = TCS3200_red_value;
    base_g = TCS3200_green_value;
    base_b = TCS3200_blue_value;

    Update_LCD_Display(total_red, total_green, total_blue);

    Motor_Run_Forward();       
    Motor_SetTargetRPM(75.0);
    // Biến tạo độ trễ Non-blocking cho việc in UART
    uint32_t last_uart_print = ms_ticks; 
    uint32_t servo_delay_start = 0;
    uint32_t servo_hold_start = 0;
    uint8_t pending_color = 0;
    uint8_t servo_is_gating = 0;

    while (1) {
        if (system_start == 0) {
            Motor_Stop();                     // Dừng động cơ
            Servo1_Red_SetAngle(180);           // Thu cần gạt Red
            Servo2_Green_SetAngle(180);         // Thu cần gạt Green
            GPIOB->BSRR = GPIO_BSRR_BR10;     // Tắt Relay

            // Chờ người dùng nhấn 'S' để tiếp tục chạy lại
            while (system_start == 0) {
                Delay_ms(100);
            }
            Motor_PID_Init(0.3, 0.02, 0.01, 234);
            // Chạy lại hệ thống khi nhấn 'S'
            LCD_Set_Cursor(0, 0);
            LCD_String("  H.THONG SAN  ");
            LCD_Set_Cursor(1, 0);
            LCD_String(" SANG HOAT DONG ");
            
            Update_LCD_Display(total_red, total_green, total_blue);
            Motor_Run_Forward();
            Motor_SetTargetRPM(75.0);
        }

        // In tốc độ RPM mỗi 200ms bằng Non-blocking
        if (ms_ticks - last_uart_print >= 200) {
            last_uart_print = ms_ticks;
            UART_SendString("Toc do RPM: ");
            UART_SendNumber((uint32_t)current_rpm);
            UART_SendString("\r\n");

            // In thêm giá trị cảm biến màu
            UART_SendString(" | R: "); UART_SendNumber(TCS3200_red_value);
            UART_SendString(" G: "); UART_SendNumber(TCS3200_green_value);
            UART_SendString(" B: "); UART_SendNumber(TCS3200_blue_value);
            UART_SendString("\r\n");
        }

        uint8_t current_color = 0;
        if (TCS3200_red_value > (base_r + SENSITIVITY_OFFSET) &&
            TCS3200_red_value > TCS3200_green_value &&
            TCS3200_red_value > TCS3200_blue_value) {
            current_color = 1;
        }
        else if (TCS3200_green_value > (base_g + SENSITIVITY_OFFSET) &&
                 TCS3200_green_value > TCS3200_red_value &&
                 TCS3200_green_value > TCS3200_blue_value) {
            current_color = 2;
        }
        else if (TCS3200_blue_value > (base_b + SENSITIVITY_OFFSET) &&
                 TCS3200_blue_value > TCS3200_red_value &&
                 TCS3200_blue_value > TCS3200_green_value) {
            current_color = 3;
        }

        // 1. ĐỌC CẢM BIẾN (Chỉ ghi nhớ màu và bấm giờ, KHÔNG gạt Servo)
        if (current_color != 0) {
            if (debounce_count < 20) debounce_count++; // Giảm mức đếm xuống cho nhạy

            if (debounce_count >= 10 && is_object_present == 0) {
                is_object_present = 1;
                GPIOC->BSRR = GPIO_BSRR_BR13; // Bật LED báo hiệu

                // Ghi nhớ màu và thời điểm phát hiện để chờ phôi chạy tới Servo
                pending_color = current_color;
                servo_delay_start = ms_ticks; 
            }
        } else {
            if (debounce_count > 0) debounce_count--;

            if (debounce_count == 0 && is_object_present == 1) {
                is_object_present = 0;
                GPIOC->BSRR = GPIO_BSRR_BS13; // Tắt LED
                // BỎ LỆNH THU SERVO Ở ĐÂY VÌ PHÔI VẪN ĐANG TRÊN ĐƯỜNG ĐI
            }
        }

// 2. BỘ HẸN GIỜ GẠT SERVO (Tách riêng quãng đường cho từng màu)
        uint8_t ready_to_sort = 0; // Cờ báo hiệu phôi đã tới đúng vị trí gạt

        // Phôi Đỏ: Cần thời gian di chuyển tới Servo 1 (VD: 1000ms)
        if (pending_color == 1 && (ms_ticks - servo_delay_start >= 800)) { 
            total_red++;
            Servo1_Red_SetAngle(60);
            ready_to_sort = 1;
        }
        // Phôi Xanh Lá: Nằm xa hơn nên cần thời gian lâu hơn tới Servo 2 (VD: 2200ms)
        else if (pending_color == 2 && (ms_ticks - servo_delay_start >= 1500)) { 
            total_green++;
            Servo2_Green_SetAngle(60);
            ready_to_sort = 1;
        }
        // Phôi Xanh Dương: Đi thẳng xuống cuối băng chuyền (Delay 500ms cho qua mắt cảm biến rồi đếm)
        else if (pending_color == 3 && (ms_ticks - servo_delay_start >= 500)) { 
            total_blue++; 
            ready_to_sort = 1;
        }

        // Khi phôi đã rớt vào máng, chuyển sang trạng thái giữ Servo và kiểm tra kết thúc
        if (ready_to_sort == 1) {
            Update_LCD_Display(total_red, total_green, total_blue);
            
            pending_color = 0; 
            servo_is_gating = 1;
            servo_hold_start = ms_ticks; // Bắt đầu tính thời gian giữ cần gạt 0.5s
        }

        // 3. BỘ HẸN GIỜ THU SERVO (Giữ 0.5 giây rồi thu về)
        if (servo_is_gating == 1 && (ms_ticks - servo_hold_start >= 500)) { // 500ms = 0.5s
            Servo1_Red_SetAngle(180);
            Servo2_Green_SetAngle(180);
            servo_is_gating = 0;

            // 4. KIỂM TRA ĐẠT CHỈ TIÊU KHI ĐÃ ĐÓNG GÓI XONG 1 PHÔI
            if (total_red >= target_red && total_green >= target_green && total_blue >= target_blue) {
                Delay_ms(3500);
                Buzzer_LongBeep(500);
                Motor_Stop();
                GPIOB->BSRR = GPIO_BSRR_BS10; // Kích Relay 

                GPIOB->BSRR = GPIO_BSRR_BR10; // Đóng Relay 
                total_red = 0; total_green = 0; total_blue = 0;
                encoder_pulse_count = 0;
                system_start = 0;
                Update_LCD_Display(total_red, total_green, total_blue);
                Motor_Run_Forward();
            }
        }
    }
}