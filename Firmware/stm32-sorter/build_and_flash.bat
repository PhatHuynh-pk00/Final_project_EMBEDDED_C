@echo off
setlocal

:: Di chuyển đến đúng thư mục chứa file chạy script để tránh lỗi sai đường dẫn
cd /d "%~dp0"

:: -------------------------------------------------------------------------
:: 5.1 (Clean): Xóa thư mục build cũ
:: -------------------------------------------------------------------------
echo STEP 1: CLEANING BUILD DIRECTORY
if exist build (
    echo Deleting existing build folder...
    rmdir /s /q build
)
:: Tạo lại thư mục build sạch để chuẩn bị cho quá trình mới
mkdir build

:: -------------------------------------------------------------------------
:: 5.2 (Configure): Cấu hình dự án với CMake và Ninja
:: -------------------------------------------------------------------------
echo.
echo STEP 2: CONFIGURING PROJECT WITH CMAKE
cmake -B build -G Ninja
if %ERRORLEVEL% neq 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

:: -------------------------------------------------------------------------
:: 5.3 (Build): Biên dịch với Ninja để tạo file app_firmware.bin
:: -------------------------------------------------------------------------
echo.
echo STEP 3: COMPILING FIRMWARE WITH NINJA
ninja -C build
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

:: -------------------------------------------------------------------------
:: 5.4 (Flash & Reset): Nạp file binary xuống vi điều khiển STM32
:: -------------------------------------------------------------------------
echo.
echo STEP 4: FLASHING FIRMWARE TO TARGET MCU
:: Ràng buộc tuyệt đối: Đường dẫn build/app_firmware.bin, địa chỉ 0x08000000 và cờ -rst
STM32_Programmer_CLI -c port=SWD -w build/app_firmware.bin 0x08000000 -v -rst
if %errorlevel% neq 0 (
    echo [ERROR] Nap Firmware that bai!
    pause
    exit /b %errorlevel%
)

echo [INFO] COMPLETEDDDDDDD!
pause