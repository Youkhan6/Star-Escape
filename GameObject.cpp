#include "GameObject.h"
#include "Game.h"
#include <SDL_image.h>
#include <iostream>

GameObject::GameObject() {}

GameObject::GameObject(const char* spriteFileName, int xPos, int yPos)
{
	x = float(xPos);
	y = float(yPos);
	loadSpriteTexture(spriteFileName);
}

void GameObject::loadSpriteTexture(const char* spriteFileName)
{
	SDL_Surface* tempSurface = IMG_Load(spriteFileName);
	if (tempSurface == NULL)
	{
		std::cout << "Image Load Error: " << spriteFileName << std::endl;
		return;
	}

	spriteTexture = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
	SDL_FreeSurface(tempSurface);

	int w, h;
	SDL_QueryTexture(spriteTexture, NULL, NULL, &w, &h);

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = w;
	srcRect.h = h;

	destRect.w = SPRITE_SIZE;
	destRect.h = SPRITE_SIZE;
}

void GameObject::render()
{
	SDL_RenderCopy(Game::renderer, spriteTexture, &srcRect, &destRect);
}

void GameObject::update()
{
	destRect.x = (int)x;
	destRect.y = (int)y;
}

void GameObject::setSize(int width, int height)
{
	srcRect.w = float(width);
	srcRect.h = float(height);
	destRect.w = float(width) * SPRITE_SCREEN_SIZE / SPRITE_SIZE;
	destRect.h = float(height) * SPRITE_SCREEN_SIZE / SPRITE_SIZE;
}

void GameObject::screenLimit()
{
	bool stopMoving = false;
	if (x > SCREEN_WIDTH - SPRITE_SIZE)
	{
		stopMoving = true;
		x = SCREEN_WIDTH - SPRITE_SIZE;
	}
	if (x < 0)
	{
		stopMoving = true;
		x = 0;
	}
	if (y > SCREEN_HEIGHT - SPRITE_SIZE)
	{
		stopMoving = true;
		y = SCREEN_HEIGHT - SPRITE_SIZE;
	}
	if (y < 0)
	{
		stopMoving = true;
		y = 0;
	}

	if (stopMoving)
	{
		xVel = 0;
		yVel = 0;
	}
}

void GameObject::screenBounce()
{
	if (x > SCREEN_WIDTH - SPRITE_SIZE)  xVel = -xVel;
	if (x < 0) xVel = -xVel;
	if (y > SCREEN_HEIGHT - SPRITE_SIZE) yVel = -yVel;
	if (y < 0) yVel = -yVel;
}

void GameObject::screenWrap()
{
	if (x > SCREEN_WIDTH - SPRITE_SIZE)  x = 0;
	if (x < 0) x = SCREEN_WIDTH - SPRITE_SIZE;
	if (y > SCREEN_HEIGHT - SPRITE_SIZE) y = 0;
	if (y < 0) y = SCREEN_HEIGHT - SPRITE_SIZE;
}

void GameObject::disableOffScreen()
{
	if (x > SCREEN_WIDTH || x < 0 || y > SCREEN_HEIGHT || y < 0)
	{
		isActive = false;
	}
}

void GameObject::stop()
{
	xVel = yVel = 0;
	x = oldX;
	y = oldY;
}

void GameObject::setLastPosition()
{
	x = oldX;
	y = oldY;
}

void GameObject::drawHPBar() const
{
	SDL_Rect hpBar{ 0,0,0,0 };
	hpBar.w = SPRITE_SIZE * health / 100; ;
	hpBar.h = 2;
	hpBar.x = (int)x;
	hpBar.y = (int)y - 6;

	if (health >= 50)
		SDL_SetRenderDrawColor(Game::renderer, 0, 200, 00, 255);
	else if (health >= 25 && health < 50)
		SDL_SetRenderDrawColor(Game::renderer, 250, 250, 0, 255);
	else
		SDL_SetRenderDrawColor(Game::renderer, 250, 0, 0, 255);

	SDL_RenderDrawRect(Game::renderer, &hpBar);
}