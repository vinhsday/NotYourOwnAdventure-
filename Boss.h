#pragma once
#include "Unit.h"
#include "Bat.h"

class Boss : public Unit {
public:
    Boss(SDL_Renderer* renderer, Vector2D spawnPos);
    void update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) override; // Xóa renderer
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) override;
    int getHealth() { return health; }
    void summonMinions(SDL_Renderer* renderer, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits);

private:
    void loadAnimations(SDL_Texture* spriteSheet, std::vector<SDL_Rect>& frames, int frameCount);

    SDL_Texture* idleTexture;
    SDL_Texture* spawnTexture;
    SDL_Texture* attackTexture;
    SDL_Texture* textureRun;
    SDL_Texture* textureHurt;
    SDL_Texture* textureDeath;

    std::vector<SDL_Rect> idleFrames;
    std::vector<SDL_Rect> spawnFrames;
    std::vector<SDL_Rect> attackFrames;
    std::vector<SDL_Rect> runFrames;
    std::vector<SDL_Rect> hurtFrames;
    std::vector<SDL_Rect> deathFrames;

    bool isSpawning = true;
    bool isAttacking = false;
    float spawnTimer = 2.0f;
    float attackTimer = 0.0f;
    int attackFrame = 0;

    float animationTimer = 0.0f;
    float frameTimeBoss = 0.18f;
    float summonTimer = 0.0f;

    bool isSummoning = false;

    float idleTimer = 0.0f;
    float idleDuration = 2.0f;
    bool isIdle = false;

    SDL_Renderer* renderer_; // Lưu renderer trong class
};
