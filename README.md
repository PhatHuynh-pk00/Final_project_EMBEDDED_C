# Final Project – Embedded C: Product Counting & Color Sorting Conveyor (STM32F103)

Hệ thống đếm và phân loại sản phẩm theo màu trên băng chuyền, sử dụng STM32F103C8T6
kết hợp cảm biến màu TCS3200, cảm biến hồng ngoại đếm sản phẩm (ngắt EXTI), điều khiển
động cơ băng chuyền bằng L298N, gạt sản phẩm bằng servo, hiển thị trạng thái trên LCD1602 (I2C).

## Repository Layout

| Path                        | Vai trò                                                                                                        | Trạng thái          |
|------------------------------|-----------------------------------------------------------------------------------------------------------------|----------------------|
| `Firmware/stm32-sorter/`     | Firmware chính chạy trên STM32F103C8T6: đọc TCS3200, đếm IR (EXTI), điều khiển băng chuyền, servo gạt, LCD      | _(điền trạng thái)_  |
| `Firmware/esp32s3-vision/`   | Mô-đun vision ESP32-S3 (camera + AI phân biệt màu/đếm) — dự án **độc lập**, xem repo riêng nếu đã tách ra       | Đang phát triển / tham khảo |
| `Electrical/`                | Sơ đồ nguyên lý (EasyEDA), file schematic, ảnh mạch                                                             | _(điền trạng thái)_  |
| `Mechanical/`                | Bản vẽ cơ khí băng chuyền, giá đỡ servo, khung máy                                                              | _(điền trạng thái)_  |

## System Architecture

```
   TCS3200 (màu) ──┐
                    │
   IR sensor ───────┼──► STM32F103C8T6 ──► L298N ──► Động cơ băng chuyền
   (đếm, EXTI)      │         │
                    │         ├──► Servo đỏ / Servo xanh lá (gạt sản phẩm)
                    │         ├──► Relay (cắt nguồn 12V khi đủ số lượng)
                    │         └──► LCD1602 (qua PCF8574 I2C) hiển thị trạng thái
```

## Nguyên lý phân loại

- TCS3200 đọc màu sản phẩm khi đi qua vị trí cảm biến.
- Vị trí gạt được xác định bằng thời gian chạy động cơ (delay/timer), không dùng thêm cảm biến vị trí.
- Sản phẩm đỏ / xanh lá → gạt bằng servo tương ứng. Sản phẩm xanh dương đi thẳng.
- IR sensor đếm số lượng sản phẩm qua băng chuyền, tín hiệu nối trực tiếp vào STM32.
- Khi đủ số lượng, Relay cắt nguồn 12V cấp cho L298N để dừng băng chuyền (độc lập với điều khiển IN1/IN2).

## Getting Started

1. Mở `Firmware/stm32-sorter/` bằng STM32CubeIDE (hoặc Keil), build và nạp qua ST-Link.
2. Kết nối phần cứng theo sơ đồ trong `Electrical/schematic/`.
3. Cấp nguồn, quan sát LCD hiển thị trạng thái đếm/phân loại.

## Known Gaps

- _(điền: ví dụ ESP32-S3 vision chưa tích hợp chung với STM32, đang chạy độc lập)_
- _(điền các phần chưa hoàn thiện khác)_

## Hardware

- STM32F103C8T6, TCS3200, L298N, Relay, Buzzer, LCD1602 + PCF8574 (I2C), LM2596, CP2102 (debug UART)
- 2 servo (đỏ, xanh lá), cảm biến IR đếm sản phẩm
