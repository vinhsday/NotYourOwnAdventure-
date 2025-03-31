#include "Coin.h"
#include <iostream>
#include "Support/AudioManager.h"
#include "Support/TextureLoader.h"
Coin::Coin(Vector2D position, SDL_Renderer* renderer) : pos(position) {
    if (!renderer) {
        std::cout << "Error: Renderer is nullptr in Coin constructor!" << std::endl;
        return;
    }
    texture = TextureLoader::loadTexture(renderer, "coin.png");
}

void Coin::update(float deltaTime) {

    if (collected) return;

    // Xử lý animation
    frameTimer += deltaTime;
    if (frameTimer >= frameTime) {
        frameTimer = 0.0f;
        frame = (frame + 1) % frameCount;
    }
}

void Coin::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) {
    if (collected) return;

    int frameWidth = 128;  // Mỗi frame rộng 128px
    int frameHeight = 128; // Mỗi frame cao 128px

    SDL_Rect srcRect = { frame * frameWidth, 0, frameWidth, frameHeight };

    int drawSize = 16;  // Coin sẽ hiển thị với kích thước 32x32 trên màn hình
    SDL_Rect destRect = {
        (int)(pos.x * tileSize) - drawSize / 2 - (int)(cameraPos.x * tileSize),
        (int)(pos.y * tileSize) - drawSize / 2 - (int)(cameraPos.y * tileSize),
        drawSize, drawSize
    };

    SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
}

bool Coin::checkCollision(Vector2D playerPos, float pickupRange) {
    if ((pos - playerPos).magnitude() < pickupRange) {
        AudioManager::playSound("Data/Sound/coin_pickup.mp3");
        Mix_Chunk* sound = AudioManager::getSound("Data/Sound/coin_pickup.mp3");
        if (sound) Mix_VolumeChunk(sound, 50);
        collected = true;
        return true;
    }
    return false;
}
