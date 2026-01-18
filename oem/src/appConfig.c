#include "main.h"
#include "appConfig.h"
#include "stm32f4xx_hal.h" // Needed for Flash operations

// Define the address for saving the high score (Sector 7 start)
#define FLASH_STORAGE_ADDR 0x08060000


// Default App Parameters
appConfig_t AppConfig = {
		.gameVersion = 1,
		.appVersion = 0,
		.invert_y_axis = 0,   // Standard controls by default
		.sensitivity = 1.0f,  // Normal speed
		.difficulty = 0       // Normal difficulty
};

// Function to intiialise the appStatus
void appInit()
{
	appStatus_t sc = APP_STATUS_OK;
	// Check the app version and save file usage
	sc = createTasks();
	if(sc!= APP_STATUS_OK) {Error_Handler();}
	printf("hello");

	// Read the version number
}

//
// Reads the high score from Flash Memory
//
uint32_t LoadHighScore(void)
{
	// Simply cast the address to a pointer and dereference it
	uint32_t saved_score = *(__IO uint32_t*)FLASH_STORAGE_ADDR;

	// If flash is empty, it reads as 0xFFFFFFFF. Return 150 (default) instead.
	if (saved_score == 0xFFFFFFFF) {
		return 0;
	}
	return saved_score;
}

//
// HARD SAVE: Erases flash sector and writes new score
//
void SaveHighScore(uint32_t new_score)
{
	HAL_FLASH_Unlock();

	// 1. Erase Sector 7
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = FLASH_SECTOR_7;
	EraseInitStruct.NbSectors = 1;

	uint32_t SectorError = 0;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
	{
		// Handle Error
		HAL_FLASH_Lock();
		return;
	}

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORAGE_ADDR, new_score);

	HAL_FLASH_Lock();
}
