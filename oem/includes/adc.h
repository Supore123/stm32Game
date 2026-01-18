#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h" // CMSIS-RTOS v2 for best practices
#include "stm32f4xx_hal.h"
#include <stdint.h>

// --- Joystick Pin Definitions ---
#define JOY_PORT_BUTTON          GPIOA
#define JOY_PIN_BUTTON           GPIO_PIN_10

// --- Function Prototypes ---
void ADC_Joystick_Init(void);

// New "Zero-Cost" Accessors (Read directly from RAM)
uint32_t ADC_GetX(void);
uint32_t ADC_GetY(void);
uint8_t  ADC_ReadButton(void);

#endif /* __ADC_H */
