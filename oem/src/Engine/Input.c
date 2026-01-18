#include "input.h"
#include <math.h>
#include "cmsis_os2.h" // Updated for CMSIS-RTOS v2

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
// Internal cache to remember the last known state
// (Static ensures it persists between function calls)
static PlayerInput_t cached_state = {0.0f, 0.0f, 0};

PlayerInput_t Input_ReadState(void)
{
    uint32_t raw_msg;
    osStatus_t status;
    int data_found = 0;

    // --- 1. DRAIN THE QUEUE (CMSIS-RTOS v2) ---
    // Loop until the queue is empty. We only care about the very last packet.
    // osMessageQueueGet replaces osMessageGet
    while (1)
    {
        // Arguments: (Queue ID, buffer pointer, priority pointer, timeout in ticks)
        status = osMessageQueueGet(xInputQueue, &raw_msg, NULL, 0);

        if (status == osOK) {
            data_found = 1; // Mark that we got new data
        } else {
            break; // Queue is empty, stop looping
        }
    }

    // --- 2. UPDATE CACHE (If we got new data) ---
    if (data_found)
    {
        // UNPACK: Mirroring the packing logic from InputTask
        // [Btn (1 bit) | Unused | Y (12 bits) | X (12 bits)]
        uint16_t raw_x = raw_msg & 0xFFF;
        uint16_t raw_y = (raw_msg >> 12) & 0xFFF;
        uint8_t  btn   = (raw_msg >> 24) & 1;

        // NORMALIZE: Convert 0..4095 -> -1.0 .. 1.0
        // (Assuming 2048 is center)
        cached_state.x = ((float)raw_x - 2048.0f) / 2048.0f;
        cached_state.y = ((float)raw_y - 2048.0f) / 2048.0f;

        // Deadzone (Optional: Snaps small drift to 0.0)
        if (fabsf(cached_state.x) < 0.1f) cached_state.x = 0.0f;
        if (fabsf(cached_state.y) < 0.1f) cached_state.y = 0.0f;

        cached_state.is_firing = btn;
    }

    // --- 3. RETURN STATE ---
    // If queue was empty, this returns the joystick position from the previous frame,
    // which effectively acts as a "Hold" (perfect for smooth movement).
    return cached_state;
}
