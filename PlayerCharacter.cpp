#include "PlayerCharacter.h"

PlayerCharacter::PlayerCharacter(const char* spriteFileName, int xPos, int yPos)
{
	x = float(xPos);
	y = float(yPos);

	loadSpriteTexture(spriteFileName);
	setAlive(true);

	srcRect.w = 16;
	srcRect.h = 16;

	destRect.w = 32;
	destRect.h = 32;
}

void PlayerCharacter::update(float xAxis, float yAxis, float frameTime)
{
	oldX = x;
	oldY = y;

	float moveSpeed = 150.0f;

	x += xAxis * moveSpeed * frameTime;
	y -= yAxis * moveSpeed * frameTime;

	destRect.x = (int)x;
	destRect.y = (int)y;

	if (yAxis > 0)       srcRect.y = srcRect.h * 1;
	else if (yAxis < 0)  srcRect.y = srcRect.h * 0;
	else if (xAxis > 0)  srcRect.y = srcRect.h * 2;
	else if (xAxis < 0)  srcRect.y = srcRect.h * 3;

	if (xAxis != 0 || yAxis != 0)
	{
		int animSpeed = 150;
		int currentFrame = (SDL_GetTicks() / animSpeed) % 4;
		srcRect.x = currentFrame * srcRect.w;
	}
	else
	{
		srcRect.x = 0;
	}
}

void PlayerCharacter::stop()
{
	x = oldX;
	y = oldY;

	destRect.x = (int)x;
	destRect.y = (int)y;
}

void PlayerCharacter::rotateMove(float xAxis, float yAxis, float frameTime)
{
	if (xAxis != 0) angle += xAxis * rotationSpeed * frameTime;

	if (yAxis != 0)
	{
		xVel += yAxis * sin(angle * M_PI / 180) * acceleration;
		yVel -= yAxis * cos(angle * M_PI / 180) * acceleration;
	}
	else
	{
		if (abs(yVel) > 0.2f) yVel *= drag; else yVel = 0;
		if (abs(xVel) > 0.2f) xVel *= drag; else xVel = 0;
	}

	if (xVel > speed) xVel = speed;
	if (yVel > speed) yVel = speed;
	if (xVel < -speed) xVel = -speed;
	if (yVel < -speed) yVel = -speed;

	x += xVel * frameTime;
	y += yVel * frameTime;
}

void PlayerCharacter::stepMove(float xAxis, float yAxis, float frameTime)
{
	if (xAxis != 0)	 x += xAxis * SPRITE_SIZE;
	if (yAxis != 0)	 y -= yAxis * SPRITE_SIZE;
}

void PlayerCharacter::smoothMove(float xAxis, float yAxis, float frameTime)
{
	if (xAxis != 0)
		xVel += xAxis * acceleration;
	else
		if (abs(xVel) > 5) xVel *= drag; else xVel = 0;

	if (yAxis != 0)
		yVel += -yAxis * acceleration;
	else
		if (abs(yVel) > 5) yVel *= drag; else yVel = 0;

	if (xVel > speed) xVel = speed;
	if (yVel > speed) yVel = speed;
	if (xVel < -speed) xVel = -speed;
	if (yVel < -speed) yVel = -speed;

	x += xVel * frameTime;
	y += yVel * frameTime;
}

void PlayerCharacter::render()
{
	drawHPBar();
	SDL_RenderCopyEx(Game::renderer, spriteTexture, &srcRect, &destRect, angle, NULL, SDL_FLIP_NONE);
}