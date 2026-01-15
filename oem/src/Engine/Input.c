#include "input.h"
#include <math.h>

// Threshold to ignore small joystick drift (Stick Drift)
#define JOY_DEADZONE  0.15f

//
// Initialize all Input Hardware
//
void Input_Init(void)
{
    // Initialize the Analog hardware (Axes)
    ADC_Joystick_Init();

    // The Button is initialized inside ADC_Joystick_Init as a GPIO
    // purely for convenience of grouping the wires, which is fine.
}

//
// Reads and Processes Player Input
//
PlayerInput_t Input_ReadState(void)
{
    PlayerInput_t input;

    // 1. Read Raw Hardware Values
    // Note: We use ADC for axes and GPIO for the button
    uint16_t raw_x = ADC_Read_Locked(JOY_CHANNEL_X);
    uint16_t raw_y = ADC_Read_Locked(JOY_CHANNEL_Y);
    uint8_t btn_state = ADC_ReadButton(); // This calls HAL_GPIO_ReadPin internally

    // 2. Convert Raw ADC (0-4095) to Normalized Float (-1.0 to 1.0)
    // 2048 is the theoretical center.
    // We divide by 2048 to get a range of roughly 0 to 2, then subtract 1.
    input.x = ((float)raw_x / 2048.0f) - 1.0f;
    input.y = ((float)raw_y / 2048.0f) - 1.0f;

    // 3. Apply Deadzone (Fixes jitter when stick is centered)
    if (fabsf(input.x) < JOY_DEADZONE) input.x = 0.0f;
    if (fabsf(input.y) < JOY_DEADZONE) input.y = 0.0f;

    // 4. Process Button (Active Low: 0 = Pressed)
    // We invert it so 1 = Firing, which is easier for game logic
    input.is_firing = (btn_state == GPIO_PIN_RESET) ? 1 : 0;

    return input;
}
