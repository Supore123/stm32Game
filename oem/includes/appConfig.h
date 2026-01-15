/*
 * appConfig.h
 *
 *  Created on: Nov 14, 2025
 *      Author: jonathan
 */

#ifndef HEADER_APPCONFIG_H_
#define HEADER_APPCONFIG_H_

#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "cmsis_os.h"

// Global Configuration State
typedef struct appConfig
{
	uint8_t gameVersion;	// Used for determining version
	uint8_t appVersion;
}appConfig_t;

// TaskConfig_t: Handles the task at hand being utilised
typedef struct
{
	const char *name;          // Task name for debugging
	osThreadFunc_t func;       // Pointer to the task function
	osPriority_t priority;     // osPriorityNormal, osPriorityHigh, etc.
	uint32_t stack_size;       // Stack size in BYTES
	osThreadId_t *handle;      // Pointer to the global handle variable
}taskConfig_t;


typedef enum
{
	APP_STATUS_OK = 0,
	APP_STATUS_TASK_ERROR = 1,
	APP_ERROR_INIT,
	APP_ERROR_UKNOWN = 255,
}appStatus_t;

// Function Definitions
appStatus_t createTasks();

#endif /* HEADER_APPCONFIG_H_ */
