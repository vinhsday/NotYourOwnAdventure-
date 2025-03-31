#pragma once
#include <queue>
#include <vector>
#include <string>
#include "SDL.h"
#include "Support/Vector2D.h"
#include "Support/Vector2D.h"
#include <chrono>
#include <vector>
#include "Coin.h"
class Player;

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

    void spawnPotions();

    void drawPotions(SDL_Renderer* renderer, float camX, float camY);

    void checkPotionPickup(Vector2D characterPosition, Player* player);



private:
	TileType getTileType(int x, int y);
	void setTileType(int x, int y, TileType tileType);
	void calculateFlowField();
	void calculateDistances();
	void calculateFlowDirections();



	std::vector<Tile> listTiles;
	const int tileCountX, tileCountY;

    int targetX = 0, targetY = 0;


	private:
    SDL_Texture* textureGrass = nullptr;
    std::vector<SDL_Texture*> decorTextures;
    std::vector<Vector2D> decorPositions;
    std::vector<Vector2D> potionPositions;
    std::vector<SDL_Texture*> potionTextures;


    SDL_Texture* potionHealthTexture = nullptr;
    SDL_Texture* potionManaTexture = nullptr;

    std::chrono::steady_clock::time_point lastPotionSpawnTime;

    std::vector<Coin> coins;

};
