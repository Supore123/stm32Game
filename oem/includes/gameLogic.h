#ifndef __GAMELOGIC_H
#define __GAMELOGIC_H

#include "levels.h"
#include <stdint.h>

// Define the missing Menu options
typedef enum {
    MENU_START = 0,
    MENU_LOAD  = 1
} MenuOption_t;

typedef enum {
    STATE_MENU,
    STATE_PLAYING
} SceneState_t;

typedef struct {
    SceneState_t state;
    const Level_t* current_level;
    struct {
        float x, y;
        float dir_x, dir_y;
        float plane_x, plane_y;
    } player;
    uint32_t high_score;
} GameState_t;

extern GameState_t Game;

void Game_Init(void);
void Game_LoadLevel(int level_index);
void Game_Update(void);

#endif
