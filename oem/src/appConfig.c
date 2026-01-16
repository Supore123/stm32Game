#include "main.h"
#include "appConfig.h"

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

void appSave()
{

}

void appLoad()
{

}
