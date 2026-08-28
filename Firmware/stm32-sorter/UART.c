#include "UART.h"

/* Khai báo mượn 3 biến target từ main.c */
extern volatile uint16_t target_red;
extern volatile uint16_t target_green;
extern volatile uint16_t target_blue;
volatile uint8_t system_start = 0;
// Biến tĩnh nội bộ để nhớ trạng thái cài đặt
static char current_color_setting = 0; 
static uint16_t temp_value = 0; // Biến tạm để cộng dồn các chữ số

void UART1_Init(void) {
    // 1. Cấp xung nhịp cho USART1 và GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    
    // 2. Cấu hình chân
    // PA9 (TX): Alternate Function Push-Pull (AF-PP), 50MHz -> Mã 0xB
    // PA10 (RX): Input Floating -> Mã 0x4
    GPIOA->CRH &= ~(0x00000FF0);
    GPIOA->CRH |=  (0x000004B0);
    
    // 3. Baudrate 9600
    USART1->BRR = 833; 
    
    // 4. Bật TX, RX, ngắt nhận và kích hoạt USART1
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;
    
    // 5. Bật ngắt trên NVIC
    NVIC_EnableIRQ(USART1_IRQn);
}

void UART_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE)); 
    USART1->DR = c;
}

void UART_SendString(char *str) {
    while (*str) UART_SendChar(*str++);
}

// Hàm bổ sung: Tách các chữ số để in ra màn hình Serial
void UART_SendNumber(uint32_t num) {
    char buffer[10];
    int i = 0;
    if (num == 0) { UART_SendChar('0'); return; }
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        UART_SendChar(buffer[--i]);
    }
}

/*
 * Hàm ngắt nhận dữ liệu UART (Hỗ trợ nhập 0 - 99)
 */
void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        char rx_data = USART1->DR; 
        
        if (rx_data == 'F' || rx_data == 'f') {
            system_start = 0; 
            current_color_setting = 0; // Hủy ngay trạng thái chọn màu
            temp_value = 0;            // Xóa bộ nhớ tạm
            UART_SendString("\n\n>>> TAM DUNG HE THONG BAN CHUYEN! <<<\n");
            return;
        }

        // --- CHỐT CHẶN: Nếu hệ thống đã chạy, không cho gõ phím linh tinh nữa ---
        if (system_start == 1) {
            return; // Bỏ qua mọi phím bấm
        }

        // BƯỚC 0: BẮT TÍN HIỆU START KHI BẤM CHỮ 'S' HOẶC 's'
        if (rx_data == 'S' || rx_data == 's') {
            system_start = 1; // Bật cờ lên 1
            UART_SendString("\n\n>>> LET'S GO! BANG CHUYEN BAT DAU CHAY! <<<\n");
            return; // Thoát ngắt ngay lập tức, bỏ qua phím Enter nếu có bấm kèm
        }
        
        // BƯỚC 1: Chọn màu cần cài đặt
        else if (rx_data == 'R' || rx_data == 'r') {
            current_color_setting = 'R';
            temp_value = 0; 
            UART_SendString("\n>> Chon [DO]. Nhap so luong (0-99): ");
        } 
        else if (rx_data == 'G' || rx_data == 'g') {
            current_color_setting = 'G';
            temp_value = 0;
            UART_SendString("\n>> Chon [X.LA]. Nhap so luong (0-99): ");
        } 
        else if (rx_data == 'B' || rx_data == 'b') {
            current_color_setting = 'B';
            temp_value = 0;
            UART_SendString("\n>> Chon [X.DUONG]. Nhap so luong (0-99): ");
        }
        
        // BƯỚC 2: Nhận từng chữ số
        else if (rx_data >= '0' && rx_data <= '9') {
            temp_value = temp_value * 10 + (rx_data - '0');
            if (temp_value > 99) temp_value = 99;
            UART_SendChar(rx_data); // Vọng lại phím gõ
        }
        
        // BƯỚC 3: Xác nhận lưu khi bấm phím Enter (\r hoặc \n)
        else if (rx_data == '\r' || rx_data == '\n') {
            // Chỉ lưu nếu thực sự có nhập số (tránh lỗi ấn Enter 2 lần liên tục)
            if (current_color_setting == 'R') {
                target_red = temp_value;
                UART_SendString("\n[OK] Target DO = ");
            } 
            else if (current_color_setting == 'G') {
                target_green = temp_value;
                UART_SendString("\n[OK] Target X.LA = ");
            } 
            else if (current_color_setting == 'B') {
                target_blue = temp_value;
                UART_SendString("\n[OK] Target X.DUONG = ");
            }
            
            UART_SendNumber(temp_value);
            UART_SendString("\n");
            temp_value = 0; // Xóa tạm chờ màu sau
        }
    } 
}