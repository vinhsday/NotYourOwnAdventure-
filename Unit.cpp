#include "Unit.h"
#include "Game.h"
#include <iostream>


const float Unit::size = 0.48f;

Unit::Unit(SDL_Renderer* renderer, Vector2D setPos)
    : pos(setPos), speed(1.0f), health(10), maxHealth(10), attackDamage(5), attackRange(0.5f) {

    textureRun = TextureLoader::loadTexture(renderer, "Slime_run.png");
    textureAttack = TextureLoader::loadTexture(renderer, "Slime_attack.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Slime_hurt.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Slime_death.png");
    speed = 0.5f;

}

void Unit::update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) {
    if (state == UnitState::Death) {
        timerDeath.countDown(dT);
        if (frame < getFrameCount() - 1) {
            frameTimer += dT;
            if (frameTimer >= frameTime) {
                frameTimer = 0.0f;
                frame++;
            }
        }
        if (timerDeath.timeSIsZero()) {
            isdead = true;
        }
        return;
    }

    timerJustHurt.countDown(dT);
    damageCooldown.countDown(dT);

    frameTimer += dT;
    if (frameTimer >= frameTime) {
        frameTimer = 0.0f;
        frame = (frame + 1) % getFrameCount();
    }

    if (state == UnitState::Hurt) {
        if (frame == getFrameCount() - 1) {
            setState(UnitState::Run);
        }
        return;
    }

    Vector2D playerPos = player.getPos();
    Vector2D dirVector = playerPos - pos;
    float length = dirVector.magnitude();

    // 🛠 **Kiểm tra khoảng cách và đổi trạng thái phù hợp**
    if (length > attackRange) {  // Nếu nhân vật đi xa hơn 2 đơn vị
        setState(UnitState::Run);
    } else if (length <= attackRange) {  // Nếu gần hơn 1 đơn vị
        setState(UnitState::Attack);
    }

    if (state == UnitState::Run) {
        if (length > 0) {
            dirVector.normalize();
            pos = pos + dirVector * speed * dT;
        }
    }

    // 🏃 **Cập nhật hướng theo vị trí người chơi**
if (state == UnitState::Run || state == UnitState::Attack) {
    if (length > 0) {
        dirVector.normalize();
        int newDirection;
        if (abs(dirVector.x) > abs(dirVector.y)) {
            newDirection = (dirVector.x > 0) ? 3 : 2;  // Right = 3, Left = 2
        } else {
            newDirection = (dirVector.y > 0) ? 0 : 1;  // Front = 0, Back = 1
        }

        if (newDirection != rowIndex) {
            rowIndex = newDirection;
            frame = 0;
        }
    }
}

// 🚨 Khi quái bị thương hoặc chết, nó giữ nguyên hướng cũ!
if (state == UnitState::Hurt || state == UnitState::Death) {
    frame = (frame + 1) % getFrameCount();
}

    if (length < attackRange && damageCooldown.timeSIsZero()) {
    player.removeHealth(attackDamage);
    damageCooldown.resetToMax();
}
}





void Unit::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) {
    if (!renderer || isDead()) return;

    SDL_Texture* currentTexture = getTextureForState();
    if (!currentTexture) return;  // 🔴 Nếu nullptr thì không vẽ

    int columns = 8;  // ✅ Orc có 8 cột
    int totalFrames = getFrameCount();
    frame = std::max(0, std::min(frame, totalFrames - 1));  // 🔥 Đảm bảo frame hợp lệ

    int row = frame / columns;
    int column = frame % columns;

    SDL_Rect srcRect = { column * frameWidth, rowIndex * frameHeight, frameWidth, frameHeight };
    SDL_Rect destRect = { (int)(pos.x * tileSize) - frameWidth / 2 - (int)(cameraPos.x * tileSize),
                           (int)(pos.y * tileSize) - frameHeight / 2 - (int)(cameraPos.y * tileSize),
                           frameWidth, frameHeight };

    SDL_RenderCopy(renderer, currentTexture, &srcRect, &destRect);
}




bool Unit::checkOverlap(Vector2D posOther, float sizeOther) {
    return (posOther - pos).magnitude() <= (sizeOther + size) / 2.0f;
}

bool Unit::isAlive() { return !isdead; }
bool Unit::isDead() { return isdead; }
Vector2D Unit::getPos() { return pos; }

void Unit::takeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        setState(UnitState::Death);
        timerDeath.resetToMax();  // ⏳ Đếm thời gian chạy hết animation Death
    } else {
        setState(UnitState::Hurt);
        timerJustHurt.resetToMax();
        frame = 0;  // 🔥 Reset lại frame để animation Hurt chạy từ đầu
    }
}



void Unit::setState(UnitState newState) {
    if (state != newState) {
        state = newState;
        frame = 0;
    }
}

SDL_Texture* Unit::getTextureForState() {
    switch (state) {
        case UnitState::Run: return textureRun;
        case UnitState::Attack: return textureAttack;
        case UnitState::Hurt: return textureHurt;
        case UnitState::Death: return textureDeath;
        default: return textureRun;
    }
}

int Unit::getFrameCount() {
    switch (state) {
        case UnitState::Run: return 8;
        case UnitState::Attack: return 10;
        case UnitState::Hurt: return 20;
        case UnitState::Death: return 40;
        default: return 8;
    }
}

