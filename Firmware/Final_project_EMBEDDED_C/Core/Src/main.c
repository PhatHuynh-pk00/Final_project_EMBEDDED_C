/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Hệ thống phân loại sản phẩm băng chuyền
  * @author         : Vũ Thành Đạt - HCMUT
  * @note           : Lập trình Bare-metal (CMSIS) kết hợp HAL Init
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint16_t count_red = 0;
volatile uint16_t count_green = 0;
volatile uint16_t count_blue = 0;
volatile uint16_t count_total = 0;
volatile uint16_t target_count = 10; // Số lượng cài đặt mặc định

volatile uint8_t ir_flag = 0;      
volatile uint8_t debounce_cnt = 0; 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// =========================================================================
// 1. NGẮT EXTI & TIMER CHỐNG NHIỄU (CẢM BIẾN HỒNG NGOẠI PB12)
// =========================================================================
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & (1 << 12)) {
        EXTI->PR |= (1 << 12); // Xóa cờ ngắt
        TIM4->CR1 |= TIM_CR1_CEN; // Khởi động Timer đếm chống nhiễu
        debounce_cnt = 0;
    }
}

void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {
        TIM4->SR &= ~TIM_SR_UIF; // Xóa cờ ngắt
        debounce_cnt++;
        if (debounce_cnt >= 20) {
            TIM4->CR1 &= ~TIM_CR1_CEN; // Tắt Timer
            if (!(GPIOB->IDR & (1 << 12))) { // Nếu vẫn có vật
                ir_flag = 1;
            }
        }
    }
}

// =========================================================================
// 2. ĐIỀU KHIỂN ĐỘNG CƠ & SERVO (BARE-METAL)
// =========================================================================
void Set_Servo_1(uint16_t angle_ms) { TIM3->CCR1 = angle_ms; } // Góc gạt sản phẩm 1
void Set_Servo_2(uint16_t angle_ms) { TIM3->CCR2 = angle_ms; } // Góc gạt sản phẩm 2

void Conveyor_Motor_Run(uint16_t speed) {
    GPIOB->BSRR = (1 << 13);         // IN1 = 1
    GPIOB->BSRR = (1 << (14 + 16));  // IN2 = 0
    TIM3->CCR3 = speed;
}

void Conveyor_Motor_Stop(void) {
    GPIOB->BSRR = (1 << (13 + 16));  // IN1 = 0
    GPIOB->BSRR = (1 << (14 + 16));  // IN2 = 0
    TIM3->CCR3 = 0;
}

void Trigger_Buzzer_Relay(uint8_t state) {
    if (state) {
        GPIOB->BSRR = (1 << 15); // Relay ON
        GPIOA->BSRR = (1 << 8);  // Buzzer ON
    } else {
        GPIOB->BSRR = (1 << (15 + 16)); 
        GPIOA->BSRR = (1 << (8 + 16));  
    }
}

// =========================================================================
// 3. I2C LCD BARE-METAL
// =========================================================================
#define LCD_ADDR 0x4E 

void I2C_WriteByte(uint8_t data) {
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

void LCD_Send_Cmd(char cmd) {
    char data_u = (cmd & 0xf0), data_l = ((cmd << 4) & 0xf0);
    uint8_t data_t[4] = {data_u | 0x0C, data_u | 0x08, data_l | 0x0C, data_l | 0x08};
    for(int i=0; i<4; i++) I2C_WriteByte(data_t[i]);
}

void LCD_Send_Data(char data) {
    char data_u = (data & 0xf0), data_l = ((data << 4) & 0xf0);
    uint8_t data_t[4] = {data_u | 0x0D, data_u | 0x09, data_l | 0x0D, data_l | 0x09};
    for(int i=0; i<4; i++) I2C_WriteByte(data_t[i]);
}

void LCD_Init(void) {
    HAL_Delay(50);
    LCD_Send_Cmd(0x30); HAL_Delay(5);
    LCD_Send_Cmd(0x30); HAL_Delay(1);
    LCD_Send_Cmd(0x30); HAL_Delay(10);
    LCD_Send_Cmd(0x20); HAL_Delay(10);  
    LCD_Send_Cmd(0x28); HAL_Delay(1);   
    LCD_Send_Cmd(0x0C); HAL_Delay(1);   
    LCD_Send_Cmd(0x01); HAL_Delay(5);   
    LCD_Send_Cmd(0x06); HAL_Delay(1);   
}

void LCD_String(char *str) { while (*str) LCD_Send_Data(*str++); }
void LCD_Set_Cursor(uint8_t row, uint8_t col) { LCD_Send_Cmd((row == 0) ? (0x80 + col) : (0xC0 + col)); }

// =========================================================================
// 4. UART BARE-METAL (GIAO TIẾP MÁY TÍNH)
// =========================================================================
void UART_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE)); // Chờ bộ đệm TX trống
    USART1->DR = c;
}

void UART_SendString(char *str) {
    while (*str) UART_SendChar(*str++);
}

// Ngắt nhận UART (Nhận cấu hình số lượng từ máy tính)
void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        char rx_data = USART1->DR; // Đọc dữ liệu nhận được
        
        // Giả sử gửi ký tự số từ '1' đến '9' để set target (Đơn giản hóa cho BTL)
        if (rx_data >= '0' && rx_data <= '9') {
            target_count = rx_data - '0';
            if(target_count == 0) target_count = 10; // Nếu gửi số 0 thì set 10
            
            UART_SendString("\n[OK] Đã cập nhật số lượng cần đếm!\n");
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  // 1. KHỞI ĐỘNG CÁC TIMER BẰNG THANH GHI
  TIM3->CR1 |= TIM_CR1_CEN; 
  TIM3->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E); 
  
  TIM2->CR1 |= TIM_CR1_CEN; // Bật TIM2 đếm Input Capture (TCS3200)

  // 2. CHO PHÉP NGẮT UART NHẬN DỮ LIỆU
  USART1->CR1 |= USART_CR1_RXNEIE; 

  // 3. THIẾT LẬP CƠ CẤU BAN ĐẦU
  Set_Servo_1(1000); // Servo về 0 độ
  Set_Servo_2(1000);
  Trigger_Buzzer_Relay(0);

  // 4. KHỞI TẠO LCD & GIAO DIỆN
  LCD_Init();
  LCD_Set_Cursor(0, 0); LCD_String("R:0 G:0 B:0     ");
  LCD_Set_Cursor(1, 0); LCD_String("Total: 0        ");
  
  UART_SendString("== HE THONG BANG CHUYEN KHOI DONG ==\n");

  // 5. CHẠY BĂNG CHUYỀN
  Conveyor_Motor_Run(10000); // Chạy 50% tốc độ

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  char lcd_buffer[20];
  char uart_buffer[50];
  uint8_t current_color = 0; // 0=None, 1=Red, 2=Green, 3=Blue

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    if (ir_flag == 1) {
        ir_flag = 0; 
        
        Conveyor_Motor_Stop(); 
        HAL_Delay(500); // Dừng lại để cảm biến màu ổn định đọc
        
        // ---------------------------------------------------------
        // LOGIC ĐỌC TCS3200 (Giả lập cấu trúc - Bạn cần cân chỉnh lại)
        // ---------------------------------------------------------
        /* Bạn sẽ cấu hình chân S2, S3 mức logic cao/thấp (thông qua BSRR)
           sau đó đọc tần số trả về từ thanh ghi TIM2->CCR1.
           Dưới đây là khối logic phân loại sau khi đã xác định được màu: */
           
        current_color = 1; // Giả sử hàm đo trả về màu Đỏ (1)
        
        if (current_color == 1) {
            count_red++;
            Set_Servo_1(1500); // Servo 1 gạt 90 độ
        } else if (current_color == 2) {
            count_green++;
            Set_Servo_2(1500); // Servo 2 gạt 90 độ
        } else if (current_color == 3) {
            count_blue++;
            // Để đi thẳng
        }
        
        count_total++;
        HAL_Delay(500); // Chờ servo gạt xong
        
        // Trả servo về vị trí cũ
        Set_Servo_1(1000); 
        Set_Servo_2(1000);

        // ---------------------------------------------------------
        // CẬP NHẬT HIỂN THỊ LCD & GỬI DATA LÊN MÁY TÍNH UART
        // ---------------------------------------------------------
        sprintf(lcd_buffer, "R:%d G:%d B:%d    ", count_red, count_green, count_blue);
        LCD_Set_Cursor(0, 0); LCD_String(lcd_buffer);
        
        sprintf(lcd_buffer, "Total: %d/%d    ", count_total, target_count); 
        LCD_Set_Cursor(1, 0); LCD_String(lcd_buffer);
        
        sprintf(uart_buffer, "SP moi! Tong so: %d (R:%d, G:%d, B:%d)\n", count_total, count_red, count_green, count_blue);
        UART_SendString(uart_buffer);

        // ---------------------------------------------------------
        // KIỂM TRA ĐẠT ĐỦ SỐ LƯỢNG
        // ---------------------------------------------------------
        if (count_total >= target_count) {
            UART_SendString("=> DA DAT DU SO LUONG CAI DAT!\n");
            Trigger_Buzzer_Relay(1); // Bật relay dừng hệ thống tổng & còi
            
            while(count_total >= target_count) {
                // Đợi người dùng gửi lệnh từ máy tính (UART) để thay đổi target_count
                // hoặc bạn có thể thiết kế nút nhấn reset vật lý để set count_total = 0
            }
            
            Trigger_Buzzer_Relay(0); // Tắt còi, mở lại relay
            
            // Xóa màn hình reset về 0
            count_red = 0; count_green = 0; count_blue = 0;
            LCD_Set_Cursor(0, 0); LCD_String("R:0 G:0 B:0     ");
        } else {
            // Nếu chưa đủ, chạy tiếp băng chuyền
            Conveyor_Motor_Run(10000); 
        }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, S0_Pin|S1_Pin|S2_Pin|S3_Pin
                          |GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : S0_Pin S1_Pin S2_Pin S3_Pin
                           PA8 */
  GPIO_InitStruct.Pin = S0_Pin|S1_Pin|S2_Pin|S3_Pin
                          |GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
