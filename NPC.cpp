#include "NPC.h"
#include <cmath>

NPC::NPC(const char* spriteFileName, int xPos, int yPos, float rotation)
{
    loadSpriteTexture("assets/images/ai.png");

    x = (float)xPos;
    y = (float)yPos;
    angle = rotation;

    srcRect.w = 16;
    srcRect.h = 16;

    destRect.w = 32;
    destRect.h = 32;

    destRect.x = (int)x;
    destRect.y = (int)y;
}

void NPC::render()
{
    drawHPBar();
    SDL_RenderCopyEx(Game::renderer, spriteTexture, &srcRect, &destRect, angle, NULL, SDL_FLIP_NONE);
}

void NPC::update(float frameTime)
{
    oldX = x; oldY = y;

    destRect.x = (int)x;
    destRect.y = (int)y;

    if (health > healthMax) health = healthMax;

    if (health <= 0) setAlive(false);
}

void NPC::chasePC(float pcX, float pcY)
{
    float diffX = x - pcX;
    float diffY = y - pcY;

    if (abs(diffX) > abs(diffY))
    {
        if (diffX > 0) srcRect.y = srcRect.h * 3;
        else           srcRect.y = srcRect.h * 2;
    }
    else
    {
        if (diffY > 0) srcRect.y = srcRect.h * 1;
        else           srcRect.y = srcRect.h * 0;
    }

    if (x > pcX) x--;
    if (x < pcX) x++;
    if (y > pcY) y--;
    if (y < pcY) y++;

    srcRect.x = 0;
}

void NPC::roam(float frameTime)
{
    if (x > (SCREEN_WIDTH - SPRITE_SIZE) || x < 0 || y > SCREEN_HEIGHT - SPRITE_SIZE || y < 0)	angle = rand() % 360 + 1;
    xVel = sin(angle * M_PI / 180) * speed * frameTime;
    yVel = -cos(angle * M_PI / 180) * speed * frameTime;
    x += xVel;
    y += yVel;
}

void NPC::screenCrawl(float frameTime)
{
    if (xVel == 0) { xVel = speed; yVel = -1; }
    if (x > SCREEN_WIDTH - SPRITE_SIZE) { x = SCREEN_WIDTH - SPRITE_SIZE; if (yVel < 0) y += SPRITE_SIZE; else y -= SPRITE_SIZE; xVel = -xVel; }
    if (x < 0) { x = 0; if (yVel < 0) y += SPRITE_SIZE; else y -= SPRITE_SIZE; xVel = -xVel; }
    if (y > SCREEN_HEIGHT - SPRITE_SIZE) { y = SCREEN_HEIGHT - SPRITE_SIZE; yVel = -yVel; }
    if (y < 0) { y = 0; yVel = -yVel; }
    x += xVel * frameTime;
}

void NPC::changeDirection()
{
    angle = rand() % 360 + 1;
    x = oldX; y = oldY;
}

float NPC::checkDistance(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2; float dy = y1 - y2;
    return std::sqrt(dx * dx + dy * dy);
}

void NPC::updateWPPos(float dx, float dy)
{
    int wpTotal = sizeof(waypointsX) / sizeof(waypointsX[0]);
    for (int i = 0; i < wpTotal; i++) { waypointsX[i] += dx; waypointsY[i] += dy; }
}

void NPC::setWayPoints(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5)
{
    waypointsX[0] = x1; waypointsX[1] = x2; waypointsX[2] = x3; waypointsX[3] = x4; waypointsX[4] = x5;
    waypointsY[0] = y1; waypointsY[1] = y2; waypointsY[2] = y3; waypointsY[3] = y4; waypointsY[4] = y5;
}

void NPC::patrol(float frameTime)
{
    targetX = waypointsX[currentWP];
    targetY = waypointsY[currentWP];
    if (checkDistance(x, y, targetX, targetY) < 5)
    {
        currentWP++;
        if (currentWP >= sizeof(waypointsX) / sizeof(waypointsX[0])) currentWP = 0;
        return;
    }
    float rate = 100 * frameTime;
    if (x > targetX) x -= rate; if (x < targetX) x += rate;
    if (y > targetY) y -= rate; if (y < targetY) y += rate;
}