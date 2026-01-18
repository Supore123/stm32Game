/* oem/src/UI/manager.c */
#include "gameLogic.h"
#include "input.h"
#include "raycaster.h"
#include "levels.h"
#include "display.h"
#include <math.h>
#include <stdlib.h> // For rand()

GameState_t Game;

static int IsValidLevel(int idx) {
    return (idx >= 0 && idx < TOTAL_LEVELS);
}

void Game_Init(GameMode_t mode)
{
    Game.state = STATE_MENU;
    Game.mode = mode;

    Game.high_score = LoadHighScore();
    Game.current_score = 0;

    Game_LoadLevel(0);
}

void Game_LoadLevel(int level_index)
{
    if (!IsValidLevel(level_index)) {
        Game.state = STATE_VICTORY;
        return;
    }

    Game.current_level_idx = level_index;
    Game.current_level = AllLevels[level_index];

    // Reset Player
    Game.player.x = Game.current_level->start_x;
    Game.player.y = Game.current_level->start_y;
    float angle = Game.current_level->start_angle;

    Game.player.dir_x = cosf(angle);
    Game.player.dir_y = sinf(angle);
    Game.player.plane_x = -0.66f * sinf(angle);
    Game.player.plane_y =  0.66f * cosf(angle);

    Game.player.health = 100;

    // Reset Enemies
    for(int i=0; i<5; i++) Game.enemies[i].active = 0;

    for(int i=0; i < Game.current_level->enemy_count; i++) {
        Game.enemies[i].x = Game.current_level->enemies[i].x;
        Game.enemies[i].y = Game.current_level->enemies[i].y;
        Game.enemies[i].active = 1;
        Game.enemies[i].health = 3;
    }
}

void Game_Update(void)
{
    if (Game.state != STATE_PLAYING) return;
    PlayerInput_t input = Input_ReadState();

    // Rotation
    if (fabsf(input.x) > 0.15f) {
        float rotSpeed = input.x * 0.05f;
        float s = sinf(-rotSpeed);
        float c = cosf(-rotSpeed);
        float oldDirX = Game.player.dir_x;
        Game.player.dir_x = Game.player.dir_x * c - Game.player.dir_y * s;
        Game.player.dir_y = oldDirX * s + Game.player.dir_y * c;
        float oldPlaneX = Game.player.plane_x;
        Game.player.plane_x = Game.player.plane_x * c - Game.player.plane_y * s;
        Game.player.plane_y = oldPlaneX * s + Game.player.plane_y * c;
    }

    // Movement
    if (fabsf(input.y) > 0.15f) {
        float moveSpeed = input.y * 0.08f;
        float nextX = Game.player.x + Game.player.dir_x * moveSpeed;
        float nextY = Game.player.y + Game.player.dir_y * moveSpeed;

        if(Game.current_level->map[(int)nextX][(int)Game.player.y] == 0) Game.player.x = nextX;
        if(Game.current_level->map[(int)Game.player.x][(int)nextY] == 0) Game.player.y = nextY;
    }
}

static int IsFacingEnemy(float playerAngle, float angleToEnemy, float threshold) {
    float diff = fabsf(playerAngle - angleToEnemy);
    if (diff > 3.14159f) diff = 6.28318f - diff;
    return (diff < threshold);
}

void Game_HandleCombat(void)
{
    PlayerInput_t input = Input_ReadState();

    if (input.is_firing)
    {
        // Visual Recoil/Flash
        DrawVLine(62, 28, 36, 1);
        DrawVLine(66, 28, 36, 1);

        float maxRangeSq = 64.0f;

        for(int i=0; i<5; i++) {
            if (!Game.enemies[i].active) continue;

            float dx = Game.enemies[i].x - Game.player.x;
            float dy = Game.enemies[i].y - Game.player.y;
            float distSq = (dx*dx + dy*dy);

            if (distSq < maxRangeSq)
            {
                float angleToEnemy = atan2f(dy, dx);
                float playerAngle = atan2f(Game.player.dir_y, Game.player.dir_x);

                if (IsFacingEnemy(playerAngle, angleToEnemy, 0.35f))
                {
                    Game.enemies[i].health--;

                    if (Game.enemies[i].health <= 0) {
                        Game.enemies[i].active = 0;

                        Game.current_score += 100;
                        if (Game.current_score > Game.high_score) {
                            Game.high_score = Game.current_score;
                            SaveHighScore(Game.high_score);
                        }
                    }
                    return;
                }
            }
        }
    }
}

void Game_UpdateAI(void)
{
    int active_enemies = 0;

    // AI Logic
    for(int i = 0; i < 5; i++)
    {
        if(!Game.enemies[i].active) continue;
        active_enemies++;

        float dx = Game.player.x - Game.enemies[i].x;
        float dy = Game.player.y - Game.enemies[i].y;
        float distSq = dx*dx + dy*dy;

        if (distSq > 0.25f && distSq < 100.0f) {
            float dist = sqrtf(distSq);
            Game.enemies[i].x += (dx / dist) * 0.04f;
            Game.enemies[i].y += (dy / dist) * 0.04f;
        }

        if (distSq < 0.6f) {
            Game.player.health -= 2;
        }
    }

    if (Game.player.health <= 0) {
        Game.state = STATE_GAMEOVER;
        return;
    }

    // Progression
    if (Game.mode == MODE_CLASSIC)
    {
        if (active_enemies == 0) {
            //  Trigger Transition instead of immediate load
            Game.state = STATE_LEVEL_TRANSITION;
            Game.transition_timer = 250; // 50Hz * 5 seconds = 250 ticks
        }
    }
    else if (Game.mode == MODE_ARCADE)
    {
        if (active_enemies < 3) {
            for(int i=0; i<5; i++) {
                if (!Game.enemies[i].active) {
                    Game.enemies[i].x = 2.0f + (rand() % 10);
                    Game.enemies[i].y = 2.0f + (rand() % 10);

                    if(Game.current_level->map[(int)Game.enemies[i].x][(int)Game.enemies[i].y] == 0) {
                        Game.enemies[i].health = 3;
                        Game.enemies[i].active = 1;
                        break;
                    }
                }
            }
        }
    }
}

//  Transition Handler
void Game_HandleTransition(void)
{
    if (Game.transition_timer > 0) {
        Game.transition_timer--;
    } else {
        // Timer done, load next level
        Game.current_level_idx++;

        if (IsValidLevel(Game.current_level_idx)) {
            Game_LoadLevel(Game.current_level_idx);
            Game.state = STATE_PLAYING;
        } else {
            Game.state = STATE_VICTORY;
        }
    }
}
