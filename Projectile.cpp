#include "Projectile.h"

Projectile::Projectile(const char* spriteFileName, int xPos, int yPos, float rotation, int spriteSize)
{
	loadSpriteTexture(spriteFileName);
	bulletSize = spriteSize;
	x = xPos; 	y = yPos;
	angle = rotation;
	srcRect.h = srcRect.w = bulletSize;
	destRect.h = destRect.w = bulletSize;
}

void Projectile::fire(float startX, float startY, float startAngle)
{
	if (!isActive)
	{
		isActive = true;
		x = startX + SPRITE_SIZE / 2;
		y = startY + SPRITE_SIZE / 2;
		angle = startAngle;
		xVel = sin(angle * M_PI / 180) * speed;
		yVel = -cos(angle * M_PI / 180) * speed;
	}
}
void Projectile::fireAtTarget(int startX, int startY, int targetX, int targetY)
{
	if (!isActive)
	{
		isActive = true;
		x = float(startX + SPRITE_SIZE / 2);
		y = float(startY + SPRITE_SIZE / 2);
		angle = atan2(targetX - x, targetY - y);
		xVel = sin(angle) * speed;
		yVel = cos(angle) * speed;
	}
}
void Projectile::renderProjectile()
{
	SDL_RenderCopyEx(Game::renderer, spriteTexture, &srcRect, &destRect, angle, NULL, SDL_FLIP_NONE);
}
void Projectile::update(float frameTime)
{
	x += xVel * frameTime;
	y += yVel * frameTime;

	disableOffScreen();

	destRect.x = (int)x;
	destRect.y = (int)y;
}
