#pragma once
#include "Unit.h"
class Boss : public Unit {
public:
    Boss(SDL_Renderer* renderer, Vector2D spawnPos);
    void update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) override;
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) override;
    int getHealth() { return health; };

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
    std::vector<SDL_Rect> runFrames;    // Thêm runFrames
    std::vector<SDL_Rect> hurtFrames;   // Thêm hurtFrames
    std::vector<SDL_Rect> deathFrames;  // Thêm deathFrames

    bool isSpawning = true;
    bool isAttacking = false;
    float spawnTimer = 2.0f;
    float attackTimer = 0.0f;
    int attackFrame = 0;

    float animationTimer = 0.0f;
    float frameTimeBoss = 0.05f;
};
