#pragma once
#include <queue>
#include <vector>
#include <string>
#include "SDL.h"
#include "Vector2D.h"
#include "TextureLoader.h"



class Level
{
private:
	enum class TileType : char {
		empty,
		wall,
		enemySpawner
	};

	static const unsigned char flowDistanceMax = 255;

	struct Tile {
		TileType type = TileType::empty;
		int flowDirectionX = 0;
		int flowDirectionY = 0;
		unsigned char flowDistance = flowDistanceMax;
	};


public:
	Level(SDL_Renderer* renderer, int setTileCountX, int setTileCountY);
	void draw(SDL_Renderer* renderer, int tileSize, float camX, float camY);

	Vector2D getRandomEnemySpawnerLocation();
	Vector2D getTargetPos();
	Vector2D getFlowNormal(int x, int y);
    int GetX(){return tileCountX;}
    int GetY(){return tileCountY;}

    void drawDecor(SDL_Renderer* renderer, float camX, float camY);

    void generateDecorPositions();




private:
	TileType getTileType(int x, int y);
	void setTileType(int x, int y, TileType tileType);
	void calculateFlowField();
	void calculateDistances();
	void calculateFlowDirections();


	std::vector<Tile> listTiles;
	const int tileCountX, tileCountY;

	const int targetX = 0, targetY = 0;

	private:
    SDL_Texture* textureGrass = nullptr;
    std::vector<SDL_Texture*> decorTextures;
    std::vector<Vector2D> decorPositions;


};
