# BOOM! Raycasting Engine for STM32F401RE

This repository contains the source code for a 3D raycasting engine optimized for the STM32F401RE Nucleo-64. The game uses FreeRTOS to manage multiple tasks and DMA to handle joystick input without stalling the processor.

---

## Technical Specifications

| Component | Detail |
| :--- | :--- |
| STM32CubeIDE Version | 1.19.0 |
| Compiler | GNU Tools for STM32 (12.3.rel1) |
| Operating System | FreeRTOS (CMSIS-RTOS v2 API) |
| Display Driver | SSD1306 (I2C) |
| Input Method | ADC with Circular DMA |

---

## Game Design

The engine runs three main threads: a high-priority Input task to sample the joystick, a Game Logic task for physics and AI, and a Render task for the 3D projection.

There are two primary ways to play. Classic mode takes you through several levels with a 5-second transition screen between each stage once the enemies are cleared. Arcade mode is a survival-style game with continuous enemy spawning, where your current health and score are displayed on a specialized HUD. The high score is maintained in the internal Flash memory so it persists after a power cycle.

---

## Hardware Setup and Wiring

To run this, you will need a standard SSD1306 OLED (128x64) and an analog 2-axis joystick with a built-in push button.



| Component | Pin Name | Nucleo Pin | Function |
| :--- | :--- | :--- | :--- |
| SSD1306 OLED | VCC | 3.3V | Power |
| SSD1306 OLED | GND | GND | Ground |
| SSD1306 OLED | SCL | PB8 | I2C1 Clock |
| SSD1306 OLED | SDA | PB9 | I2C1 Data |
| Joystick | VCC | 3.3V | Power |
| Joystick | GND | GND | Ground |
| Joystick | VRX | PA0 | X-Axis (ADC1_CH0) |
| Joystick | VRY | PA1 | Y-Axis (ADC1_CH1) |
| Joystick | SW | PA10 | Fire Button (Digital Input) |

---

## How to Build and Run

1. Import the project folder into your STM32CubeIDE workspace.
2. Check the .ioc file to ensure that ADC1 is configured for "Scan Conversion Mode" and "Continuous Conversion Mode." The DMA should be set to "Circular" with "Half Word" increments to match the 12-bit ADC data.
3. Make sure I2C1 is set to Fast Mode (400kHz) to prevent the display from lagging.
4. Clean and build the project.
5. Connect your Nucleo board via USB and flash the code.

Once the "BOOM!" title screen appears, use the joystick to navigate the menu and the button to select your mode.

---

## Gameplay Controls

- Menu Navigation: Tilt the stick Up or Down to highlight Classic or Arcade.
- Movement: Forward and Backward on the stick moves the player in the 3D space.
- Turning: Left and Right on the stick rotates the camera view.
- Fire: Press the joystick button to shoot at enemies.
