# Electrical / Schematic

Sơ đồ nguyên lý được vẽ bằng EasyEDA (pro.easyeda.com).

## Linh kiện chính

- STM32F103C8T6
- TCS3200 (cảm biến màu)
- L298N (driver động cơ băng chuyền)
- Relay, Buzzer
- LCD1602 + module I2C (PCF8574, biến trở chỉnh tương phản tích hợp sẵn trên module)
- LM2596 (hạ áp 12V → 5V)
- CP2102 (UART debug, nạp code dùng ST-Link riêng)

## Ghi chú thiết kế

- LCD1602 + PCF8574 được vẽ chi tiết (không dùng header đại diện): P0–P2, P4–P7 của PCF8574
  nối vào RS/RW/E/DB4–DB7 của LCD.
- Relay dùng để cắt nguồn 12V cấp cho L298N (qua chân VS) khi đếm đủ sản phẩm, làm lớp dừng
  băng chuyền độc lập với điều khiển IN1/IN2.
- IR sensor đếm sản phẩm nối thẳng vào STM32 (không qua relay).

## File trong thư mục `schematic/`

_(liệt kê file schematic export, ảnh mạch PCB khi thêm vào)_
