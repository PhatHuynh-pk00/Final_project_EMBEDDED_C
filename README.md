# COLOR SORTER CONVEYOR – Bare-Metal STM32

A bare-metal color sorting conveyor system implemented on STM32 using direct register-level programming (no HAL, no RTOS).
This project focuses on deterministic timing, interrupt-driven design, and finite state machines for sensor logic.

## 1. Project Overview

This project implements a bare-metal color sorting system on STM32.
The system receives RGB color data from a TCS3200 sensor, tracks passing objects via an IR sensor using external interrupts (EXTI), and controls a DC motor conveyor via a PWM-driven L298N driver.

User interaction is handled via UART to set target quotas, while the system continuously updates the current count on an I2C LCD1602 display.
When the sorting quota is met, a hardware relay cuts the 12V power supply to the motor driver as an independent safety stop.

## 2. Learning Objectives

- Practice bare-metal programming on STM32 without using HAL or RTOS.
- Understand EXTI, I2C, PWM (TIMERS), UART, and SysTick at the register level.
- Apply finite state machines to structure embedded sensor logic.
- Design a non-blocking main loop using time-driven events.
- Improve documentation and system-level thinking for embedded projects.

## 3. Hardware Overview

MCU: STM32F103C8T6.

Clock: 8 MHz internal clock (default, stable mode).

Display: LCD1602 with PCF8574 (I2C).

Input: TCS3200 Color Sensor and IR Proximity Sensor.

Output: 2x SG90/MG996R Servos, L298N Motor Driver, 5V Relay, Buzzer.

Programming: ST-Link.

Development style: Bare metal (direct register access).

## 4. Pinout Schematic

The following schematic shows the pin mapping between the STM32, LCD display, sensors, motor driver, and servos.

<p align="center">
  <img src="docs/images/Color_Sorter_schematic.svg" alt="Color Sorter - Pinout Schematic" width="700">
</p>

## 5. Software Architecture

The software is structured around a non-blocking main loop and multiple state machines.
Timing-critical tasks are driven by timer interrupts, while system logic remains deterministic and easy to follow.
This separation improves maintainability and makes system behavior easier to reason about.

### 5.1 Overall System State Machine

This diagram describes the overall execution flow of the system.

<p align="center">
  <img src="docs/images/Color_Sorter_general.svg" alt="Color Sorter - General System State Machine" width="700">
</p>

### 5.2 Sorter Logic State Machine

This diagram focuses on the internal sorting logic and actuation states.
Each state block represents a group of related instructions responsible for a specific behavior (e.g. object detection, non-blocking delay, servo gating).

<p align="center">
  <img src="docs/images/Color_Sorter_detail.svg" alt="Color Sorter - Detail System State Machine" width="1000">
</p>

## 6. Timing and Interrupt Design

A hardware SysTick interrupt is used as the system time base (1ms tick).
Sensor inputs (TCS3200) are sampled periodically (every 20ms) via a state machine to ensure responsive and real-time control.
Product counting is handled strictly via EXTI hardware interrupts for immediate response.
The main loop remains non-blocking and reacts to state changes.
Servo gating delays (1000ms for Red, 2200ms for Green) and hold times (500ms) are decoupled from the main motor control.
This approach ensures predictable timing and avoids blocking delays.

## 7. Demo Video

A short demo video showing the sorting process and target completion:

▶️ [Color Sorter Conveyor - Demo Video](Link_YouTube_Cua_Ban_Vao_Day)

## 8. Build and Flash

Toolchain: arm-none-eabi-gcc.

Build system: CMake.

Programmer: ST-Link.

No vendor libraries or code generators are used.

## 9. Why Bare Metal?

This project intentionally avoids HAL and RTOS in order to:
- Understand STM32 peripherals at register level.
- Gain full control over timing and execution flow.
- Avoid hidden abstractions and unnecessary overhead.
- Build a solid foundation for embedded system debugging.

## 10. Performance & Timing Trade-offs

The system prioritizes deterministic behavior for object sorting.
Because the sorting mechanism relies on precise non-blocking time delays rather than independent position sensors at each sorting bin, there is a mechanical constraint on the feed rate.
Objects must be placed on the conveyor with a minimum spacing of 1.5 to 2 seconds. Feeding objects too closely causes the state machine to overwrite the pending color variable, leading to missed servo actuations.
This behavior illustrates a practical trade-off between hardware simplicity (fewer sensors) and overall system throughput in a bare-metal design.

## 11. Limitations and Future Improvements

This project is primarily intended as a learning exercise and therefore has several limitations:
- Target quotas are hardcoded or require manual UART input upon reset.
- No power-saving modes are used between processing ticks.
- The system lacks a physical automated feeder mechanism.

Planned improvements and learning directions include:
- Refactoring the code to better separate hardware abstraction and sorting logic.
- Adding non-volatile EEPROM storage to remember quotas after power loss.
- Transitioning from First Principles bare-metal programming to professional industry frameworks by integrating ROS 2 for advanced system coordination.

## 12. Author

Author: Huỳnh Đức Phát.
Author: Hoàng Anh.
Author: Vũ Thành Đạt.