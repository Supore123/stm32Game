#include "i2c.h"

// External handle declaration for STM32 HAL
extern I2C_HandleTypeDef hi2c1;

// FreeRTOS Mutex for I2C Bus arbitration
SemaphoreHandle_t i2c_mutex = NULL;

//
// Initialize the I2C1 peripheral and the FreeRTOS Mutex
//
void I2C_Init(void)
{
    // 1. Create the Mutex for thread safety
    i2c_mutex = xSemaphoreCreateMutex();

    // 2. Enable Clocks
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // 3. Configure GPIO (PB8=SCL, PB9=SDA)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 4. Configure I2C1 Peripheral
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;               // Increased to 400kHz (Fast Mode) for Doom FPS
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);
}

//
// Thread-safe transmission to a device
//
HAL_StatusTypeDef I2C_Write_Locked(uint8_t devAddr, uint8_t *pData, uint16_t size) {
    HAL_StatusTypeDef status = HAL_ERROR;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        status = HAL_I2C_Master_Transmit(&hi2c1, devAddr, pData, size, 25);

        // If the bus is stuck, try a software reset of the peripheral
        if (status != HAL_OK) {
            __HAL_I2C_DISABLE(&hi2c1);
            HAL_Delay(1);
            __HAL_I2C_ENABLE(&hi2c1);
        }
        xSemaphoreGive(i2c_mutex);
    }
    return status;
}
//
// Thread-safe single byte transmission
//
HAL_StatusTypeDef I2C_WriteByte_Locked(uint8_t devAddr, uint8_t byte)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        status = HAL_I2C_Master_Transmit(&hi2c1, devAddr, &byte, 1, HAL_MAX_DELAY);
        xSemaphoreGive(i2c_mutex);
    }

    return status;
}
