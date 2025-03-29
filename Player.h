#pragma once
#include "SDL.h"
#include "Vector2D.h"
#include "TextureLoader.h"
#include "Projectile.h"
#include "Game.h"
#include <vector>
#include "Level.h"
#include "Timer.h"

class Projectile;
class Unit;
class Game;  // Khai báo trước để tránh lỗi
class Level;


enum class PlayerState {
    IdleLeft,
    IdleRight,
    RunLeft,
    Attack1,
    RunRight,
    Attack1Left,
     Hurt,  // 🔴 Thêm trạng thái bị đánh
    Death,

};

class Player
{
public:
    ~Player();  // Destructor để giải phóng texture

    Player(Game* gamePtr, SDL_Renderer* renderer, Vector2D startPos);
    void handleInput(const Uint8* keyState, SDL_Renderer* renderer);
    void update(float dT, std::vector<std::shared_ptr<Unit>>& listUnits, SDL_Renderer* renderer, Level& level);
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);
    Vector2D getPos(){return pos;}

    int getCurrentHP() { return currentHP; }
    int getMaxHP() { return maxHP; }
    int getCurrentMP() { return currentMP; }
    int getMaxMP() { return maxMP; }

    void removeHealth(int damage);

    void increaseHealth();

    void levelUp();

    int getCoins() const { return coin; }

    int getLastDirection(){return lastDirection;}

    int currentMP = 50;

    bool isDead = false;


    Timer deathTimer = Timer(2.0f);

    int frame = 0;
    int frameCount = 8; // 8 frame mỗi animation
    float frameTime = 0.07f;
    float frameTimer = 0.0f;

    void reset();


private:
    Vector2D pos;
    Vector2D direction;
    float speed = 3.0f;
    SDL_Texture* textureIdleLeft = nullptr;
    SDL_Texture* textureIdleRight = nullptr;
    SDL_Texture* textureRunRight = nullptr;
    SDL_Texture* textureAttack1 = nullptr;
    SDL_Texture* textureRunLeft = nullptr;
    SDL_Texture* textureAttack1Left = nullptr;
    SDL_Texture* textureHurt = nullptr;
    SDL_Texture* textureDeath = nullptr;




    Timer shootCooldown = Timer(0.3f); // Khoảng cách giữa các lần bắn

    void shoot(SDL_Renderer* renderer);




private:
    PlayerState state = PlayerState::IdleRight; // Mặc định là đứng yên
    int spriteWidth = 2000;  // Kích thước toàn bộ spritesheet
    int spriteHeight = 250;
    int frameWidth = spriteWidth / 8;  // Kích thước của một frame = 2000/8 = 250px
    int frameHeight = spriteHeight;
    int lastDirection;

    Vector2D smoothPos; // Vị trí nội suy

    bool isAttacking = false;  // Kiểm soát khi nào player đang tấn công


    float attackRange = 1.5f;  // Phạm vi đánh cận chiến


    Game* game;

    int attackDamage = 10;
    int maxHP = 100;
    int currentHP = 100;
    int maxMP = 50;
    int coin = 0;
    int level = 1;

    Timer damageCooldown = Timer(1.0f); // 1 giây giữa mỗi lần nhận damage


    Timer hurtTimer { 0.5f, 0.0f }; // ⏳ Nhấp nháy 0.5 giây khi bị đánh


    PlayerState prevStateBeforeHurt;
    int prevFrameBeforeHurt;



};
