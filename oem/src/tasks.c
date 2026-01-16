#include "appConfig.h"
#include "main.h"
#include "gameLogic.h"
#include "levels.h"
#include "input.h"
#include "display.h"
#include "i2c.h"

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
		// Name,         Function,           Priority,          Stack,  Handle Pointer
		{"Input",       InputTask,       osPriorityNormal,      2048,   &inputTaskHandle},
		{"GameLogic",   GameLogicTask,   osPriorityHigh,    	2048,   &gameLogicTaskHandle},
		{"Render",      RenderTask,      osPriorityAboveNormal, 2048,   &renderTaskHandle},
};

#define TASK_COUNT (sizeof(TaskRegistry) / sizeof(taskConfig_t))

appStatus_t createTasks(void)
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
		ClearScreen();

		if (Game.state == STATE_MENU) {
			UI_DrawMenu(selected_option);
		}
		else if (Game.state == STATE_PLAYING) {
			Render_3D_View();
			Render_Enemies();

			// --- NEW: COMPASS ARROW ---
			// 1. Find the Exit Position (ID 9)
			float exitX = 0, exitY = 0;
			// Scan map to find exit (Fast enough for 16x16 map)
			for(int x=0; x<16; x++) {
				for(int y=0; y<16; y++) {
					if(Game.current_level->map[x][y] == 9) {
						exitX = x + 0.5f; // Center of the tile
						exitY = y + 0.5f;
					}
				}
			}

			// 2. Calculate Angle to Exit
			float dx = exitX - Game.player.x;
			float dy = exitY - Game.player.y;

			// World Angle to target
			float targetAngle = atan2f(dy, dx);
			// Player's Facing Angle
			float playerAngle = atan2f(Game.player.dir_y, Game.player.dir_x);

			// Relative Angle (Difference)
			float angleDiff = targetAngle - playerAngle;

			// 3. Draw Rotating Arrow
			// Center of compass (Top Middle of screen)
			int cx = 64;
			int cy = 10;
			float arrowLen = 8.0f;

			// Screen Rotation: Subtract 90 degrees (1.57 rad) so 0 diff = UP
			float screenAngle = angleDiff - 1.5708f;

			// Tip of the arrow
			int tipX = cx + (int)(cosf(screenAngle) * arrowLen);
			int tipY = cy + (int)(sinf(screenAngle) * arrowLen);

			// Draw Shaft
			DrawLine(cx, cy, tipX, tipY);

			// Draw Arrowhead Wings ( +/- 140 degrees from tip)
			float wingAngle1 = screenAngle + 2.4f;
			float wingAngle2 = screenAngle - 2.4f;
			float wingLen = 5.0f;

			DrawLine(tipX, tipY, tipX + (int)(cosf(wingAngle1) * wingLen), tipY + (int)(sinf(wingAngle1) * wingLen));
			DrawLine(tipX, tipY, tipX + (int)(cosf(wingAngle2) * wingLen), tipY + (int)(sinf(wingAngle2) * wingLen));
			// ---------------------------

			DrawChar(62, 30, '+'); // Crosshair

			// HUD
			char hpBuf[10];
			sprintf(hpBuf, "HP:%d", Game.player.health);
			DrawString(2, 0, hpBuf); // Requires <stdio.h>
		}
		else if (Game.state == STATE_GAMEOVER) {
			DrawBigTitle(20, 20); // Draws "DOOM" or similar
			DrawString(30, 45, "YOU DIED");
		}
		else if (Game.state == STATE_WIN)
		{
			char buf[32];
			sprintf(buf, "LEVEL %d", Game.current_level_idx + 1);
			DrawString(40, 20, buf);
			DrawString(30, 40, "COMPLETE!");
		}

		OLED_Update();
		vTaskDelay(pdMS_TO_TICKS(33));
	}
}

//
// The Main Game Loop (High Priority)
//
void GameLogicTask(void *params)
{
	// 1. Initialize the Game State (Level 1, Player HP, etc.)
	Game_Init();

	// Counter to slow down the AI
	uint8_t ai_tick_counter = 0;
	// Timer to control how long screens stay visible
	uint32_t state_timer = 0;
	// Flag to ensure we don't accidentally double-click
	int button_was_pressed = 0;
	int button_latch = 1;

	for(;;)
	{
		// Check inputs every frame
		PlayerInput_t input = Input_ReadState();

		if (input.is_firing)
		{
			button_latch = 1;
		} else
		{
			button_latch = 0;
		}

		if (Game.state == STATE_MENU || Game.state == STATE_GAMEOVER || Game.state == STATE_WIN)
		{
			// Menu Navigation Logic
			if (input.y < -0.5f) selected_option = MENU_LOAD;
			else if (input.y > 0.5f) selected_option = MENU_START;

			// Handle "Press Start" / "Restart"
			if (input.is_firing) {
				if (Game.state != STATE_MENU) Game_Init(); // Reset if restarting
				Game.state = STATE_PLAYING;

				// Debounce simple button press
				vTaskDelay(pdMS_TO_TICKS(500));
			}
		}
		else if (Game.state == STATE_PLAYING)
		{
			// --- FAST LOGIC (50Hz) ---
			// Player moves smoothly every single frame
			Game_Update();      // Player movement & collision
			Game_HandleCombat(); // Shooting & Hitscan

			// --- SLOW LOGIC (10Hz) ---
			// Only run AI once every 5 frames (50Hz / 5 = 10Hz)
			ai_tick_counter++;
			if (ai_tick_counter >= 5)
			{
				Game_UpdateAI(); // <--- CALLED FROM HERE
				ai_tick_counter = 0;
			}
		}
		else if (Game.state == STATE_WIN)
		{
			state_timer += 20;

			// Wait 2 seconds before allowing input
			if (state_timer > 1500)
			{
				if (input.is_firing) {
					button_was_pressed = 1;
				}
				else if (button_was_pressed) {
					button_was_pressed = 0;
				}

				if (input.is_firing && !button_was_pressed)
				{
					int nextLevel = Game.current_level_idx + 1;

					// --- CHECK IF GAME IS OVER ---
					if (nextLevel >= TOTAL_LEVELS) {
						// WE BEAT THE GAME!

						// Check High Score
						if (Game.high_score < 9999) { // Example logic
							// You might want to track a 'current_run_score' variable
							// For now, let's assume we boost high score by 1000 on win
							Game.high_score += 1000;
							SaveHighScore(Game.high_score); // <--- HARD SAVE HERE
						}

						Game.state = STATE_VICTORY; // Go to final screen
					}
					else {
						// Load Next Level Normally
						Game_LoadLevel(nextLevel);
						Game.state = STATE_PLAYING;
					}
				}
			}
		}
		// --- NEW: FINAL VICTORY SCREEN STATE ---
		else if (Game.state == STATE_VICTORY)
		{
			// Display Victory Screen

			// Just wait for reset
			if (input.is_firing) {
				Game_Init(); // Back to Menu
			}
		}
		else if (Game.state == STATE_GAMEOVER)
		{
			// Same logic for Game Over - Force a delay
			state_timer += 20;

			if (state_timer > 1000) // 1 second delay
			{
				// Allow restart only if button is fresh press
				if (input.is_firing && !button_was_pressed) {
					Game_Init(); // Reset to Menu
				}
			}
		}

		// Track button state for the NEXT frame
		// This remembers if you were holding the button down
		button_was_pressed = input.is_firing;
		// Run this loop at ~50Hz (20ms delay)
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
