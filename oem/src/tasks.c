#include "appConfig.h"
#include "main.h"
#include "gameLogic.h"
#include "levels.h"
#include "input.h"

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
    {"Input",       InputTask,       osPriorityNormal,       2048,   &inputTaskHandle},
    {"GameLogic",   GameLogicTask,   osPriorityHigh,     2048,   &gameLogicTaskHandle},
    {"Render",      RenderTask,      osPriorityAboveNormal, 2048,   &renderTaskHandle},
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
    I2C_Init();
    OLED_Init();

    ClearScreen();
    OLED_Update();

    for(;;)
    {
        // 1. Clear the frame buffer for the new frame
        ClearScreen();

        // 2. Decide what to render based on Game State
        if (Game.state == STATE_MENU)
        {
            // Pass the current menu selection to be highlighted
            UI_DrawMenu(selected_option);
        }
        else if (Game.state == STATE_PLAYING)
        {
            // Draw the 3D Raycaster world
            Render_3D_View();

            // Draw a simple crosshair in the center
            DrawChar(62, 30, '+');
        }

        // 3. Push the buffer to the physical OLED
        OLED_Update();

        // 4. Maintain ~30 FPS (33ms delay)
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void GameLogicTask(void *params)
{
    // Initialize Game state and set starting position/level
    Game_Init();

    for(;;)
    {
        // 1. Get the latest input from the joystick and button
        PlayerInput_t input = Input_ReadState();

        if (Game.state == STATE_MENU)
        {
            // 2. Handle Menu Selection (Move frame between START and LOAD)
            if (input.y < -0.5f) {
                selected_option = MENU_LOAD;
            } else if (input.y > 0.5f) {
                selected_option = MENU_START;
            }

            // 3. Handle Button Press to Start Game
            if (input.is_firing)
            {
                Game.state = STATE_PLAYING;
            }
        }
        else if (Game.state == STATE_PLAYING)
        {
            // Handle 3D Movement and Collision
            Game_Update();

            // Handle Shooting logic (Muzzle flash/Hitscan)
            Game_HandleCombat();
        }

        // Run logic at 50Hz
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void InputTask(void *params)
{
    // Initialize the hardware drivers once
    ADC_Joystick_Init();

    for(;;)
    {
        // Poll Joystick X and Y (using the ADC channels defined in adc.h)
        // Values are typically 0-4095 for a 12-bit ADC
        uint16_t x_raw = ADC_Read_Locked(JOY_CHANNEL_X);
        uint16_t y_raw = ADC_Read_Locked(JOY_CHANNEL_Y);

        // Read Button State (Digital)
        uint8_t btn_raw = ADC_ReadButton();

        // Process the raw data into the high-level Input state
        // (This would typically update a shared global Input structure)

        // Yield to other tasks for 10ms (100Hz polling rate)
        osDelay(10);
    }
}
