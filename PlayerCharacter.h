#pragma once
#include "GameObject.h"
#include <cmath> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PlayerCharacter : public GameObject
{
public:
	PlayerCharacter(const char* spriteFileName, int xPos, int yPos);
	~PlayerCharacter() {}

	void update(float xAxis, float yAxis, float frameTime);
	void render();
	void stop();
	void rotateMove(float xAxis, float yAxis, float frameTime);
	void stepMove(float xAxis, float yAxis, float frameTime);
	void smoothMove(float xAxis, float yAxis, float frameTime);
	void setHP(int h) { hp = h; }
	int getHP() { return hp; }
	void changeHP(int h) { hp += h; }
	void setSpeed(float s) { speed = s; }
	float getSpeed() { return speed; }
	int money = 0;
	void addMoney(int amount) { money += amount; }
	int getMoney() { return money; }

private:
	int hp = 100;
	float speed = 128.0f;
	float oldX = 0;
	float oldY = 0;
	float acceleration = 10.0f;
	float drag = 0.95f;
	float rotationSpeed = 90.0f;
};