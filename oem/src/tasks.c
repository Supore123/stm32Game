#include "appConfig.h"
#include "main.h"
#include "gameLogic.h"
#include "levels.h"

// Thread Handles
osThreadId_t inputTaskHandle;
osThreadId_t gameLogicTaskHandle;
osThreadId_t renderTaskHandle;

void InputTask(void *params);
void GameLogicTask(void *params);
void RenderTask(void *params);

// The Registry for the tasks being used
/* Define the Task List */
const taskConfig_t TaskRegistry[] = {
    // Name,         Function,           Priority,           Stack,  Handle Pointer
//    {"Input",       InputTask,       osPriorityNormal,       2048,   &inputTaskHandle},
    {"GameLogic",   GameLogicTask,   osPriorityHigh,     1024,   &gameLogicTaskHandle},
    {"Render",      RenderTask,      osPriorityAboveNormal, 1024,   &renderTaskHandle},
};

#define TASK_COUNT (sizeof(TaskRegistry) / sizeof(taskConfig_t))

appStatus_t createTasks()
{
	appStatus_t sc = APP_STATUS_OK;

	for (int i = 0; i < TASK_COUNT; i++)
	{
	    osThreadAttr_t attributes = {0}; // Zero-initialize for safety

	    attributes.name = TaskRegistry[i].name;
	    attributes.priority = TaskRegistry[i].priority;
	    attributes.stack_size = TaskRegistry[i].stack_size;

	    // Create the thread and save the ID to your global handle
	    *(TaskRegistry[i].handle) = osThreadNew(TaskRegistry[i].func, NULL, &attributes);

	    // Basic safety check
	    if (*(TaskRegistry[i].handle) == NULL)
	    {
	        Error_Handler();
	    }
	}

	return sc;
}


#include "display.h"
//#include "i2c.h"

// Global state variables
static MenuOption_t selected_option = MENU_START;

//
// Logic Task: Manages state transitions and menu selection
//

extern GameState_t Game;

//
// Render Task: Draws the menu based on the current state
//
void RenderTask(void *params)
{
    // 1. Initialize Hardware INSIDE the task
    I2C_Init();   // Safe to call here or main, but here is safer for Mutex
    OLED_Init();  // MUST be here because of vTaskDelay

    // 2. Clear junk pixels once
    ClearScreen();
    OLED_Update();
//    UI_DrawMenu(1);
//    DrawColorPalette();

    for(;;)
    {
        // 3. Render the current state (Menu or Game)
        // Check your global state to decide what to draw
//        if (1) {
//        	UI_DrawMenu(0);
//        }
//        else if (Game.state == STATE_PLAYING) {
//            // Render_Raycaster(); // We will build this later
//        }

        // 4. Flush to screen
    	DrawColorPalette();
//        OLED_Update();

        // 5. Maintain Frame Rate (~30 FPS)
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void GameLogicTask(void *params)
{
	vTaskDelay(pdMS_TO_TICKS(500));
}
