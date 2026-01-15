#include "main.h"
#include "appConfig.h"

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
