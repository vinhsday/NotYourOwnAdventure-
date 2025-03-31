#include "Vampire.h"

Vampire::Vampire(SDL_Renderer* renderer, Vector2D setPos)
    : Unit(renderer, setPos) {
    textureRun = TextureLoader::loadTexture(renderer, "Vampires2_Run_full.png");
    textureAttack = TextureLoader::loadTexture(renderer, "Vampires2_Attack_full.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Vampires2_Hurt_full.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Vampires2_Death_full.png");

    this->speed = 1.5f;      // 🔥 Tăng tốc độ từ 0.7 → 1.5 (Chạy nhanh hơn)
    this->health = 50;      // 🛡️ Tăng máu từ 5 → 300 (Trâu hơn)
    this->maxHealth = 50;
    this->attackDamage = 25; // ⚔️ Tăng damage từ 5 → 15 (Mạnh hơn)
    this->attackRange = 0.8f; // 📏 Giữ khoảng cách tấn công

}
