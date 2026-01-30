#include "Game.h"
#include "GameObject.h"
#include "PlayerCharacter.h"
#include "NPC.h"
#include "Projectile.h"
#include "Levels.h"
#include <iostream> 

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::playerInputEvent;
GameInput playerInput;

GameObject* backGround = nullptr;
PlayerCharacter* pc = nullptr;
GameObject* items[20] = {};
NPC* npcs[20] = {};
Projectile* bulletsPC[10] = {};
Projectile* bulletsNPC[20] = {};
GameObject* terrainBlocks[200] = {};
GameObject* goal = nullptr;
Levels* levelMaps = nullptr;

GameObject* merchant = nullptr;
bool isShopOpen = false;

TTF_Font* font = nullptr;
TTF_Font* font2 = nullptr;
SDL_Color textColour = { 255, 255, 200 };
SDL_Surface* textSurface = nullptr;
SDL_Texture* textTexture = nullptr;

Game::Game()
{
	startSDL();
	createGameObjects();
	gameLoop();
}

void Game::gameLoop()
{
	const double defaultFrameTime = 1000.0 / FPS;
	Uint64 frameStart;
	float deltaTicks = defaultFrameTime / 1000;

	while (isReplaying())
	{
		welcomeScreen();
		while (isRunning())
		{
			frameStart = SDL_GetTicks();
			handleEvents();
			updateGameObjects(deltaTicks);
			manageGame();
			render();

			deltaTicks = float(SDL_GetTicks() - frameStart);
			if (defaultFrameTime > deltaTicks)
			{
				SDL_Delay(int(defaultFrameTime - deltaTicks));
				deltaTicks = float(defaultFrameTime);
			}
		}
		replayScreen();
	}
}

void Game::manageGame()
{
	if (isShopOpen)
	{
		SDL_Rect speedBtnRect = { 480, 210, 100, 30 };
		SDL_Rect healthBtnRect = { 480, 270, 100, 30 };
		SDL_Point mousePoint = { playerInput.mouseX, playerInput.mouseY };

		if (playerInput.keyPressed == SDLK_1 || (playerInput.mouseL && SDL_PointInRect(&mousePoint, &speedBtnRect)))
		{
			if (pc->getMoney() >= 50)
			{
				pc->addMoney(-50);
				pc->setSpeed(pc->getSpeed() + 50);
				Mix_PlayChannel(-1, buySound, 0);
			}
			SDL_Delay(250);
		}

		if (playerInput.keyPressed == SDLK_2 || (playerInput.mouseL && SDL_PointInRect(&mousePoint, &healthBtnRect)))
		{
			if (pc->getMoney() >= 20)
			{
				pc->addMoney(-20);
				pc->setHP(100);
				Mix_PlayChannel(-1, buySound, 0);
			}
			SDL_Delay(250);
		}

		if (playerInput.keyPressed == SDLK_q)
		{
			isShopOpen = false;
			SDL_Delay(200);
		}
		return;
	}

	if (merchant->getAliveState())
	{
		SDL_Rect pcRect = { (int)pc->getX(), (int)pc->getY(), SPRITE_SIZE, SPRITE_SIZE };
		SDL_Rect merchRect = { (int)merchant->getX(), (int)merchant->getY(), SPRITE_SIZE, SPRITE_SIZE };

		if (SDL_HasIntersection(&pcRect, &merchRect))
		{
			if (playerInput.keyPressed == SDLK_e)
			{
				isShopOpen = true;
				SDL_Delay(200);
			}
		}
	}

	if (currentLevel == 0)
	{
		resetAllObjects();
		loadMap(1);
		currentLevel = 1;
	}

	if (goalReached)
	{
		goalReached = false;
		resetAllObjects();
		levelCompleteScreen();

		if (currentLevel == 1) { loadMap(2); currentLevel = 2; }
		else if (currentLevel == 2) { loadMap(3); currentLevel = 3; }
		else if (currentLevel == 3) { loadMap(4); currentLevel = 4; }
		else if (currentLevel == 4) { gameRunning = false; currentLevel = 99; }
		return;
	}

	checkAttacks();
	checkCollision();

	if (activeItems <= 0) goal->setAlive(true);

	if (pc->getHP() <= 0)
	{
		levelCompleteScreen();
		resetAllObjects();
		loadMap(currentLevel);
	}
}

void Game::checkCollision()
{
	SDL_Rect pcRect = { (int)pc->getX(), (int)pc->getY(), SPRITE_SIZE, SPRITE_SIZE };
	SDL_Rect objectRect = { -100,-100, SPRITE_SIZE, SPRITE_SIZE };
	SDL_Rect bulletRect = { 0,0,8,8 };

	objectRect.x = (int)goal->getX(); objectRect.y = (int)goal->getY();
	if (SDL_HasIntersection(&pcRect, &objectRect)) goalReached = true;

	for (GameObject* block : terrainBlocks)
	{
		if (!block->getAliveState()) continue;
		objectRect.x = (int)block->getX();
		objectRect.y = (int)block->getY();
		if (SDL_HasIntersection(&pcRect, &objectRect)) pc->stop();

		for (Projectile* bullet : bulletsNPC)
		{
			if (!bullet->getAliveState()) continue;
			bulletRect.x = bullet->getX();
			bulletRect.y = bullet->getY();
			if (SDL_HasIntersection(&bulletRect, &objectRect)) bullet->setAlive(false);
		}
	}

	for (Projectile* bullet : bulletsNPC)
	{
		if (!bullet->getAliveState()) continue;
		bulletRect.x = bullet->getX();
		bulletRect.y = bullet->getY();
		if (SDL_HasIntersection(&bulletRect, &pcRect))
		{
			bullet->setAlive(false);
			pc->changeHP(-bullet->getDamage());
		}
	}

	for (GameObject* item : items)
	{
		if (!item->getAliveState()) continue;
		objectRect.x = int(item->getX());
		objectRect.y = int(item->getY());
		if (SDL_HasIntersection(&pcRect, &objectRect))
		{
			item->setAlive(false);
			activeItems--;
			Mix_PlayChannel(-1, pickupSound, 0);
		}
	}

	for (GameObject* npc : npcs)
	{
		if (!npc->getAliveState()) continue;
		SDL_Rect npcRect = { (int)npc->getX(), (int)npc->getY(), 32, 32 };

		for (Projectile* bullet : bulletsPC)
		{
			if (!bullet->getAliveState()) continue;
			SDL_Rect bulletRect = { bullet->getX(), bullet->getY(), 8, 8 };
			if (SDL_HasIntersection(&bulletRect, &npcRect))
			{
				bullet->setAlive(false);
				npc->changeHP(-bullet->getDamage());
				if (npc->getHP() <= 0)
				{
					npc->setAlive(false);
					activeNPCs--;
					pc->addMoney(10);
					Mix_PlayChannel(-1, explodeSound, 0);
				}
			}
		}
	}
}

void Game::checkAttacks()
{
	if (playerInput.keyPressed == 32)
		for (Projectile* bullet : bulletsPC)
		{
			if (bullet->getAliveState()) continue;
			bullet->fire(int(pc->getX()), int(pc->getY()), pc->getAngle());
			Mix_PlayChannel(-1, shootSound, 0);
			break;
		}

	if (playerInput.mouseL)
		for (Projectile* bullet : bulletsPC)
		{
			if (bullet->getAliveState()) continue;
			bullet->fireAtTarget(int(pc->getX()), int(pc->getY()), int(playerInput.mouseX), int(playerInput.mouseY));
			Mix_PlayChannel(-1, shootSound, 0);
			break;
		}

	for (NPC* npc : npcs)
	{
		if (!npc->getAliveState()) continue;
		if (npc->getNextShotTime() > SDL_GetTicks()) continue;
		for (Projectile* bullet : bulletsNPC)
		{
			if (bullet->getAliveState() == false)
			{
				bullet->fireAtTarget((int)npc->getX(), (int)npc->getY(), (int)pc->getX(), pc->getY());
				npc->setNextShotTime(SDL_GetTicks() + 3000 + rand() % 7000);
				break;
			}
		}
	}
}

void Game::levelCompleteScreen()
{
	GameObject* background = new GameObject("assets/images/frame.png", 0, 0);
	background->setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	background->render();
	SDL_RenderPresent(renderer);

	std::string screenText;
	textColour = { 0, 0, 0 };
	SDL_Rect textRect = { 200, 250, 0, 0 };

	if (pc->getHP() <= 0) screenText = "GAME OVER \n \npress a key to try again";
	else screenText = "NICE\n \npress a key to go to the next level";

	textSurface = TTF_RenderText_Blended_Wrapped(font2, screenText.c_str(), textColour, 0);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_RenderPresent(renderer);

	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);
	delete background;

	SDL_Delay(500);
	waitForKeyPress();
}

void Game::resetAllObjects()
{
	pc->setHP(100);
	pc->setX(-100);
	pc->setVelX(0); pc->setVelY(0);
	pc->setAngle(0); pc->setSpeed(128);
	goal->setAlive(false);
	merchant->setAlive(false);

	for (NPC* npc : npcs) { npc->setAlive(false); npc->setHP(100); }
	for (GameObject* block : terrainBlocks) block->setAlive(false);
	for (Projectile* bullet : bulletsPC) bullet->setAlive(false);
	for (Projectile* bullet : bulletsNPC) bullet->setAlive(false);
	for (GameObject* item : items) item->setAlive(false);

	activeItems = 0;
	activeNPCs = 0;
}

void Game::createGameObjects()
{
	int xPos = -96; int yPos = -96;
	levelMaps = new Levels;
	backGround = new GameObject("assets/images/grid.png", 0, 0);
	backGround->setSize(800, 600);
	pc = new PlayerCharacter("assets/images/player.png", 32, 32);
	merchant = new GameObject("assets/images/shop.png", xPos, yPos);

	for (int i = 0; i < 20; i++) items[i] = new GameObject("assets/images/button.png", xPos, yPos);
	for (int i = 0; i < 10; i++) bulletsPC[i] = new Projectile("assets/images/pb.png", xPos, yPos, 0, 8);
	for (int i = 0; i < 20; i++) bulletsNPC[i] = new Projectile("assets/images/eb.png", xPos, yPos, 0, 8);
	for (int i = 0; i < 20; i++) npcs[i] = new NPC("assets/images/ai.png", xPos, yPos, 0);
	for (int i = 0; i < 200; i++) terrainBlocks[i] = new GameObject("assets/images/frame.png", xPos, yPos);
	goal = new GameObject("assets/images/door.png", xPos, yPos);
}

void Game::updateGameObjects(float deltaTicks)
{
	if (isShopOpen) return;
	float frameTime = deltaTicks / 1000;

	for (GameObject* item : items) { if (item->getAliveState()) item->update(); }
	for (GameObject* block : terrainBlocks) block->update();
	for (NPC* npc : npcs) if (npc->getAliveState()) npc->update(frameTime);
	for (Projectile* bullet : bulletsPC) if (bullet->getAliveState()) bullet->update(frameTime);
	for (Projectile* bullet : bulletsNPC) if (bullet->getAliveState()) bullet->update(frameTime);

	pc->update(playerInput.xAxis, playerInput.yAxis, frameTime);
	goal->update();
	if (merchant->getAliveState()) merchant->update();
}

void Game::render() const
{
	SDL_RenderClear(renderer);
	backGround->render();
	for (GameObject* block : terrainBlocks) if (block->getAliveState()) block->render();
	for (GameObject* item : items) if (item->getAliveState()) item->render();
	if (merchant->getAliveState()) merchant->render();
	for (NPC* npc : npcs) if (npc->getAliveState()) npc->render();
	for (Projectile* bullet : bulletsPC) if (bullet->getAliveState()) bullet->renderProjectile();
	for (Projectile* bullet : bulletsNPC) if (bullet->getAliveState()) bullet->renderProjectile();
	if (goal->getAliveState()) goal->render();
	pc->render();
	updateGUI();

	if (isShopOpen)
	{
		SDL_Point mousePoint = { playerInput.mouseX, playerInput.mouseY };
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
		SDL_Rect shadowRect = { 208, 158, 400, 300 };
		SDL_RenderFillRect(renderer, &shadowRect);
		SDL_SetRenderDrawColor(renderer, 30, 30, 60, 255);
		SDL_Rect shopRect = { 200, 150, 400, 300 };
		SDL_RenderFillRect(renderer, &shopRect);
		SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
		SDL_RenderDrawRect(renderer, &shopRect);
		SDL_Rect innerBorder = { 204, 154, 392, 292 };
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &innerBorder);
		SDL_RenderDrawLine(renderer, 220, 195, 580, 195);

		SDL_Rect speedBtnRect = { 480, 210, 100, 30 };
		SDL_Rect healthBtnRect = { 480, 270, 100, 30 };

		if (SDL_PointInRect(&mousePoint, &speedBtnRect)) SDL_SetRenderDrawColor(renderer, 50, 255, 50, 255);
		else SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
		SDL_RenderFillRect(renderer, &speedBtnRect);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &speedBtnRect);

		if (SDL_PointInRect(&mousePoint, &healthBtnRect)) SDL_SetRenderDrawColor(renderer, 50, 255, 50, 255);
		else SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
		SDL_RenderFillRect(renderer, &healthBtnRect);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &healthBtnRect);

		SDL_Color white = { 255, 255, 255 };
		SDL_Rect titleRect = { 330, 160, 0, 0 };
		SDL_Surface* s = TTF_RenderText_Blended(font, "TRADER", white);
		SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
		SDL_QueryTexture(t, NULL, NULL, &titleRect.w, &titleRect.h);
		SDL_RenderCopy(renderer, t, NULL, &titleRect);
		SDL_FreeSurface(s); SDL_DestroyTexture(t);

		SDL_Rect itemTextRect = { 230, 215, 0, 0 };
		s = TTF_RenderText_Blended(font, "Speed Up ($50)", white);
		t = SDL_CreateTextureFromSurface(renderer, s);
		SDL_QueryTexture(t, NULL, NULL, &itemTextRect.w, &itemTextRect.h);
		SDL_RenderCopy(renderer, t, NULL, &itemTextRect);
		SDL_FreeSurface(s); SDL_DestroyTexture(t);

		itemTextRect.y = 275;
		s = TTF_RenderText_Blended(font, "Full HP ($20)", white);
		t = SDL_CreateTextureFromSurface(renderer, s);
		SDL_QueryTexture(t, NULL, NULL, &itemTextRect.w, &itemTextRect.h);
		SDL_RenderCopy(renderer, t, NULL, &itemTextRect);
		SDL_FreeSurface(s); SDL_DestroyTexture(t);

		SDL_Color black = { 0, 0, 0 };
		SDL_Rect btnTextRect = { 505, 215, 0, 0 };
		s = TTF_RenderText_Blended(font2, "BUY", black);
		t = SDL_CreateTextureFromSurface(renderer, s);
		SDL_QueryTexture(t, NULL, NULL, &btnTextRect.w, &btnTextRect.h);
		SDL_RenderCopy(renderer, t, NULL, &btnTextRect);
		btnTextRect.y = 275;
		SDL_RenderCopy(renderer, t, NULL, &btnTextRect);
		SDL_FreeSurface(s); SDL_DestroyTexture(t);

		SDL_Rect qRect = { 330, 400, 0, 0 };
		s = TTF_RenderText_Blended(font2, "Press Q to Leave", white);
		t = SDL_CreateTextureFromSurface(renderer, s);
		SDL_QueryTexture(t, NULL, NULL, &qRect.w, &qRect.h);
		SDL_RenderCopy(renderer, t, NULL, &qRect);
		SDL_FreeSurface(s); SDL_DestroyTexture(t);
	}
	SDL_RenderPresent(renderer);
}

void Game::updateGUI() const
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect uiBar = { 0, 0, 800, 35 };
	SDL_RenderFillRect(renderer, &uiBar);

	std::string screenText;
	SDL_Rect textRect = { 10, 5, 0, 0 };
	SDL_Color green = { 0, 255, 0 };

	screenText = "Lvl: " + std::to_string(currentLevel);
	screenText += " Buttons: " + std::to_string(activeItems);
	screenText += " Aliens: " + std::to_string(activeNPCs);

	textSurface = TTF_RenderText_Blended_Wrapped(font2, screenText.c_str(), green, 0);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);

	textRect = { 480, 5, 0, 0 };
	screenText = "HP: " + std::to_string(int(pc->getHP()));
	screenText += " $: " + std::to_string(pc->getMoney());
	screenText += " Time: " + std::to_string(SDL_GetTicks64() / 1000);

	textSurface = TTF_RenderText_Blended_Wrapped(font2, screenText.c_str(), green, 0);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);
}

void Game::waitForKeyPress()
{
	SDL_Delay(1000);
	bool exitLoop = false;
	while (!exitLoop)
	{
		SDL_PollEvent(&playerInputEvent);
		if (playerInputEvent.type == SDL_MOUSEBUTTONDOWN || playerInputEvent.type == SDL_KEYDOWN) exitLoop = true;
	}
}

void Game::handleEvents()
{
	playerInput.keyPressed = NULL;
	playerInput.xAxis = 0;
	playerInput.yAxis = 0;
	playerInput.mouseL = false;
	SDL_PollEvent(&playerInputEvent);

	switch (playerInputEvent.type)
	{
	case SDL_QUIT: gameRunning = false; replay = false; break;
	case SDL_MOUSEMOTION:
		int mouseXpos, mouseYpos;
		SDL_GetMouseState(&mouseXpos, &mouseYpos);
		playerInput.mouseX = mouseXpos;
		playerInput.mouseY = mouseYpos;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (playerInputEvent.button.button == SDL_BUTTON_LEFT) playerInput.mouseL = true;
		if (playerInputEvent.button.button == SDL_BUTTON_RIGHT) playerInput.mouseR = true;
		break;
	case SDL_KEYDOWN: playerInput.keyPressed = playerInputEvent.key.keysym.sym; break;
	}

	const Uint8* keysPressed = SDL_GetKeyboardState(NULL);
	if (keysPressed[SDL_SCANCODE_UP] || keysPressed[SDL_SCANCODE_W]) playerInput.yAxis++;
	if (keysPressed[SDL_SCANCODE_LEFT] || keysPressed[SDL_SCANCODE_A]) playerInput.xAxis--;
	if (keysPressed[SDL_SCANCODE_DOWN] || keysPressed[SDL_SCANCODE_S]) playerInput.yAxis--;
	if (keysPressed[SDL_SCANCODE_RIGHT] || keysPressed[SDL_SCANCODE_D]) playerInput.xAxis++;
}

Game::~Game()
{
	closeSDL();
}

void Game::startSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return;
	gameWindow = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!gameWindow) return;

	if (TTF_Init() == 0)
	{
		font = TTF_OpenFont("assets/fonts/arial.ttf", 22);
		font2 = TTF_OpenFont("assets/fonts/bubble_pixel-7_dark.ttf", 18);
	}

	renderer = SDL_CreateRenderer(gameWindow, -1, 0);
	if (!renderer) return;
	gameRunning = true;

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) printf("Mixer Error: %s\n", Mix_GetError());
	bgMusic = Mix_LoadMUS("assets/audio/DDLoop1.wav");
	shootSound = Mix_LoadWAV("assets/audio/shoot.wav");
	explodeSound = Mix_LoadWAV("assets/audio/explosion.wav");
	pickupSound = Mix_LoadWAV("assets/audio/beep.wav");
	buySound = Mix_LoadWAV("assets/audio/jump.wav");
	if (bgMusic) Mix_PlayMusic(bgMusic, -1);
}

void Game::welcomeScreen()
{
	GameObject* splash = new GameObject("assets/images/screen.png", 0, 0);
	splash->setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	splash->render();
	SDL_RenderPresent(renderer);
	SDL_Delay(500);
	delete splash;

	GameObject* bg = new GameObject("assets/images/gs.png", 0, 0);
	bg->setSize(800, 600);
	bg->render();
	std::string t = "Welcome to STAR ESCAPE\n \n \n\nPress all the buttons\nto complete a level\n \nBe careful of the green alien\n \nUse W,A,S,D to move\nAnd shoot with mouse\n \n \n\n\nPress a key to start";
	textSurface = TTF_RenderText_Blended_Wrapped(font2, t.c_str(), { 0, 0, 0 }, 0);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_Rect tr = { 100, 150, 0, 0 };
	SDL_QueryTexture(textTexture, NULL, NULL, &tr.w, &tr.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &tr);
	SDL_RenderPresent(renderer);
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);
	waitForKeyPress();
}

void Game::closeSDL()
{
	Mix_FreeMusic(bgMusic);
	Mix_FreeChunk(shootSound); Mix_FreeChunk(explodeSound); Mix_FreeChunk(pickupSound); Mix_FreeChunk(buySound);
	Mix_CloseAudio();
	TTF_CloseFont(font); TTF_CloseFont(font2); TTF_Quit();
	SDL_DestroyWindow(gameWindow); SDL_DestroyRenderer(renderer); SDL_Quit();
}

void Game::loadMap(int levelNumber)
{
	if (levelNumber == 3) { merchant->setAlive(true); merchant->setX(128); merchant->setY(64); }
	else merchant->setAlive(false);

	for (int row = 0; row < 18; row++)
	{
		for (int col = 0; col < 25; col++)
		{
			int tile = levelMaps->getTileContent(levelNumber, col, row);
			if (tile == 1)
			{
				for (auto block : terrainBlocks) if (!block->getAliveState()) { block->setAlive(true); block->setX(col * 32); block->setY(row * 32); break; }
			}
			if (tile == 2) { pc->setX(col * 32); pc->setY(row * 32); }
			if (tile == 3)
			{
				for (auto npc : npcs) if (!npc->getAliveState()) { npc->setAlive(true); npc->setX(col * 32); npc->setY(row * 32); activeNPCs++; break; }
			}
			if (tile == 5)
			{
				for (auto item : items) if (!item->getAliveState()) { item->setAlive(true); item->setX(col * 32); item->setY(row * 32); activeItems++; break; }
			}
			if (tile == 9) { goal->setX(col * 32); goal->setY(row * 32); }
		}
	}
}

void Game::replayScreen()
{
	GameObject* bg = new GameObject("assets/images/gs.png", 0, 0);
	bg->setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	bg->render();
	textSurface = TTF_RenderText_Blended_Wrapped(font2, "You escaped\n \n \n\ndo you want to play again? (y/n)", { 0, 0, 0 }, 0);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_Rect tr = { 100, 150, 0, 0 };
	SDL_QueryTexture(textTexture, NULL, NULL, &tr.w, &tr.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &tr);
	SDL_RenderPresent(renderer);
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);

	bool exit = false;
	while (!exit)
	{
		SDL_PollEvent(&playerInputEvent);
		if (playerInputEvent.type == SDL_KEYDOWN)
		{
			if (playerInputEvent.key.keysym.sym == 121) { replay = true; gameRunning = true; currentLevel = 0; }
			if (playerInputEvent.key.keysym.sym == 110) replay = false;
			exit = true;
		}
	}
}