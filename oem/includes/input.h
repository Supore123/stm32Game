#ifndef __INPUT_H
#define __INPUT_H

#include "stm32f4xx_hal.h"
#include "adc.h" // Includes the low-level ADC driver

//
// Data Structure for Game Input
// This cleanly packages everything the Doom engine needs
//
typedef struct
{
    float x;            // Range: -1.0 (Left) to 1.0 (Right)
    float y;            // Range: -1.0 (Back) to 1.0 (Forward)
    uint8_t is_firing;  // 1 = Pressed, 0 = Released
} PlayerInput_t;

// Function Prototypes
void Input_Init(void);
PlayerInput_t Input_ReadState(void);

#endif
