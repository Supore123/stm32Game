#include "gameLogic.h"
#include "input.h"
#include "raycaster.h"
#include "levels.h"
#include <math.h>

GameState_t Game;
void Game_UpdateAI(void);
//
// Initialize the Game State on Boot
//
void Game_Init(void)
{
    Game.state = STATE_MENU;
    // LOAD FROM HARD SAVE
    Game.high_score = LoadHighScore();

    Game_LoadLevel(0);
}

//
// Loads level data and sets player spawn points
//
void Game_LoadLevel(int level_index)
{
	if (level_index >= TOTAL_LEVELS) level_index = 0;

	Game.current_level = AllLevels[level_index];

	// 1. Reset Player
	Game.player.x = Game.current_level->start_x;
	Game.player.y = Game.current_level->start_y;
	float angle = Game.current_level->start_angle;
	Game.player.dir_x = cosf(angle);
	Game.player.dir_y = sinf(angle);
	Game.player.plane_x = -0.66f * sinf(angle);
	Game.player.plane_y = 0.66f * cosf(angle);

	// 2. Load Enemies
	// Clear old enemies first
	for(int i=0; i<5; i++) Game.enemies[i].active = 0;

	// Copy from Level Data to Game State
	for(int i=0; i < Game.current_level->enemy_count; i++) {
		Game.enemies[i].x = Game.current_level->enemies[i].x;
		Game.enemies[i].y = Game.current_level->enemies[i].y;
		Game.enemies[i].active = 1; // It is alive!
	}

	Game.player.health = 100; // Reset HP

	for(int i=0; i < Game.current_level->enemy_count; i++) {
		Game.enemies[i].x = Game.current_level->enemies[i].x;
		Game.enemies[i].y = Game.current_level->enemies[i].y;
		Game.enemies[i].active = 1;
		Game.enemies[i].health = 1; // Takes 3 shots to kill
	}
}

//
// Main Game Logic Loop (Called by GameLogicTask)
//
void Game_Update(void)
{
	if (Game.state != STATE_PLAYING) return;

	PlayerInput_t input = Input_ReadState();

	// 1. Handle Rotation (Joystick X)
	if (fabsf(input.x) > 0.1f) {
		float rotSpeed = input.x * 0.06f;
		float oldDirX = Game.player.dir_x;
		Game.player.dir_x = Game.player.dir_x * cosf(-rotSpeed) - Game.player.dir_y * sinf(-rotSpeed);
		Game.player.dir_y = oldDirX * sinf(-rotSpeed) + Game.player.dir_y * cosf(-rotSpeed);

		float oldPlaneX = Game.player.plane_x;
		Game.player.plane_x = Game.player.plane_x * cosf(-rotSpeed) - Game.player.plane_y * sinf(-rotSpeed);
		Game.player.plane_y = oldPlaneX * sinf(-rotSpeed) + Game.player.plane_y * cosf(-rotSpeed);
	}

	// 2. Handle Movement (Joystick Y) WITH COLLISION
	if (fabsf(input.y) > 0.1f) {
		float moveSpeed = input.y * 0.10f; // Movement Speed

		// Calculate where we WANT to be
		float nextX = Game.player.x + Game.player.dir_x * moveSpeed;
		float nextY = Game.player.y + Game.player.dir_y * moveSpeed;

		// --- X-AXIS COLLISION CHECK ---
		int gridX = (int)nextX;
		int currY = (int)Game.player.y;

		if (gridX >= 0 && gridX < 16)
		{
			uint8_t tile = Game.current_level->map[gridX][currY];

			if (tile == 0) {
				Game.player.x = nextX;
			} else if (tile == 9) {
				Game.state = STATE_WIN; // Fixed: Go to WIN, not MENU
			}
		}

		// --- Y-AXIS COLLISION CHECK ---
		int currX = (int)Game.player.x;
		int gridY = (int)nextY;

		if (gridY >= 0 && gridY < 16)
		{
			uint8_t tile = Game.current_level->map[currX][gridY];

			if (tile == 0) {
				Game.player.y = nextY;
			} else if (tile == 9) {
				Game.state = STATE_WIN; // Fixed: Go to WIN, not MENU
			}
		}
	}

    // --- DELETED: The "Reset HP / Reset Enemies" loop was here ---
    // That code belongs ONLY in Game_LoadLevel, not Game_Update!

    // --- DELETED: Duplicate AI Code ---
    // You already have Game_UpdateAI() defined below.
    // We should not run AI logic twice.
}

// Helper to check angles safely
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
		// 1. Draw Muzzle Flash (Visual only)
		for(int i = 0; i < 5; i++) {
			DrawVLine(64-2+i, 32-2, 32+2, 2);
		}

		// 2. CHECK ENEMIES (No Raycasting!)
		for(int i=0; i<5; i++) {
			if (!Game.enemies[i].active) continue;

			float dx = Game.enemies[i].x - Game.player.x;
			float dy = Game.enemies[i].y - Game.player.y;
			float dist = sqrtf(dx*dx + dy*dy);

			float angleToEnemy = atan2f(dy, dx);
			float playerAngle = atan2f(Game.player.dir_y, Game.player.dir_x);

			// Check if we are facing the enemy (within 14 degrees) AND they are close
			if (IsFacingEnemy(playerAngle, angleToEnemy, 0.25f) && dist < 8.0f)
			{
				// HIT!
				Game.enemies[i].health--;

				// Visual Hitmarker
				DrawChar(64, 25, 'x');

				if (Game.enemies[i].health <= 0) {
					Game.enemies[i].active = 0; // Enemy Dies
					Game.high_score += 100;
				}
				return; // Stop after hitting one enemy
			}
		}
	}
}

void Game_UpdateAI(void)
{
	int enemies_remaining = 0;

	for(int i = 0; i < 5; i++)
	{
		if(!Game.enemies[i].active) continue;
		enemies_remaining++;

		// --- 1. CHASE PLAYER ---
		float dx = Game.player.x - Game.enemies[i].x;
		float dy = Game.player.y - Game.enemies[i].y;
		float dist = sqrtf(dx*dx + dy*dy);

		// --- FIX: Prevent Divide by Zero ---
		if (dist < 0.1f) dist = 0.1f;

		if (dist > 0.5f) {
			// Move towards player
			float speed = 0.15f; // AI Speed (at 10Hz tick)
			float moveX = (dx / dist) * speed;
			float moveY = (dy / dist) * speed;

			// Basic Collision
			if(Game.current_level->map[(int)(Game.enemies[i].x + moveX)][(int)(Game.enemies[i].y)] == 0)
				Game.enemies[i].x += moveX;
			if(Game.current_level->map[(int)(Game.enemies[i].x)][(int)(Game.enemies[i].y + moveY)] == 0)
				Game.enemies[i].y += moveY;
		}

		// --- 2. ATTACK PLAYER ---
		// If touching player, deal damage
		if (dist < 0.6f) {
			Game.player.health -= 5; // Ouch!

			// Push enemy back slightly to prevent instant death (bounce)
			Game.enemies[i].x -= (dx / dist) * 0.5f;
			Game.enemies[i].y -= (dy / dist) * 0.5f;
		}
	}

	// --- 3. CHECK WIN/LOSS CONDITIONS ---
	if (Game.player.health <= 0) {
		Game.state = STATE_GAMEOVER;
	}
	else if (enemies_remaining == 0) {
		// All enemies dead?
		// Optional: Check if we are also at the exit?
		// For now, let's say "Kill all enemies = Level Clear"
		Game.state = STATE_WIN;
	}
}
