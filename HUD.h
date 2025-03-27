#pragma once
#include "SDL.h"
#include "Player.h"
#include <vector>

class Player;

struct Skill {
    SDL_Texture* icon;
    float cooldown;      // Thời gian cooldown hiện tại
    float maxCooldown;   // Cooldown tối đa

    bool ready() const { return cooldown <= 0; }

    Skill(SDL_Texture* texture, float maxCD)
        : icon(texture), cooldown(0), maxCooldown(maxCD) {}

    void update(float dT) {
        if (cooldown > 0) cooldown -= dT;
    }

    void activate() { cooldown = maxCooldown; } // Kích hoạt skill và đặt cooldown
};

class HUD {
public:
    HUD(SDL_Renderer* renderer, Player* player);
    void update(float dT);
    void draw(SDL_Renderer* renderer);
    void addSkill(SDL_Texture* icon, float maxCooldown);
    void useSkill(int index);

private:
    Player* player;
    SDL_Texture* playerAvatar;
    std::vector<Skill> skills;
};
