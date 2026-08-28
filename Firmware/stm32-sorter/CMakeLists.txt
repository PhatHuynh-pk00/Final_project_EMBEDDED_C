cmake_minimum_required(VERSION 3.22)

# 1. Khai báo toolchain từ file ngoài (Chuẩn cấu trúc giống Learning_Startup)
if(NOT CMAKE_TOOLCHAIN_FILE)
    include("cmake/gcc-arm-none-eabi.cmake")
endif()

# Bỏ qua bước linking khi test compiler để tránh lỗi undefined reference
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(FINAL_PROJECT C CXX ASM)

# 2. Cấu hình Cờ biên dịch và Linker tối ưu cho STM32
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -O2 -Wall -Wextra -fdata-sections -ffunction-sections -g -gdwarf-2")
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "${MCU_FLAGS} -Wl,--gc-sections -Wl,-Map=${CMAKE_PROJECT_NAME}.map --specs=nano.specs --specs=nosys.specs")

# 3. Khai báo file Linker Script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F103xx_FLASH.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${LINKER_SCRIPT}")

# 4. Quản lý mã nguồn (Sau này thêm file PWM, L298N, Relay thì điền vào đây)
set(SOURCES
    main.c
    tcs3200.c
    servo1_red.c      
    servo2_green.c    
    LCD.c
    UART.c
    motor.c
    buzzer.c
    Core/Src/system_stm32f1xx.c
)
set(STARTUP_FILE "startup_stm32f103xb.s")

# Thư mục chứa các file .h
include_directories(
    .
    Core/Inc
    Drivers/CMSIS/Include
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
)

# Macro định nghĩa dòng chip
add_compile_definitions(STM32F103xB)

# 5. Build file thực thi (Đã bỏ đuôi .elf cứng ở tên Target)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${STARTUP_FILE})

# Ép CMake sử dụng đúng đuôi .elf cho file đầu ra
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES SUFFIX ".elf")

# 6. Chuyển đổi thành app_firmware.bin để file .bat có thể nạp được
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> app_firmware.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "Generating app_firmware.bin and .hex... Done!"
)