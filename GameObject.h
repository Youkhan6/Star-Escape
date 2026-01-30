#pragma once
#include "Game.h"


class GameObject
{
public:
	GameObject();
	GameObject(const char* spriteFileName, int xPos, int yPos);

	void loadSpriteTexture(const char* spriteFileName);
	void update();
	void render();
	bool getAliveState() const { return isActive; }
	void setAlive(bool newState) { isActive = newState; }
	void setSize(int width, int height);
	void setX(float newX) { x = newX; }
	void setY(float newY) { y = newY; }
	float getX() const { return x; }
	float getY() const { return y; }
	void addX(float xAmount) { x += xAmount; }
	void addY(float yAmount) { y += yAmount; }
	float getAngle()  const { return angle; }
	void setAngle(float newAngle) { angle = newAngle; }
	void setSpeed(float newSpeed) { speed = newSpeed; }
	float getVelX() const { return xVel; }
	float getVelY()  const { return yVel; }
	void setVelX(float newXVel) { xVel = newXVel; }
	void setVelY(float newYVel) { yVel = newYVel; }
	float getOldX() const { return oldX; }
	float getOldY() const { return oldY; }
	void stop();
	void setLastPosition();
	void screenLimit();
	void screenWrap();
	void screenBounce();
	void disableOffScreen();

	// Health 
	void drawHPBar() const;
	float getHP() const { return health; }
	void setHP(float newHP) { health = newHP; }
	void changeHP(float hpChange) { health += hpChange; }
	void setTextureRect(int x, int y, int w, int h) { srcRect = { x, y, w, h }; }

protected:
	bool isActive = false;
	float x = -100, y = -100;
	float xVel = 0, yVel = 0;
	float speed = 100;
	float angle = 0;
	float oldX = 0, oldY = 0;

	float health = 100;
	float healthMax = 100;

	SDL_Rect srcRect = { 0,0,0,0 };
	SDL_Rect destRect = { 0,0,0,0 };
	SDL_Texture* spriteTexture = nullptr;
};