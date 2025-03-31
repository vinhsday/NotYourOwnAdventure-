#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include "SDL.h"
#include "Enemies/Unit.h"
#include "Level.h"
#include "Support/Timer.h"
#include "Player.h"
#include "HUD.h"
#include "Skills/FireEffect.h"
#include "Skills/IceEffect.h"

const int windowWidth = 960;
const int windowHeight = 576;

class HUD;
class FireEffect;
class IceEffect;
class Player;
class Projectile;
class Unit;

enum class GameState { Menu, Gameplay, GameOver, Quit, Victory, Paused };

class Game {
public:
    Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight);
    ~Game();
    void run(); // Thêm hàm chạy game

    std::vector<std::shared_ptr<Coin>> coins;

    SDL_Renderer* getRenderer() const { return renderer_; }

    bool bossSpawned = false;
    void spawnBoss();
    bool allEnemiesDead();

    void addFireEffect(const FireEffect& effect) { listFireEffects.push_back(effect); }
    void addIceEffect(const IceEffect& effect) { listIceEffects.push_back(effect); }

    bool showMenu(SDL_Renderer* renderer);
    bool showAboutScreen(SDL_Renderer* renderer);
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize);
    void showGameOverMenu(SDL_Renderer* renderer);
    void showVictoryMenu(SDL_Renderer* renderer); // Thêm hàm hiển thị khung Victory
    void restartGame();
    void showPauseMenu(SDL_Renderer* renderer);

    GameState getState() const { return gameState; }
    void setState(GameState state) { gameState = state; }

private:
    void processEvents(SDL_Renderer* renderer, bool& running);
    void update(SDL_Renderer* renderer, float dT, Level& level);
    void updateUnits(float dT);
    void updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT);
    void draw(SDL_Renderer* renderer);
    void addUnit(SDL_Renderer* renderer, Vector2D posMouse);
    void updateFireEffects(float dT);
    void updateIceEffects(float dT);

    const int tileSize = 64;
    Level level;

    std::vector<std::shared_ptr<Unit>> listUnits;
    Player* player;
    Timer spawnTimer, roundTimer;
    int spawnUnitCount = 0;

    void updateCamera();

    Vector2D cameraPos;
    const float cameraSpeed = 5.0f;
    HUD* hud;
    SDL_Renderer* renderer_;

    std::vector<FireEffect> listFireEffects;
    std::vector<IceEffect> listIceEffects;

    bool gameOver = false;
    int windowWidth;
    int windowHeight;

    GameState gameState = GameState::Menu;
};
