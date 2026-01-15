#ifndef __I2C_H
#define __I2C_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

// External handle for the I2C mutex
extern SemaphoreHandle_t i2c_mutex;

// Function Prototypes
void I2C_Init(void);
HAL_StatusTypeDef I2C_Write_Locked(uint8_t devAddr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef I2C_WriteByte_Locked(uint8_t devAddr, uint8_t byte);

#endif /* __I2C_H */
