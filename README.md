# Hệ Thống Băng Chuyền Phân Loại Sản Phẩm (Color Sorter Conveyor)

Dự án hệ thống cơ điện tử tự động nhận diện và phân loại sản phẩm theo màu sắc (Đỏ, Xanh lá, Xanh dương) sử dụng vi điều khiển STM32F103C8T6. Hệ thống tích hợp xử lý tín hiệu cảm biến, thuật toán điều khiển động cơ và giao diện giám sát thời gian thực.

---

## 1. STM32 Sorter Firmware (Phần Mềm)

Firmware điều khiển trung tâm chạy trên vi điều khiển STM32F103C8T6, đảm nhận các chức năng xử lý:
*   **Đọc cảm biến màu:** Nhận diện phôi Đỏ / Xanh lá / Xanh dương từ cảm biến TCS3200.
*   **Đếm sản phẩm:** Xử lý ngắt ngoài (EXTI) từ cảm biến hồng ngoại (IR) để cập nhật số lượng.
*   **Điều khiển băng chuyền:** Băm xung PWM và xuất tín hiệu qua driver L298N (IN1/IN2) để vận hành động cơ.
*   **Phân loại tự động:** Điều khiển 2 cánh tay Servo gạt phôi Đỏ và Xanh lá vào máng tương ứng; phôi Xanh dương đi thẳng.
*   **Bảo vệ hệ thống:** Kích hoạt Relay ngắt toàn bộ nguồn 12V khi hệ thống đếm đủ chỉ tiêu sản phẩm.
*   **Giao diện giám sát (HMI):** Hiển thị số lượng và trạng thái hoạt động lên màn hình LCD1602 thông qua giao thức I2C (module PCF8574).

### Build & Flash Code
1. Mở project bằng STM32CubeIDE (hoặc VS Code / CMake).
2. Biên dịch (Build) mã nguồn.
3. Nạp firmware bằng mạch nạp ST-Link (Giao tiếp UART qua mạch CP2102 được giữ riêng phục vụ mục đích debug).

### Bảng kết nối chân (Pinout)
*(Cập nhật theo cấu hình thực tế trong file .ioc / file khởi tạo)*

| Chức năng phần cứng | Chân vi điều khiển (STM32) |
| :--- | :--- |
| **TCS3200 (Cảm biến màu)** | S0-S3, OUT: `...` |
| **IR Sensor (Cảm biến tiệm cận)**| EXTI Pin: `...` |
| **L298N (Driver động cơ)** | IN1/IN2: `...` , PWM: `...` |
| **Servo Đỏ / Xanh lá** | PWM Pins: `...` |
| **Module Relay** | GPIO: `...` |
| **I2C (LCD PCF8574)** | SCL: `...` , SDA: `...` |

**Ghi chú thuật toán:** 
*   Vị trí gạt sản phẩm được tính toán bằng độ trễ thời gian (Timer/Delay) di chuyển của động cơ băng chuyền từ mắt cảm biến đến vị trí tay gạt, không sử dụng cảm biến hành trình độc lập.
*   Relay đóng vai trò là lớp bảo vệ vật lý độc lập với tín hiệu IN1/IN2. Khi đạt chỉ tiêu, Relay sẽ trực tiếp cắt nguồn 12V cấp vào chân VS của driver L298N để dừng băng chuyền tuyệt đối.

---

## 2. Electrical & Schematic (Phần Điện)

Sơ đồ nguyên lý toàn hệ thống được thiết kế bằng EasyEDA. Bản vẽ chi tiết được lưu trữ trong thư mục `schematic/`.

### Danh sách linh kiện chính
*   **Vi điều khiển:** STM32F103C8T6 (Blue Pill)
*   **Cảm biến:** TCS3200 (Màu sắc), Cảm biến hồng ngoại IR (Vật cản)
*   **Động lực:** Module L298N (Driver động cơ), Động cơ DC giảm tốc, 2x Động cơ Servo (SG90/MG996R)
*   **Giao tiếp & Hiển thị:** Màn hình LCD1602 kèm module I2C PCF8574, Buzzer 5V, Module UART CP2102
*   **Nguồn cấp:** Mạch giảm áp LM2596 (Hạ áp 12V xuống 5V nuôi hệ thống điều khiển), Module Relay 5V

### Ghi chú thiết kế mạch
*   **Giao tiếp hiển thị:** Mạch LCD1602 và PCF8574 được khai báo chi tiết trong sơ đồ nguyên lý. Các chân P0–P2, P4–P7 của IC PCF8574 được đấu nối trực tiếp vào các chân RS/RW/E/DB4–DB7 của LCD. Biến trở chỉnh độ tương phản đã được tích hợp sẵn trên module.
*   **Hệ thống đếm:** Tín hiệu từ cảm biến IR đếm sản phẩm được đưa thẳng vào chân ngắt (EXTI) của STM32, đảm bảo tốc độ phản hồi không bị suy hao qua các lớp cách ly cơ điện.

---

## 3. Mechanical (Phần Cơ Khí)

Bản vẽ và mô tả kết cấu cơ khí của hệ thống băng chuyền phân loại:
*   **Khung băng chuyền:** _(Cập nhật vật liệu: Mica / Nhôm định hình / Inox, kích thước tổng thể L x W x H)_
*   **Cơ cấu phân loại:** Giá đỡ 2 tay gạt Servo (Đỏ, Xanh lá) được cố định dọc theo hành trình băng chuyền, khoảng cách được tính toán khớp với bộ định thời trong phần mềm.
*   **Bố trí cảm biến:** Trạm quét màu (TCS3200) và trạm đếm (IR sensor) được lắp đặt đầu chu trình và cuối các máng trượt.
*   **Truyền động:** Vị trí lắp đặt động cơ DC giảm tốc kéo Rulo băng chuyền, cơ cấu tăng đai.

---

## 4. Tác giả
*   **Huỳnh Đức Phát** - Đại học Bách Khoa TP.HCM (HCMUT)