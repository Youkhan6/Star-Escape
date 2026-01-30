#include "Levels.h"

int Levels::getTileContent(int level, int x, int y)
{
	level = level - 1;
	return levelMaps[level][y][x];
}


void Levels::setTileContent(int level, int x, int y, int newObject)
{
	levelMaps[level][y][x] = newObject;
}