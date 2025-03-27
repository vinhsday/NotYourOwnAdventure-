#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include "SDL.h"
#include "Unit.h"
#include "Projectile.h"
#include "Level.h"
#include "Timer.h"
#include "Player.h"
#include "HUD.h"


const int windowWidth = 960;
const int windowHeight = 576;

class HUD;
class Player;
class Projectile;
class Unit;
class Game
{


public:
	Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight);
	~Game();
public:
    void addProjectiles(const Projectile& projectile) {
        listProjectiles.push_back(projectile);
    }
    std::vector<Projectile> listProjectiles;

private:
	void processEvents(SDL_Renderer* renderer, bool& running);
	void update(SDL_Renderer* renderer, float dT, Level& level);
	void updateUnits(float dT);
	void updateProjectiles(float dT);
	void updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT);
	void draw(SDL_Renderer* renderer);
	void addUnit(SDL_Renderer* renderer, Vector2D posMouse);

	int mouseDownStatus = 0;

	const int tileSize = 64;
	Level level;

	std::vector<std::shared_ptr<Unit>> listUnits;

    Player* player;

	Timer spawnTimer, roundTimer;
	int spawnUnitCount = 0;

	void updateCamera();




    Vector2D cameraPos; // 📸 Camera position
    const float cameraSpeed = 5.0f; // 📸 Tốc độ di chuyển camera

    HUD* hud;

};
