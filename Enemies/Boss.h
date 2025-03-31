#pragma once
#include "Unit.h"
#include "Bat.h"

class Boss : public Unit {
public:
    Boss(SDL_Renderer* renderer, Vector2D spawnPos);
    void update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) override;
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) override;
    int getHealth() { return health; }
    void summonMinions(SDL_Renderer* renderer, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits);
    void takeDamage(int damage, Game* game) override;
private:
    void loadAnimations(SDL_Texture* spriteSheet, std::vector<SDL_Rect>& frames, int frameCount);
    void drawHealthBar(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);

    SDL_Renderer* renderer_;
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
    float frameTimeBoss = 1.0f / 15.0f; // Mặc định 15 FPS
    float summonTimer = 0.0f;
    bool isSummoning = false;
    float idleTimer = 0.0f;
    float idleDuration = 2.0f;
    bool isIdle = false;
    bool hurtAnimationFinished = false; // Theo dõi khi Hurt kết thúc

    int frameWidth = 120;
    int frameHeight = 120;

    float hurtTimer = 0.0f; // Thời gian bị thương
    const float hurtDuration = 0.5f; // Thời gian chạy animation Hurt (0.5 giây)
    UnitState previousState;
};
