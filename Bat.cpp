#include "Bat.h"

Bat::Bat(SDL_Renderer* renderer,  Vector2D setPos)
    : Unit(renderer, setPos) {
    textureRun = TextureLoader::loadTexture(renderer, "Bat-Run.png");
    textureAttack = TextureLoader::loadTexture(renderer, "Bat-Attack1.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Bat-Hurt.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Bat-Die.png");

    this->speed = 1.2f;      // 🔥 Tăng tốc độ từ 0.7 → 1.5 (Chạy nhanh hơn)
    this->health = 50;      // 🛡️ Tăng máu từ 5 → 300 (Trâu hơn)
    this->maxHealth = 50;
    this->attackDamage = 15; // ⚔️ Tăng damage từ 5 → 15 (Mạnh hơn)
    this->attackRange = 0.7f; // 📏 Giữ khoảng cách tấn công
}
