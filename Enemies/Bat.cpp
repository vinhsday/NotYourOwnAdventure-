#include "Bat.h"
#include <iostream>

Bat::Bat(SDL_Renderer* renderer, Vector2D setPos)
    : Unit(renderer, setPos) {
    textureRun = TextureLoader::loadTexture(renderer, "Bat-Run.png");
    textureAttack = TextureLoader::loadTexture(renderer, "Bat-Attack1.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Bat-Hurt.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Bat-Die.png");

    // Kiểm tra texture
    if (!textureRun) std::cerr << "Error: Failed to load Bat-Run.png" << std::endl;
    if (!textureAttack) std::cerr << "Error: Failed to load Bat-Attack1.png" << std::endl;
    if (!textureHurt) std::cerr << "Error: Failed to load Bat-Hurt.png" << std::endl;
    if (!textureDeath) std::cerr << "Error: Failed to load Bat-Die.png" << std::endl;

    // Thiết lập thông số
    this->speed = 2.0f;
    this->health = 30;
    this->maxHealth = 30;
    this->attackDamage = 20;
    this->attackRange = 0.7f;

    // Đảm bảo trạng thái ban đầu
    state = UnitState::Run;

    // Thiết lập frameWidth và frameHeight cho Bat
    frameWidth = 64;  // Mỗi frame 64 pixel
    frameHeight = 64; // Mỗi frame 64 pixel
}

void Bat::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) {
    if (!renderer || isDead()) return;

    SDL_Texture* currentTexture = getTextureForState();
    if (!currentTexture) {
        std::cerr << "Error: Null texture for Bat state " << static_cast<int>(state) << std::endl;
        return;
    }

    int columns;
    switch (state) {
        case UnitState::Run: columns = 8; break;
        case UnitState::Attack: columns = 8; break;
        case UnitState::Hurt: columns = 5; break;
        case UnitState::Death: columns = 12; break;
        default: columns = 8; break;
    }

    int totalFrames = getFrameCount();
    if (frame >= totalFrames || frame < 0) {
        frame = 0; // Reset frame nếu vượt giới hạn
    }

    int column = frame % columns;
    SDL_Rect srcRect = { column * frameWidth, 0, frameWidth, frameHeight };
    SDL_Rect destRect = {
        (int)(pos.x * tileSize) - frameWidth / 2 - (int)(cameraPos.x * tileSize),
        (int)(pos.y * tileSize) - frameHeight / 2 - (int)(cameraPos.y * tileSize),
        frameWidth, frameHeight
    };

    SDL_RenderCopy(renderer, currentTexture, &srcRect, &destRect);
}

int Bat::getFrameCount() {
    switch (state) {
        case UnitState::Run: return 8;    // Bat-Run: 8 frame
        case UnitState::Attack: return 8; // Bat-Attack1: 8 frame
        case UnitState::Hurt: return 5;   // Bat-Hurt: 5 frame
        case UnitState::Death: return 12; // Bat-Die: 12 frame
        default: return 8;
    }
}
