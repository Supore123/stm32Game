#ifndef __GAMELOGIC_H
#define __GAMELOGIC_H

#include "levels.h"
#include <stdint.h>

// Define States including Win/Loss
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
} SceneState_t;

typedef enum {
    MENU_START = 0,
    MENU_LOAD  = 1
} MenuOption_t;

// --- NEW: Enemy Structure with Health ---
typedef struct {
    float x, y;
    int active;     // 1 = Alive, 0 = Dead
    int health;     // HP (e.g., 3 hits to kill)
    float distance; // For sorting
} ActiveEnemy_t;

typedef struct {
    SceneState_t state;
    const Level_t* current_level;
    int current_level_idx; // Track which level we are on

    struct {
        float x, y;
        float dir_x, dir_y;
        float plane_x, plane_y;
        int health;  // Player HP
    } player;

    uint32_t high_score;

    // --- NEW: List of Active Enemies ---
    ActiveEnemy_t enemies[5];
} GameState_t;

extern GameState_t Game;

void Game_Init(void);
void Game_LoadLevel(int level_index);
void Game_Update(void);
void Game_HandleCombat(void);
void Game_UpdateAI(void);

#endif
