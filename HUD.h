#pragma once
#include "SDL.h"
#include "Player.h"
#include <vector>
#include <string>

class Unit;
class Player;
class Game;

struct Skill {
    SDL_Texture* icon;
    float cooldown;
    float maxCooldown;

    bool ready() const { return cooldown <= 0; }

    Skill(SDL_Texture* texture, float maxCD)
        : icon(texture), cooldown(0), maxCooldown(maxCD) {}

    void update(float dT) {
        if (cooldown > 0) cooldown -= dT;
    }

    void activate() { cooldown = maxCooldown; }
};

class HUD {
public:
    HUD(SDL_Renderer* renderer, Player* player);
    void update(float dT, const std::vector<std::shared_ptr<Unit>>& units);
    void draw(SDL_Renderer* renderer);
    void addSkill(SDL_Texture* icon, float maxCooldown);
    void useSkill(int index);
    bool handleInput(SDL_Event& event, Game* game);

    std::vector<Skill> skills;

    float survivalTime = 0.0f;
    void resetSurvivalTime() {survivalTime = 0.0f;}

private:
    Player* player;
    SDL_Texture* playerAvatar;
    int enemyCount = 0;

    // Thanh trượt âm lượng (đặt trong khung HUD)
    SDL_Rect volumeBar = { 780, 60, 150, 8 }; // Góc phải khung HUD, nhỏ gọn
    SDL_Rect volumeSlider = { 780, 55, 8, 18 }; // Nút trượt nhỏ hơn

    SDL_Rect pauseButton = { 850, 10, 50, 30 }; // Nút Pause ở góc phải trên
    SDL_Rect quitButton = { 910, 10, 50, 30 }; // Nút Quit bên cạnh
    SDL_Texture* pauseTexture = nullptr;
    SDL_Texture* pauseHoverTexture = nullptr;
    SDL_Texture* quitTexture = nullptr;
    SDL_Texture* quitHoverTexture = nullptr;

    bool draggingVolume = false;
    int volume = 30; // Giá trị âm lượng (0-128)

    void drawHealthBar(SDL_Renderer* renderer, int x, int y, int width, int height);
    void drawManaBar(SDL_Renderer* renderer, int x, int y, int width, int height);
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize, SDL_Color color);
};
