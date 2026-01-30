#pragma once
#include "GameObject.h"

class Projectile :public GameObject
{
public:
	Projectile(const char* spriteFileName, int xPos, int yPos, float rotation, int spriteSize);
	void update(float frameTime);
	void renderProjectile();
	Uint32 getSize()  const { return bulletSize; }
	void fire(float xSent, float ySent, float angleSent);
	void fireAtTarget(int startX, int startY, int targetX, int targetY);


	void setBulletSpeed(float newSpeed) { speed = newSpeed; }
	float getDamage() const { return damage; }
	void setDamage(float newDamage) { damage = newDamage; }

private:
	Uint32 lastTimeActivated = 0;
	Uint32 disableTime = 0;
	Uint32  bulletSize = 0;
	float damage = 30, range = 20;
};

