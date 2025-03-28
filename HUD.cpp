#include "HUD.h"
#include "SDL_ttf.h"
#include <iostream>

HUD::HUD(SDL_Renderer* renderer, Player* player) : player(player) {
    playerAvatar = TextureLoader::loadTexture(renderer, "Icon18.png");
    if (!playerAvatar) {
        std::cout << "⚠️ HUD: Failed to load player avatar!" << std::endl;
    }


}

void HUD::addSkill(SDL_Texture* icon, float maxCooldown) {
    skills.emplace_back(icon, maxCooldown);
}

void HUD::useSkill(int index) {
    if (index < skills.size() && skills[index].ready()) {
        skills[index].activate();
    }
}

void HUD::update(float dT) {
    for (auto& skill : skills) {
        skill.update(dT);
    }
}

void HUD::draw(SDL_Renderer* renderer) {
    int hudHeight = 100;
    SDL_Rect hudBackground = { 0, 0, 960, hudHeight };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
    SDL_RenderFillRect(renderer, &hudBackground);

    // 🩸 Vẽ thanh máu & mana
    int barWidth = 200;
    int barHeight = 10;
    SDL_Rect healthBarBg = { 120, 20, barWidth, barHeight };
    SDL_Rect healthBar = { 120, 20, (barWidth * player->getCurrentHP()) / player->getMaxHP(), barHeight };
    SDL_Rect manaBarBg = { 120, 40, barWidth, barHeight };
    SDL_Rect manaBar = { 120, 40, (barWidth * player->getCurrentMP()) / player->getMaxMP(), barHeight };

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &healthBarBg);
    SDL_RenderFillRect(renderer, &manaBarBg);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBar);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &manaBar);

    // 🖼 Hiển thị hình nhân vật
    SDL_Rect avatarRect = { 20, 20, 80, 80 };
    SDL_RenderCopy(renderer, playerAvatar, NULL, &avatarRect);

    // 💰 Hiển thị tiền (cần font chữ SDL_ttf)


    // 🎯 Hiển thị skill ở góc dưới trái màn hình
    int x = 50, y = 500;
    int iconSize = 50;

    for (auto& skill : skills) {
        SDL_Rect dstRect = {x, y, iconSize, iconSize};

        // Kiểm tra cooldown
        if (!skill.ready()) {
            SDL_SetTextureColorMod(skill.icon, 100, 100, 100); // Làm icon mờ đi
        } else {
            SDL_SetTextureColorMod(skill.icon, 255, 255, 255);
        }
        SDL_RenderCopy(renderer, skill.icon, NULL, &dstRect);

        // Vẽ thanh cooldown
        if (!skill.ready()) {
            float ratio = skill.cooldown / skill.maxCooldown;
            SDL_Rect cooldownBar = {x, y + iconSize + 5, (int)(iconSize * (1 - ratio)), 5};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &cooldownBar);
        }

        x += iconSize + 10; // Dịch sang phải để hiển thị skill tiếp theo
    }
}
