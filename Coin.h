#pragma once
#include "SDL.h"
#include "Support/Vector2D.h"
#include "Support/Vector2D.h"


class Coin {
public:
    Coin(Vector2D position, SDL_Renderer* renderer);
    void update(float deltaTime);
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);
    bool checkCollision(Vector2D playerPos, float pickupRange);
    bool isCollected() const { return collected; }
    Vector2D getPos() const {return pos;}
    int getValue() const { return value; } // Lấy số coin

private:
    Vector2D pos;
    SDL_Texture* texture;
    int value = 0;
    int frame = 0;
    int frameCount = 4; // 4 frame xoay coin
    float frameTime = 0.1f;
    float frameTimer = 0.0f;
    bool collected = false;


};
