#pragma once
#include "GameObject.h"
class NPC : public GameObject
{
public:
	NPC(const char* spriteFileName, int xPos, int yPos, float rotation);
	void render();
	void update(float frameTime);
	void roam(float frameTime);
	void chasePC(float pcX, float pcY);
	void screenCrawl(float frameTime);
	void changeDirection();
	int getNextShotTime() const { return nextShotTime; }
	void setNextShotTime(int newTime) { nextShotTime = newTime; }
	float checkDistance(float x1, float y1, float x2, float y2);
	void updateWPPos(float dx, float dy);
	void setWayPoints(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y6);
	void patrol(float frameTime);



private:
	int nextShotTime = 0;

	float waypointsX[5] = {};
	float waypointsY[5] = {};
	int currentWP = 0;
	float targetX = -1, targetY = -1;
};




