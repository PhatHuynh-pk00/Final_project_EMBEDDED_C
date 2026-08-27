# STM32 Sorter Firmware

Firmware chạy trên STM32F103C8T6, xử lý:
- Đọc màu từ TCS3200 (đỏ / xanh lá / xanh dương)
- Đếm sản phẩm qua ngắt ngoài (EXTI) từ cảm biến IR
- Điều khiển động cơ băng chuyền qua L298N (IN1/IN2)
- Điều khiển 2 servo gạt (đỏ, xanh lá); xanh dương đi thẳng
- Cắt nguồn 12V qua Relay khi đếm đủ số lượng
- Hiển thị trạng thái trên LCD1602 qua module I2C (PCF8574)

## Build & Flash

1. Mở project bằng STM32CubeIDE.
2. Build.
3. Nạp bằng ST-Link (giữ UART CP2102 riêng để debug qua UART).

## Pinout (điền theo file .ioc thực tế)

| Chức năng          | Chân STM32 |
|---------------------|------------|
| TCS3200 S0-S3, OUT  | ...        |
| IR sensor (EXTI)    | ...        |
| L298N IN1/IN2       | ...        |
| Servo đỏ / xanh lá  | ...        |
| Relay               | ...        |
| I2C (LCD)           | ...        |

## Ghi chú

- Vị trí gạt sản phẩm được tính bằng thời gian chạy động cơ (delay/timer), không dùng cảm biến vị trí riêng.
- Relay cắt nguồn 12V cấp cho L298N (qua chân VS) khi đếm đủ số lượng sản phẩm — là lớp dừng độc lập với điều khiển IN1/IN2.
