@echo off

echo Dang tien hanh nap code vao STM32...
//muon nap thi sua duong dan di//
"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD -w "C:\Users\Lenovo\Desktop\Final_project_EMBEDDED_C\Firmware\Final_project_EMBEDDED_C\build\Debug\Final_project_EMBEDDED_C.elf" -v -s

if %errorlevel% neq 0 (
    echo. 
    echo [LOI] Nap code that bai! Hay kiem tra lai day cam ST-Link hoac duong dan.
) else (
    echo.
    echo [THANH CONG] Code da duoc nap va mach dang chay!
)

pause