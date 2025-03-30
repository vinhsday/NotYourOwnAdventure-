#pragma once
#include <memory>
#include <vector>
#include "SDL.h"
#include "Vector2D.h"
#include "Level.h"
#include "TextureLoader.h"
#include "Timer.h"
#include "Player.h"
#include "Level.h"
class Game;

class Player;

class Level;

enum class UnitState {
    Run,
    Attack,
    Hurt,
    Death,
    Idle
};

enum Direction { FRONT, BACK, LEFT, RIGHT };


class Unit
{
public:
	Unit(SDL_Renderer* renderer,Vector2D setPos);
	virtual void update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) ; // Pure virtual
	virtual void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);
	bool checkOverlap(Vector2D posOther, float sizeOther);
	bool isAlive();
	Vector2D getPos();
	void takeDamage(int damage, Game* game);
	bool isDead();
	void setState(UnitState state_);
	int getFrameCount();
	int getColumnCount();
	SDL_Texture* getTextureForState();

    float getFrameTime();

   bool isFrozen() const { return frozen; }
   void setFrozen(bool value) { frozen = value; }
protected:
    	Vector2D pos;
        UnitState state = UnitState::Run; // Mặc định là chạy

private:
	static const float size;
	SDL_Texture* texture = nullptr;



	Timer changeDirectionTimer = Timer(0.2f);



    Timer timerJustHurt = Timer(0.3f);

    Timer timerDeath = Timer(0.7f);
    bool isdead = false;

    int frame = 0;
    int frameCount = 10;
    float frameTime = 0.1f;
    float frameTimer = 0.0f;

    int frameWidth = 640 / 10;  // 80 px
    int frameHeight = 256 / 4; // 64 px

    Timer damageCooldown = Timer(1.0f); // 1 giây giữa mỗi lần gây sát thương

    float hurtTimer = 0.0f;

    Game* game;

    bool frozen = false;


protected:
    SDL_Texture* textureRun = nullptr;
    SDL_Texture* textureAttack = nullptr;
    SDL_Texture* textureHurt = nullptr;
    SDL_Texture* textureDeath = nullptr;

    float speed;
    float health, maxHealth;

    float attackDamage;  // ⚔️ Sát thương của Unit
    float attackRange;   // 📏 Khoảng cách tấn công

protected:
    Direction direction = FRONT; // Mặc định hướng xuống
    int currentFrame = 0;
    int rowIndex = 0;  // Hàng trong sprite sheet (0: Front, 1: Back, 2: Left, 3: Right)


public:
    void setDirection(Direction dir) {
        direction = dir;
        rowIndex = static_cast<int>(dir);
    }

    Direction getDirection() const { return direction; }

};
