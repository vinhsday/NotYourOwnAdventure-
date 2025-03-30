#include "Orc.h"

Orc::Orc(SDL_Renderer* renderer, Vector2D setPos)
    : Unit(renderer, setPos) {
    textureRun = TextureLoader::loadTexture(renderer, "orc2_run_full.png");
    textureAttack = TextureLoader::loadTexture(renderer, "orc2_attack_full.png");
    textureHurt = TextureLoader::loadTexture(renderer, "orc2_hurt_full.png");
    textureDeath = TextureLoader::loadTexture(renderer, "orc2_death_full.png");

    this->speed = 1.2f;      // 🔥 Tăng tốc độ từ 0.7 → 1.5 (Chạy nhanh hơn)
    this->health = 50;      // 🛡️ Tăng máu từ 5 → 300 (Trâu hơn)
    this->maxHealth = 50;
    this->attackDamage = 15; // ⚔️ Tăng damage từ 5 → 15 (Mạnh hơn)
    this->attackRange = 0.7f; // 📏 Giữ khoảng cách tấn công
}

