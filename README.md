# HỆ THỐNG BĂNG CHUYỀN PHÂN LOẠI MÀU – BARE-METAL STM32

Hệ thống băng chuyền phân loại màu sắc chạy trên nền tảng STM32 theo phương pháp lập trình thanh ghi trực tiếp (Bare-metal).

## 1. Tổng quan dự án

Dự án này triển khai hệ thống phân loại màu sắc tự động trên vi điều khiển STM32.
Hệ thống nhận dữ liệu màu RGB từ cảm biến TCS3200, và điều khiển động cơ DC băng chuyền thông qua driver L298N tích hợp băm xung PWM.

Tương tác người dùng được thực hiện qua giao tiếp UART để cài đặt chỉ tiêu số lượng, trong khi hệ thống liên tục cập nhật số đếm lên màn hình LCD1602 giao tiếp I2C.
Khi đạt đủ số lượng chỉ tiêu phân loại, một relay phần cứng sẽ tự động cắt nguồn cấp 12V cho driver động cơ như một lớp dừng bảo vệ an toàn độc lập.

## 2. Mục tiêu học tập

- Thực hành lập trình Bare-metal trên STM32.
- Hiểu sâu về các ngoại vi EXTI, I2C, PWM (TIMERS), UART và SysTick ở cấp độ thanh ghi.
- Áp dụng máy trạng thái hữu hạn để cấu trúc hóa logic điều khiển cảm biến nhúng.
- Thiết kế vòng lặp chính (main loop) không chặn (non-blocking) sử dụng các sự kiện theo thời gian.
- Nâng cao kỹ năng viết tài liệu và tư duy hệ thống cho các dự án nhúng.

## 3. Tổng quan phần cứng

Vi điều khiển: STM32F103C8T6.

Xung nhịp: Thạch anh nội 8 MHz.

Hiển thị: Màn hình LCD1602 kèm module I2C PCF8574.

Thiết bị đầu vào: Cảm biến màu TCS3200.

Thiết bị đầu ra: 2x Động cơ Servo SG90, Driver động cơ L298N, Relay 5V, Còi Buzzer.

Công cụ nạp: ST-Link.

Phong cách phát triển: Bare-metal.

## 4. Sơ đồ chân và nguyên lý

Sơ đồ nguyên lý dưới đây thể hiện việc kết nối chân giữa STM32, màn hình LCD, các cảm biến, driver động cơ và các servo.

<p align="center">
  <img src="docs/schematic.jpg" alt="Color Sorter - Pinout Schematic" width="700">
</p>

## 5. Kiến trúc phần mềm

Phần mềm được xây dựng dựa trên vòng lặp chính không chặn và nhiều máy trạng thái.
Các tác vụ yêu cầu thời gian chính xác được điều khiển bởi ngắt timer, trong khi logic hệ thống duy trì sự ổn định và dễ theo dõi.
Sự tách biệt này giúp cải thiện khả năng bảo trì và dễ dàng phân tích hành vi hệ thống.

### 5.1 Máy trạng thái tổng quan hệ thống

Sơ đồ này mô tả luồng thực thi tổng thể của toàn bộ hệ thống.

stateDiagram-v2
    direction LR
    %% ÉP CỠ CHỮ TO
    %%{init: {'themeVariables': {'fontSize': '16px'}}}%%
    
    %% Định nghĩa các bảng màu
    classDef idle fill:#E0E0E0,stroke:#9E9E9E,stroke-width:2px,color:black
    classDef running fill:#C8E6C9,stroke:#4CAF50,stroke-width:2px,color:black
    classDef target fill:#FFF9C4,stroke:#FBC02D,stroke-width:2px,color:black
    
    [*] --> INIT : Cấp nguồn
    
    INIT --> STANDBY : Khởi tạo phần cứng xong
    
    STANDBY --> RUNNING : Nhận phím 'S' qua UART<br>(system_start = 1)
    
    RUNNING --> STANDBY : Tạm dừng
    
    RUNNING --> TARGET_REACHED : Đếm đủ số lượng chỉ tiêu
    
    TARGET_REACHED --> STANDBY : Tự động Reset sau 3s<br>(system_start = 0)
    
    %% Gán màu cho các khối
    class INIT, STANDBY idle
    class RUNNING running
    class TARGET_REACHED target

### 5.2 Máy trạng thái logic phân loại

Sơ đồ này tập trung vào logic phân loại bên trong và các trạng thái chấp hành.
Mỗi khối trạng thái đại diện cho tập hợp các tập lệnh liên quan chịu trách nhiệm cho một hành vi cụ thể (ví dụ: phát hiện vật cản, độ trễ không chặn, điều khiển tay gạt servo).

flowchart TD
    %% ÉP CỠ CHỮ TO VÀ GIÃN CÁCH KHỐI
    %%{init: {'themeVariables': {'fontSize': '16px'}, 'flowchart': {'nodeSpacing': 60, 'rankSpacing': 60}}}%%

    %% Định nghĩa các bảng màu
    classDef startEnd fill:#E1BEE7,stroke:#8E24AA,stroke-width:2px,color:black
    classDef process fill:#BBDEFB,stroke:#1976D2,stroke-width:2px,color:black
    classDef decision fill:#FFE082,stroke:#F57C00,stroke-width:2px,color:black
    classDef action fill:#C8E6C9,stroke:#388E3C,stroke-width:2px,color:black
    classDef stop fill:#FFCDD2,stroke:#D32F2F,stroke-width:2px,color:black

    Start(("Vòng lặp Main")):::startEnd --> ReadSensor["Đọc cảm biến màu TCS3200"]:::process
    
    ReadSensor --> CheckObject{"Phát hiện màu<br>& Chống dội OK?"}:::decision
    
    CheckObject -- "Có (Object = 1)" --> SaveColor["Ghi nhớ Màu (pending_color)<br>Bắt đầu bấm giờ (delay_start)"]:::process
    CheckObject -- "Không" --> CheckTimer1
    
    SaveColor --> CheckTimer1{"Đến mốc thời gian gạt?<br>(Đỏ: 1s, Xanh lá: 2.2s)"}:::decision
    
    CheckTimer1 -- "Có" --> ActuateServo["Gạt Servo tương ứng lên 90°<br>Tăng biến đếm màu ++"]:::action
    ActuateServo --> StartHold["Bắt đầu bấm giờ giữ (hold_start)"]:::process
    CheckTimer1 -- "Không" --> CheckTimer2
    
    StartHold --> CheckTimer2{"Đã giữ đủ 0.5s?"}:::decision
    
    CheckTimer2 -- "Có" --> ResetServo["Thu Servo về 0°"]:::action
    CheckTimer2 -- "Không" --> CheckTarget
    
    ResetServo --> CheckTarget
    
    CheckTarget{"Biến đếm >= Chỉ tiêu?"}:::decision
    
    CheckTarget -- "Chưa đạt" --> Start
    
    CheckTarget -- "Đã đạt" --> Buzzer["Còi hú 3.5s<br>(Băng chuyền vẫn đẩy phôi)"]:::stop
    Buzzer --> StopMotor["Cúp Relay 12V<br>Dừng động cơ"]:::stop
    StopMotor --> LCD["Hiển thị LCD 'DAT CHI TIEU'<br>Chờ 3 giây"]:::process
    LCD --> AutoReset["Reset toàn bộ biến đếm về 0<br>system_start = 0"]:::process
    AutoReset --> End(("Thoát vòng lặp<br>Chờ lệnh mới")):::startEnd

## 6. Thiết kế ngắt và định thời

Ngắt phần cứng SysTick được sử dụng làm cơ sở thời gian cho hệ thống (chu kỳ ngắt 1ms).
Tín hiệu cảm biến (TCS3200) được lấy mẫu định kỳ (mỗi 20ms) thông qua máy trạng thái để đảm bảo điều khiển thời gian thực và phản hồi nhanh.
Việc đếm sản phẩm được xử lý hoàn toàn bằng ngắt phần cứng EXTI để có phản hồi tức thì.
Vòng lặp chính duy trì trạng thái không chặn và phản hồi theo các thay đổi trạng thái.
Thời gian chờ gạt servo (1000ms cho Đỏ, 2200ms cho Xanh lá) và thời gian giữ cần gạt (500ms) được tách biệt khỏi bộ điều khiển động cơ chính.
Phương pháp này đảm bảo thời gian dự đoán được và tránh các độ trễ gây treo hệ thống.

## 7. Video Demo

Video ngắn ghi lại quá trình phân loại và hoàn thành chỉ tiêu:

▶️ [Color Sorter Conveyor - Demo Video](https://www.youtube.com/watch?v=gpbj189OsZc)

## 8. Biên dịch và nạp code

Bộ công cụ biên dịch (Toolchain): arm-none-eabi-gcc.

Hệ thống build: CMake.

Mạch nạp: ST-Link.

Không sử dụng thư viện của hãng hay trình sinh mã nguồn tự động.

## 9. Lý do chọn Bare-Metal?

Dự án này chủ yếu phục vụ mục đích học tập, giúp hiểu sâu cách cấu hình và điều khiển các ngoại vi ở cấp độ thanh ghi.

## 10. Đánh đổi hiệu năng và thời gian

Hệ thống ưu tiên tính ổn định và định thời chính xác cho việc phân loại sản phẩm.
Vì cơ cấu phân loại dựa vào độ trễ thời gian không chặn chính xác thay vì cảm biến vị trí độc lập tại mỗi máng, hệ thống có giới hạn cơ khí về tốc độ nạp phôi.
Các sản phẩm phải được đặt lên băng chuyền với khoảng cách tối thiểu từ 1.5 đến 2 giây. Nạp sản phẩm quá sát nhau sẽ khiến biến lưu trữ trạng thái bị ghi đè, dẫn đến việc servo không gạt kịp.

## 11. Hạn chế và hướng phát triển trong tương lai

Dự án này chủ yếu mang tính chất học tập nên vẫn còn một số hạn chế:
- Thiếu sự ổn định do thiết kế cơ khí chưa chắc chắn
- Có thể phân loại sai do cơ chế cài đặt thời gian gạt cố định
- Tốc độ băng chuyền chưa ổn định

Các hướng cải tiến và phát triển trong tương lai bao gồm:
- Thay thế ngoại vị nhận diện phôi bằng camera AI.
- Điều chỉnh cơ cấu cơ khí để chắc chắn hơn.

## 12. Tác giả

Tác giả: Huỳnh Đức Phát, Vũ Thành Đạt, Hoàng Anh