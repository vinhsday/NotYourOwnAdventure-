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
#include "FireEffect.h"
#include "IceEffect.h"

const int windowWidth = 960;
const int windowHeight = 576;

class HUD;
class FireEffect;
class IceEffect;
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

    std::vector<std::shared_ptr<Coin>> coins;

    SDL_Renderer* getRenderer() const { return renderer_; } // Hàm lấy renderer

    bool bossSpawned = false;
    void triggerBossSpawn();
    void spawnBoss();
    bool allEnemiesDead();

    void addFireEffect(const FireEffect& effect) {
        listFireEffects.push_back(effect);
    }

    void addIceEffect(const IceEffect& effect) {
        listIceEffects.push_back(effect);
    }


private:
	void processEvents(SDL_Renderer* renderer, bool& running);
	void update(SDL_Renderer* renderer, float dT, Level& level);
	void updateUnits(float dT);
	void updateProjectiles(float dT);
	void updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT);
	void draw(SDL_Renderer* renderer);
	void addUnit(SDL_Renderer* renderer, Vector2D posMouse);
    void updateFireEffects(float dT);
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

    SDL_Renderer* renderer_;

    std::vector<FireEffect> listFireEffects; // Danh sách hiệu ứng lửa

    std::vector<IceEffect> listIceEffects; // Danh sách hiệu ứng băng
    void updateIceEffects(float dT);



    bool isShaking = false;         // Trạng thái rung
    float shakeTimer = 0.0f;        // Thời gian còn lại của hiệu ứng rung
    float shakeDuration = 0.3f;     // Tổng thời gian rung (0.3 giây)
    float shakeMagnitude = 0.5f;    // Độ lớn của rung (0.5 đơn vị)
    int shakeStep = 0;              // Bước hiện tại trong hiệu ứng rung
};
