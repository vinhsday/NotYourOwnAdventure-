#include "Unit.h"
#include "Game.h"
#include <iostream>
#include "AudioManager.h"


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
    std::cout << "Unit::update - currentFrame: " << currentFrame << std::endl;
    AudioManager::init();

    if (state == UnitState::Death) {
        timerDeath.countDown(dT);
        frameTimer += dT;

        if (frameTimer >= frameTime) {
            frameTimer = 0.0f;
            frame = std::min(frame + 1, getFrameCount() - 1);
        }

        if (timerDeath.timeSIsZero()) {
            isdead = true;
        }
        return;
    }

    if (frozen) return;

    timerJustHurt.countDown(dT);
    damageCooldown.countDown(dT);

    frameTimer += dT;
    if (frameTimer >= frameTime) {
        frameTimer = 0.0f;
        frame = (frame + 1) % getFrameCount();
    }

    if (state == UnitState::Hurt) {
        if (frame >= getFrameCount() - 1) setState(UnitState::Run);
        return;
    }

    Vector2D playerPos = player.getPos();
    Vector2D dirVector = playerPos - pos;
    float length = dirVector.magnitude();

    // 🟢 Điều chỉnh trạng thái theo khoảng cách
    if (length > attackRange) {
        setState(UnitState::Run);
    } else {
        setState(UnitState::Attack);
    }

    // 🚫 **Chống đè lên nhau**
    for (auto& otherUnit : listUnits) {
        if (otherUnit.get() == this) continue; // Bỏ qua chính nó

        Vector2D otherPos = otherUnit->getPos();
        Vector2D distanceVec = pos - otherPos;
        float distance = distanceVec.magnitude();

        float minDistance = 0.5f; // Khoảng cách tối thiểu giữa các quái

        // Nếu quái quá gần nhau, đẩy chúng tách ra
        if (distance < minDistance) {
            Vector2D pushAway = distanceVec.normalize() * (minDistance - distance);
            pos = pos + pushAway * dT; // Đẩy nhẹ quái ra
        }
    }

    // 🏃 Di chuyển quái
    if (state == UnitState::Run) {
        if (length > 0) {
            dirVector.normalize();
            pos = pos + dirVector * speed * dT;
        }
    }

    // 🏃 Cập nhật hướng
    if (state == UnitState::Run || state == UnitState::Attack) {
        if (length > 0) {
            dirVector.normalize();
            int newDirection;
            if (abs(dirVector.x) > abs(dirVector.y)) {
                newDirection = (dirVector.x > 0) ? 3 : 2;
            } else {
                newDirection = (dirVector.y > 0) ? 0 : 1;
            }

            if (newDirection != rowIndex) {
                rowIndex = newDirection;
                frame = 0;
            }
        }
    }

    // 🗡 Tấn công người chơi
    if (length < attackRange && damageCooldown.timeSIsZero()) {
        AudioManager::playSound("Data/Sound/small-monster-attack-195712.mp3");
        Mix_VolumeChunk(AudioManager::getSound("Data/Sound/small-monster-attack-195712.mp3"), 50); // 32 là âm lượng nhỏ

        player.removeHealth(attackDamage);
        damageCooldown.resetToMax();
    }
}

void Unit::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) {

    if (!renderer || isDead()) return;

    SDL_Texture* currentTexture = getTextureForState();
    if (!currentTexture) return;

    int columns = 8; // Mặc định 8 cột

    // 🟢 Tự động tính toán số cột dựa trên trạng thái Hurt
    if (state == UnitState::Hurt) {
        int hurtFrameCount = getFrameCount(); // Lấy số frame của Hurt

        // 🟢 Nếu là trạng thái Hurt thì tự động xác định số cột
        if (hurtFrameCount % 6 == 0) {
            columns = 6; // Nếu chia hết cho 6 -> Orc
        } else if (hurtFrameCount % 4 == 0) {
            columns = 4; // Nếu chia hết cho 4 -> Vampire
        }
    }

    int totalFrames = getFrameCount();
    frame = std::max(0, std::min(frame, totalFrames - 1));

    int row = frame / columns;
    int column = frame % columns;

    // 🛠 Kiểm tra frame hợp lệ
    if (row * columns + column >= totalFrames) {
        frame = 0;
        row = 0;
        column = 0;
    }

    SDL_Rect srcRect = { column * frameWidth, rowIndex * frameHeight, frameWidth, frameHeight };
    SDL_Rect destRect = {
        (int)(pos.x * tileSize) - frameWidth / 2 - (int)(cameraPos.x * tileSize),
        (int)(pos.y * tileSize) - frameHeight / 2 - (int)(cameraPos.y * tileSize),
        frameWidth, frameHeight
    };

    SDL_RenderCopy(renderer, currentTexture, &srcRect, &destRect);
}




bool Unit::checkOverlap(Vector2D posOther, float sizeOther) {
    return (posOther - pos).magnitude() <= (sizeOther + size) / 2.0f;
}

bool Unit::isAlive() { return !isdead; }
bool Unit::isDead() { return isdead; }
Vector2D Unit::getPos() { return pos; }

void Unit::takeDamage(int damage, Game* game) {
    AudioManager::init();
    health -= damage;
    if (health <= 0) {
        AudioManager::playSound("Data/Sound/monster-death-grunt-131480.mp3");
        Mix_VolumeChunk(AudioManager::getSound("Data/Sound/monster-death-grunt-131480.mp3"), 50); // 32 là âm lượng nhỏ

        setState(UnitState::Death);
        timerDeath.resetToMax();
        currentFrame = 0; // Đặt lại khung hình
        frameTimer = 0.0f; // Reset thời gian
        if (game) {
            SDL_Renderer* renderer = game->getRenderer();
            if (!renderer) {
                std::cerr << "Error: Renderer is nullptr when creating coin!" << std::endl;
                return;
            }

            int numCoins = rand() % 3 + 1; // Quái rơi từ 1-3 coin
            for (int i = 0; i < numCoins; i++) {
                AudioManager::playSound("Data/Sound/drop-coin-into-glass-33522.mp3");
                Mix_VolumeChunk(AudioManager::getSound("Data/Sound/drop-coin-into-glass-33522.mp3"), 50); // 32 là âm lượng nhỏ

                Vector2D coinOffset((rand() % 10 - 5) * 0.1f, (rand() % 10 - 5) * 0.1f);
                game->coins.push_back(std::make_shared<Coin>(pos + coinOffset, renderer));
            }
        }

    } else {
        setState(UnitState::Hurt);
        timerJustHurt.resetToMax();
        currentFrame = 0;
        frameTimer = 0.0f;  // 🔥 Reset lại frame để animation Hurt chạy từ đầu
    }
}



void Unit::setState(UnitState newState) {
    if (state != newState) {
        state = newState;
        frame = 0; // Reset lại frame

        // ✅ Điều chỉnh frameTime theo từng trạng thái
        switch (state) {
            case UnitState::Run:
                frameTime = 0.1f; // Mượt mà
                break;
            case UnitState::Attack:
                frameTime = 0.15f; // Chậm hơn
                break;
            case UnitState::Hurt:
                frameTime = 0.15f; // Ngắn gọn
                break;
            case UnitState::Death:
                frameTime = 0.05f; // Animation chết kéo dài
                break;
            default:
                frameTime = 0.1f;
                break;
        }
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


