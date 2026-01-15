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
PlayerInput_t Input_ReadState(void) {
    PlayerInput_t state;

    // Read raw ADC (0-4095)
    uint16_t raw_x = ADC_Read_Locked(JOY_CHANNEL_X);
    uint16_t raw_y = ADC_Read_Locked(JOY_CHANNEL_Y);

    // Convert to -1.0 to 1.0 (Center is approx 2048)
    state.x = ((float)raw_x - 2048.0f) / 2048.0f;
    state.y = ((float)raw_y - 2048.0f) / 2048.0f;

    // Button Logic
    state.is_firing = ADC_ReadButton();

    return state;
}
