/* oem/includes/gameLogic.h */
#ifndef __GAMELOGIC_H
#define __GAMELOGIC_H

#include "levels.h"
#include <stdint.h>

// --- Game Modes ---
typedef enum {
    MODE_CLASSIC, // Levels 1 -> 2 -> Win
    MODE_ARCADE   // Infinite Spawning Enemies
} GameMode_t;

// --- State Machine ---
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_LEVEL_TRANSITION, //5-second pause between levels
    STATE_GAMEOVER,
    STATE_WIN,      // (Unused internally now, transitions handle it)
    STATE_VICTORY   // Beat ALL levels (Classic)
} SceneState_t;

// --- Menu Options ---
typedef enum {
    MENU_CLASSIC = 0,
    MENU_ARCADE  = 1
} MenuOption_t;

// --- Entity Structure ---
typedef struct {
    float x, y;
    int active;     // 1 = Alive, 0 = Dead
    int health;     // HP
    float distance; // For sorting
} ActiveEnemy_t;

// --- Main Model ---
typedef struct {
    SceneState_t state;
    GameMode_t mode;

    const Level_t* current_level;
    int current_level_idx;

    struct {
        float x, y;
        float dir_x, dir_y;
        float plane_x, plane_y;
        int health;
    } player;

    uint32_t current_score;
    uint32_t high_score;

    // Timer for transitions (ticks at 50Hz)
    int transition_timer;

    ActiveEnemy_t enemies[5];
} GameState_t;

extern GameState_t Game;

// --- Function Prototypes ---
void Game_Init(GameMode_t mode);
void Game_LoadLevel(int level_index);
void Game_Update(void);
void Game_HandleCombat(void);
void Game_UpdateAI(void);
void Game_HandleTransition(void);

#endif
