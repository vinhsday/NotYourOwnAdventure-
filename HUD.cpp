#include "HUD.h"
#include "SDL_ttf.h"
#include <iostream>
#include "Support/AudioManager.h"
#include "Game.h"

HUD::HUD(SDL_Renderer* renderer, Player* player) : player(player) {
    playerAvatar = TextureLoader::loadTexture(renderer, "avatar.png");
    if (!playerAvatar) {
        std::cout << "⚠️ HUD: Failed to load player avatar!" << std::endl;
    }



    pauseTexture = TextureLoader::loadTexture(renderer, "pause01.png");
    pauseHoverTexture = TextureLoader::loadTexture(renderer, "pause03.png");
    quitTexture = TextureLoader::loadTexture(renderer, "back_button.png");
    quitHoverTexture = TextureLoader::loadTexture(renderer, "back03.png");
    speakerTexture = TextureLoader::loadTexture(renderer, "loa_icon1.png");
    silentTexture = TextureLoader::loadTexture(renderer, "loa_icon2.png");
}

void HUD::addSkill(SDL_Texture* icon, float maxCooldown) {
    skills.emplace_back(icon, maxCooldown);
}

void HUD::useSkill(int index) {
    if (index < skills.size() && skills[index].ready()) {
        skills[index].activate();
    }
}

void HUD::update(float dT, const std::vector<std::shared_ptr<Unit>>& units) {
    for (auto& skill : skills) {
        skill.update(dT);
    }

    // Cập nhật thời gian sống sót
    if (!player->isDead) {
        survivalTime += dT;
    }
    else {
        survivalTime = 0.0f;
    }


    // Đếm số kẻ thù còn lại
    enemyCount = 0;
    for (const auto& unit : units) {
        if (unit && unit->isAlive()) {
            enemyCount++;
        }
    }
}

void HUD::draw(SDL_Renderer* renderer) {
    int hudHeight = 100;
    SDL_Rect hudBackground = { 0, 0, 960, hudHeight };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
    SDL_RenderFillRect(renderer, &hudBackground);

    SDL_Rect avatarRect = { 20, 20, 80, 80 };
    SDL_RenderCopy(renderer, playerAvatar, NULL, &avatarRect);

    drawHealthBar(renderer, 120, 20, 200, 10);
    drawManaBar(renderer, 120, 40, 200, 10);

    renderText(renderer, "Coins: " + std::to_string(player->getCoins()), 340, 30, 16, {255, 255, 0});
    renderText(renderer, "Enemies: " + std::to_string(enemyCount), 500, 30, 16, {255, 255, 255});
    renderText(renderer, "Level: " + std::to_string(player->level), 780, 30, 16, {0, 255, 0}); // Màu xanh lá cây
    int minutes = static_cast<int>(survivalTime / 60);
    int seconds = static_cast<int>(survivalTime) % 60;
    std::string timeStr = "Time: " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    renderText(renderer, timeStr, 650, 30, 16, {255, 255, 255});



    // Vẽ nút Pause và Quit
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    bool isHoverPause = (mouseX >= pauseButton.x && mouseX <= pauseButton.x + pauseButton.w &&
                         mouseY >= pauseButton.y && mouseY <= pauseButton.y + pauseButton.h);
    bool isHoverQuit = (mouseX >= quitButton.x && mouseX <= quitButton.x + quitButton.w &&
                        mouseY >= quitButton.y && mouseY <= quitButton.y + quitButton.h);

    SDL_RenderCopy(renderer, isHoverPause ? pauseHoverTexture : pauseTexture, NULL, &pauseButton);
    SDL_RenderCopy(renderer, isHoverQuit ? quitHoverTexture : quitTexture, NULL, &quitButton);
    SDL_RenderCopy(renderer, isMuted ? silentTexture : speakerTexture, NULL, &speakerButton);
    int x = 50, y = 500;
    int iconSize = 50;
    for (auto& skill : skills) {
        SDL_Rect dstRect = {x, y, iconSize, iconSize};
        if (!skill.ready()) {
            SDL_SetTextureColorMod(skill.icon, 100, 100, 100);
        } else {
            SDL_SetTextureColorMod(skill.icon, 255, 255, 255);
        }
        SDL_RenderCopy(renderer, skill.icon, NULL, &dstRect);

        if (!skill.ready()) {
            float ratio = skill.cooldown / skill.maxCooldown;
            SDL_Rect cooldownBar = {x, y + iconSize + 5, (int)(iconSize * (1 - ratio)), 5};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &cooldownBar);
        }
        x += iconSize + 10;
    }
}
void HUD::drawHealthBar(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_Rect bg = { x, y, width, height };
    SDL_Rect bar = { x, y, (width * player->getCurrentHP()) / player->getMaxHP(), height };

    // Viền
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bg);

    // Nền
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &bg);

    // Thanh máu (nhấp nháy nếu dưới 25%)
    if (player->getCurrentHP() < player->getMaxHP() * 0.25f && SDL_GetTicks() % 500 < 250) {
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // Nhấp nháy đỏ nhạt
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    SDL_RenderFillRect(renderer, &bar);
}

void HUD::drawManaBar(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_Rect bg = { x, y, width, height };
    SDL_Rect bar = { x, y, (width * player->getCurrentMP()) / player->getMaxMP(), height };

    // Viền
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bg);

    // Nền
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &bg);

    // Thanh mana
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &bar);
}

void HUD::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize, SDL_Color color) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            std::cout << "SDL_ttf Init Error: " << TTF_GetError() << "\n";
            return;
        }
    }

    TTF_Font* font = TTF_OpenFont("Data/Font/ThaleahFat.ttf", fontSize);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        std::cout << "Text Surface Error: " << TTF_GetError() << "\n";
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cout << "Text Texture Error: " << SDL_GetError() << "\n";
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

bool HUD::handleInput(SDL_Event& event, Game* game) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
        int mouseX = event.button.x;
        int mouseY = event.button.y;
            // Xử lý nút Pause
               if (mouseX >= pauseButton.x && mouseX <= pauseButton.x + pauseButton.w &&
                         mouseY >= pauseButton.y && mouseY <= pauseButton.y + pauseButton.h) {
                    AudioManager::playSound("Data/Sound/Wood Block1.mp3");
                    game->setState(GameState::Paused); // Chuyển sang trạng thái Paused
                    return true;
                }
                // Xử lý nút Quit
                else if (mouseX >= quitButton.x && mouseX <= quitButton.x + quitButton.w &&
                         mouseY >= quitButton.y && mouseY <= quitButton.y + quitButton.h) {
                    AudioManager::playSound("Data/Sound/Wood Block1.mp3");
                    game->restartGame();
                    game->setState(GameState::Menu); // Chuyển sang trạng thái Quit

                    return true;
                }
                 else if (mouseX >= speakerButton.x && mouseX <= speakerButton.x + speakerButton.w &&
                          mouseY >= speakerButton.y && mouseY <= speakerButton.y + speakerButton.h) {
                     AudioManager::playSound("Data/Sound/Wood Block1.mp3");
                     isMuted = !isMuted;
                 if (isMuted){
                    Mix_HaltMusic();
                }
                 else {
                    AudioManager::playMusic("Data/Sound/playing_background_music.mp3", -1);
                    Mix_VolumeMusic(30);
                }
                return true;
                }

            break;
    }


}
