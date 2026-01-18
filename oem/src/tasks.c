/* oem/src/tasks.c */
#include "appConfig.h"
#include "main.h"
#include "gameLogic.h"
#include "levels.h"
#include "input.h"
#include "display.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

// --- Config ---
osThreadId_t inputTaskHandle;
osThreadId_t gameLogicTaskHandle;
osThreadId_t renderTaskHandle;

#define InputTaskStackSize       512
#define GameLogicTaskStackSize   4096
#define RenderTaskStackSize      4096

// --- Sync ---
osMessageQueueId_t xInputQueue;
osMutexId_t gameMutex;

// --- Prototypes ---
void InputTask(void *params);
void GameLogicTask(void *params);
void RenderTask(void *params);

const taskConfig_t TaskRegistry[] = {
    {"Input",      InputTask,       osPriorityAboveNormal, InputTaskStackSize,       &inputTaskHandle},
    {"GameLogic",  GameLogicTask,   osPriorityNormal,      GameLogicTaskStackSize,   &gameLogicTaskHandle},
    {"Render",     RenderTask,      osPriorityBelowNormal, RenderTaskStackSize,      &renderTaskHandle},
};

#define TASK_COUNT (sizeof(TaskRegistry) / sizeof(taskConfig_t))

appStatus_t createTasks(void) {
    xInputQueue = osMessageQueueNew(16, sizeof(uint32_t), NULL);
    gameMutex = osMutexNew(NULL);
    if (!xInputQueue || !gameMutex) return APP_STATUS_TASK_ERROR;

    for (int i = 0; i < TASK_COUNT; i++) {
        osThreadAttr_t attr = { .name = TaskRegistry[i].name, .priority = TaskRegistry[i].priority, .stack_size = TaskRegistry[i].stack_size };
        *(TaskRegistry[i].handle) = osThreadNew(TaskRegistry[i].func, NULL, &attr);
        if (*(TaskRegistry[i].handle) == NULL) Error_Handler();
    }
    return APP_STATUS_OK;
}

static MenuOption_t selected_option = MENU_CLASSIC;

// ---------------------------------------------------------
// RENDER TASK
// ---------------------------------------------------------
void RenderTask(void *params)
{
    I2C_Init();
    OLED_Init();

    for(;;)
    {
        if (osMutexAcquire(gameMutex, osWaitForever) == osOK)
        {
            ClearScreen();

            if (Game.state == STATE_MENU) {
                UI_DrawMenu(selected_option);
            }
            else if (Game.state == STATE_PLAYING) {
                Render_3D_View();
                Render_Enemies();

                char hudBuf[24];
                if (Game.mode == MODE_ARCADE) {
                    // [UPDATED] Shows Health AND Score
                    snprintf(hudBuf, sizeof(hudBuf), "HP:%d SC:%lu", Game.player.health, Game.current_score);
                } else {
                    snprintf(hudBuf, sizeof(hudBuf), "L%d HP:%d", Game.current_level_idx+1, Game.player.health);
                }
                DrawString(0, 0, hudBuf);
                DrawChar(62, 30, '+');
            }
            //  Transition Screen
            else if (Game.state == STATE_LEVEL_TRANSITION) {
                char buf[30];
                snprintf(buf, sizeof(buf), "LEVEL %d COMPLETE", Game.current_level_idx + 1);
                DrawBigTitle(20, 20); // Draws Logo
                DrawString(30, 45, buf);
            }
            else if (Game.state == STATE_GAMEOVER) {
                DrawBigTitle(20, 20);
                DrawString(30, 45, "YOU DIED");

                char s[20];
                snprintf(s, sizeof(s), "SCORE: %lu", Game.current_score);
                DrawString(20, 55, s);
            }
            else if (Game.state == STATE_VICTORY) {
                DrawString(30, 10, "VICTORY!");
                DrawString(10, 30, "ALL LEVELS");
                DrawString(20, 40, "CLEARED");

                char s[20];
                snprintf(s, sizeof(s), "SCORE: %lu", Game.current_score);
                DrawString(10, 55, s);
            }

            osMutexRelease(gameMutex);
        }

        OLED_Update();
        osDelay(33);
    }
}

// ---------------------------------------------------------
// GAME LOGIC TASK
// ---------------------------------------------------------
void GameLogicTask(void *params)
{
    osMutexAcquire(gameMutex, osWaitForever);
    Game_Init(MODE_CLASSIC);
    Game.state = STATE_MENU;
    osMutexRelease(gameMutex);

    uint8_t ai_tick_counter = 0;
    int button_was_pressed = 0;

    for(;;)
    {
        PlayerInput_t input = Input_ReadState();

        if (osMutexAcquire(gameMutex, osWaitForever) == osOK)
        {
            if (Game.state == STATE_MENU) {
                if (input.y < -0.5f) selected_option = MENU_ARCADE;
                else if (input.y > 0.5f) selected_option = MENU_CLASSIC;

                if (input.is_firing && !button_was_pressed) {
                    if (selected_option == MENU_CLASSIC) Game_Init(MODE_CLASSIC);
                    else Game_Init(MODE_ARCADE);
                    Game.state = STATE_PLAYING;
                }
            }
            else if (Game.state == STATE_PLAYING) {
                Game_Update();
                Game_HandleCombat();

                if (++ai_tick_counter >= 5) {
                    Game_UpdateAI();
                    ai_tick_counter = 0;
                }
            }
            //  Handle Transition Logic
            else if (Game.state == STATE_LEVEL_TRANSITION) {
                Game_HandleTransition();
            }
            else if (Game.state == STATE_GAMEOVER || Game.state == STATE_VICTORY) {
                 if (input.is_firing && !button_was_pressed) {
                    Game.state = STATE_MENU;
                 }
            }
            osMutexRelease(gameMutex);
        }

        button_was_pressed = input.is_firing;
        osDelay(20);
    }
}

// ---------------------------------------------------------
// INPUT TASK (Unchanged)
// ---------------------------------------------------------
void InputTask(void *params)
{
    Input_Init();
    const uint32_t frequency = 20;
    uint32_t tick = osKernelGetTickCount();

    for(;;)
    {
        uint16_t raw_x = ADC_Read_Locked(JOY_CHANNEL_X);
        uint16_t raw_y = ADC_Read_Locked(JOY_CHANNEL_Y);
        uint8_t  btn   = ADC_ReadButton();

        uint32_t packet = 0;
        packet |= (raw_x & 0xFFF);
        packet |= ((raw_y & 0xFFF) << 12);
        packet |= ((uint32_t)(btn & 0x1) << 24);

        osMessageQueuePut(xInputQueue, &packet, 0, 0);

        tick += frequency;
        osDelayUntil(tick);
    }
}
