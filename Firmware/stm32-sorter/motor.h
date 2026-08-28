#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f103xb.h"
#include <stdint.h>

// Khai báo biến đếm xung Encoder
extern volatile uint32_t encoder_pulse_count;

// Biến toàn cục để main.c có thể in ra màn hình
extern volatile float current_rpm; 

// Các hàm khởi tạo và điều khiển cơ bản
void Motor_And_Encoder_Init(void);
void Motor_Run_Forward(void);
void Motor_Run_Backward(void);
void Motor_Stop(void);
void Motor_PWM_Init(void);
void Motor_SetSpeed(uint8_t speed_percent);

// ==========================================
// CÁC HÀM CHO THUẬT TOÁN PID
// ==========================================
void Motor_PID_Init(float p, float i, float d, uint32_t ppr);
void Motor_SetTargetRPM(float rpm);
void Motor_PID_Compute(void);

#endif /* MOTOR_H */