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

// Global System that determines state of app at drive
typedef struct appConfig
{
	uint8_t gameVersion;	// Used for determining version
	uint8_t appVersion;
}appConfig_t;

enum
{
	APP_ERROR_INIT,
	APP_ERROR_UKNOWN,
}appStatus;

#endif /* HEADER_APPCONFIG_H_ */
