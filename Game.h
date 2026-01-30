#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <string>

#define FPS 60
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SPRITE_SIZE 32
#define SPRITE_SCREEN_SIZE 32
#define GAME_TITLE "Star Escape"

struct GameInput
{
	int mouseX = 0;
	int mouseY = 0;
	bool mouseL = false;
	bool mouseR = false;
	int keyPressed = NULL;
	float xAxis = 0;
	float yAxis = 0;
};

class Game
{
public:
	Game();
	~Game();

	void startSDL();
	void welcomeScreen();
	void createGameObjects();
	void gameLoop();
	void handleEvents();
	void updateGameObjects(float deltaTicks);
	void render() const;
	void closeSDL();
	void updateGUI() const;
	void waitForKeyPress();
	void checkCollision();
	void checkAttacks();
	void resetAllObjects();
	void levelCompleteScreen();
	void loadMap(int levelNumber);
	void replayScreen();
	void manageGame();

	bool isRunning() { return gameRunning; }
	bool isReplaying() { return replay; }

	static SDL_Renderer* renderer;
	static SDL_Event playerInputEvent;
	SDL_Window* gameWindow = nullptr;

	bool gameRunning = false;
	bool replay = true;
	bool goalReached = false;
	int currentLevel = 0;

	int activeNPCs = 0;
	int activeItems = 0;

	Mix_Music* bgMusic = nullptr;
	Mix_Chunk* shootSound = nullptr;
	Mix_Chunk* explodeSound = nullptr;
	Mix_Chunk* pickupSound = nullptr;
	Mix_Chunk* buySound = nullptr;
};