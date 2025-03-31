#include "Player.h"
#include "iostream"
#include "Support/AudioManager.h"

Player::Player(Game* gamePtr, SDL_Renderer* renderer, Vector2D startPos)
    : game(gamePtr), pos(startPos), lastDirection(0),
      currentHP(100), maxHP(100), currentMP(100), maxMP(100),
      isAttacking(false), frame(0), frameTimer(0.0f), speed(5.0f),
      attackDamage(10), attackRange(1.5f), level(1), coin(0), isDead(false) {
    textureIdleLeft = TextureLoader::loadTexture(renderer, "Idle_left.png");
    textureIdleRight = TextureLoader::loadTexture(renderer, "Idle_right.png");
    textureRunLeft = TextureLoader::loadTexture(renderer, "Run_left.png");
    textureRunRight = TextureLoader::loadTexture(renderer, "Run_right.png");
    textureAttack1 = TextureLoader::loadTexture(renderer, "Attack1_right.png");
    textureAttack1Left = TextureLoader::loadTexture(renderer, "Attack1_left.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Hurt.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Death.png");

    shootCooldown = Timer(0.5f);
    damageCooldown = Timer(0.5f);
    hurtTimer = Timer(0.5f);
    deathTimer = Timer(2.0f);

    if (!textureIdleLeft || !textureRunLeft || !textureAttack1 || !textureRunRight) {
        std::cout << "Error loading player textures!\n";
    }
}


Player::~Player() {
    SDL_DestroyTexture(textureIdleLeft);
    SDL_DestroyTexture(textureIdleRight);
    SDL_DestroyTexture(textureRunLeft);
    SDL_DestroyTexture(textureRunRight);
    SDL_DestroyTexture(textureAttack1);
}


void Player::handleInput(const Uint8* keyState, SDL_Renderer* renderer) {
    AudioManager::init();
    if (state == PlayerState::Death) return;

    if (!isAttacking) {  // Không nhận input di chuyển khi đang tấn công
        direction = Vector2D(0, 0);
        bool up    = keyState[SDL_SCANCODE_UP];
        bool down  = keyState[SDL_SCANCODE_DOWN];
        bool left  = keyState[SDL_SCANCODE_LEFT];
        bool right = keyState[SDL_SCANCODE_RIGHT];

        if (up)    direction.y -= 1;
        if (down)  direction.y += 1;
        if (left)  {
    direction.x -= 1;
    AudioManager::playSound("Data/Sound/footstep.wav");
    Mix_VolumeChunk(AudioManager::getSound("Data/Sound/footstep.wav"), 10); // 32 là âm lượng nhỏ
}
if (right) {
    direction.x += 1;
    AudioManager::playSound("Data/Sound/footstep.wav");
    Mix_VolumeChunk(AudioManager::getSound("Data/Sound/footstep.wav"), 10);
}

        }






    if (keyState[SDL_SCANCODE_K] && !isAttacking ) {
        AudioManager::playSound("Data/Sound/player_attack.mp3");
        Mix_VolumeChunk(AudioManager::getSound("Data/Sound/player_attack.mp3"), 30); // 32 là âm lượng nhỏ

    }
    if (!isAttacking) {
    if (direction.x > 0) lastDirection = 1;
    if (direction.x < 0) lastDirection = -1;
}

}


void Player::update(float dT,
                    std::vector<std::shared_ptr<Unit>>& listUnits, SDL_Renderer* renderer, Level& level) {
    hurtTimer.countDown(dT);

    // Lưu trạng thái trước đó để kiểm tra thay đổi animation
    PlayerState previousState = state;

   std::cout << "Update called - dT: " << dT << ", currentMP: " << currentMP << ", maxMP: " << maxMP << "\n";
    if (currentMP < maxMP) {
        currentMP += dT * 5.0f;
        if (currentMP > maxMP) currentMP = maxMP;
        std::cout << "Mana increased to: " << currentMP << "\n";
    } else {
        std::cout << "Mana full: " << currentMP << "/" << maxMP << "\n";
    }

    if (state == PlayerState::Death) {
        deathTimer.countDown(dT);
        frameTimer += dT;

        if (frameTimer >= frameTime) {
            frameTimer = 0.0f;
            if (frame < 6) {
                frame++;
            }
        }

        if (deathTimer.timeSIsZero()) {
            // Xử lý sau khi chết xong (biến mất, hồi sinh, hoặc load màn mới)
        }
        return;
    }

    if (state == PlayerState::Hurt) {
    if (hurtTimer.timeSIsZero()) {
        state = prevStateBeforeHurt;
        frame = prevFrameBeforeHurt;

        // 🚀 Nếu nhân vật đang di chuyển, đặt lại đúng trạng thái
        if (direction.magnitude() > 0) {
            state = (direction.x > 0) ? PlayerState::RunRight : PlayerState::RunLeft;
        }
    } else {
        return;  // 🛑 Không update nếu vẫn còn bị Hurt
    }
}






    // Xử lý phím
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    direction = Vector2D(0, 0);

    if (keyState[SDL_SCANCODE_UP]) direction.y -= 1;
    if (keyState[SDL_SCANCODE_DOWN]) direction.y += 1;
    if (keyState[SDL_SCANCODE_LEFT]) direction.x -= 1;
    if (keyState[SDL_SCANCODE_RIGHT]) direction.x += 1;

    if (direction.magnitude() > 0) {
        direction = direction.normalize();
    }

    // Cập nhật trạng thái di chuyển nếu không đang tấn công
    if (!isAttacking) {
        if (direction.x > 0) {
            state = PlayerState::RunRight;
            lastDirection = 1;
        } else if (direction.x < 0) {
            state = PlayerState::RunLeft;
            lastDirection = -1;
        } else if (direction.magnitude() == 0) {
            state = (lastDirection == -1) ? PlayerState::IdleLeft : PlayerState::IdleRight;
        }
    }

    // Kiểm tra tấn công
    if (!isAttacking) {
        if (keyState[SDL_SCANCODE_K] && shootCooldown.timeSIsZero()) { // Attack1
            state = (lastDirection == -1) ? PlayerState::Attack1Left : PlayerState::Attack1;
            isAttacking = true;
            shootCooldown.resetToMax();
        }

    }

    // Cập nhật animation khi thay đổi trạng thái
    if (state != previousState) {
        frame = 0;
        frameTimer = 0.0f;
    }

    // Tính toán di chuyển
    float distanceMove = speed * dT;
    if (isAttacking) {
        distanceMove *= 0.5f;
    }

    Vector2D newPos = pos + direction * distanceMove;
    pos.x = newPos.x;
    pos.y = newPos.y;

    // Giữ nhân vật trong giới hạn bản đồ
    float minX = 0.5f, minY = 0.5f;
    float maxX = level.GetX() - 0.5f, maxY = level.GetY() - 0.5f;

    if (pos.x < minX) pos.x = minX;
    if (pos.y < minY) pos.y = minY;
    if (pos.x > maxX) pos.x = maxX;
    if (pos.y > maxY) pos.y = maxY;

    // Cập nhật cooldown bắn
    shootCooldown.countDown(dT);

    // Cập nhật frame animation
    frameTimer += dT;
    if (frameTimer >= frameTime) {
        frameTimer = 0.0f;
        frame++;
        int maxFrames = 8;  // Số frame cố định cho animation

        if (frame >= maxFrames) {
            frame = 0;
    if (state == PlayerState::Attack1 || state == PlayerState::Attack1Left) {
        isAttacking = false;

        // Kiểm tra input để không reset về Idle nếu đang di chuyển
        if (direction.x > 0) {
            state = PlayerState::RunRight;
        } else if (direction.x < 0) {
            state = PlayerState::RunLeft;
        } else {
            state = (lastDirection == -1) ? PlayerState::IdleLeft : PlayerState::IdleRight;
        }
    }
        }
    }


    // Cập nhật đạn

    // Làm mượt vị trí nhân vật
    smoothPos = smoothPos + (pos - smoothPos) * 0.2f;

AudioManager::init();

    if (state == PlayerState::Attack1 || state == PlayerState::Attack1Left) {
    if (damageCooldown.timeSIsZero()) { // Chỉ cho phép gây damage khi cooldown về 0
        for (auto& unit : listUnits) {
            float distance = (unit->getPos() - pos).magnitude();
            if (distance < attackRange) { // Phạm vi đánh

                unit->takeDamage(attackDamage, game);
                damageCooldown.resetToMax(); // Bắt đầu thời gian hồi cho lần đánh tiếp theo

            }
        }
    }
}



    // Cập nhật cooldown sát thương
    damageCooldown.countDown(dT);


    // Giữ HP trong giới hạn
    if (currentHP > maxHP) currentHP = maxHP;


     // Kiểm tra va chạm với coin
    for (auto it = game->coins.begin(); it != game->coins.end(); ) {
        if ((*it)->checkCollision(pos, 1.0f)) {
             coin++;   // Phạm vi 1.0f
            it = game->coins.erase(it); // Xóa coin sau khi thu thập
        } else {
            ++it;
        }
    }

}


void Player::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) { // Thêm cameraPos
    if (!renderer) return;

    SDL_Texture* currentTexture = textureIdleRight;
    switch (state) {
        case PlayerState::IdleLeft: currentTexture = textureIdleLeft; break;
        case PlayerState::IdleRight: currentTexture = textureIdleRight; break;
        case PlayerState::RunLeft: currentTexture = textureRunLeft; break;
        case PlayerState::RunRight: currentTexture = textureRunRight; break;
        case PlayerState::Attack1: currentTexture = textureAttack1; break;
        case PlayerState::Attack1Left: currentTexture = textureAttack1Left; break;
        case PlayerState::Hurt: currentTexture = textureHurt; break;
        case PlayerState::Death: currentTexture = textureDeath; break;

        default: currentTexture = (lastDirection == -1) ? textureIdleLeft : textureIdleRight; break;
    }
    if (!currentTexture) return;

    int frameWidth = 2000 / 8;
    int frameHeight = 250;

    SDL_Rect srcRect = { frame * frameWidth, 0, frameWidth, frameHeight };
    SDL_Rect destRect = { (int)(pos.x * tileSize) - frameWidth / 2 - (int)(cameraPos.x * tileSize),
                      (int)(pos.y * tileSize) - frameHeight / 2 - (int)(cameraPos.y * tileSize),
                      frameWidth, frameHeight };

    if (state == PlayerState::Hurt) {
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime / 100) % 2 == 0) {
            SDL_SetTextureAlphaMod(currentTexture, 50); // Làm mờ nhân vật
        } else {
            SDL_SetTextureAlphaMod(currentTexture, 255); // Hiện lại
        }
    } else {
        SDL_SetTextureAlphaMod(currentTexture, 255); // Bình thường không bị mờ
    }

    SDL_RenderCopy(renderer, currentTexture, &srcRect, &destRect);

}



void Player::removeHealth(int damage) {
    AudioManager::init();
    if (!damageCooldown.timeSIsZero()) return;  // Cooldown tránh bị đánh liên tục

    currentHP -= damage;
    damageCooldown.resetToMax();
    hurtTimer.resetToMax();

    if (state != PlayerState::Hurt) {  // 🛠 Chỉ lưu trạng thái nếu chưa bị Hurt
        prevStateBeforeHurt = state;
        prevFrameBeforeHurt = frame;
    }

    state = PlayerState::Hurt;
    frame = 0;  // 🚀 Reset frame Hurt để chắc chắn animation chạy từ đầu

     if (currentHP <= 0) {
        AudioManager::playSound("Data/Sound/player_die.mp3");
        Mix_VolumeChunk(AudioManager::getSound("Data/Sound/player_die.mp3"), 100);

        currentHP = 0;
        state = PlayerState::Death;
        frame = 0;
        isDead = true;
        deathTimer.resetToMax(); // Bắt đầu timer cho animation Death
        return;
    }
}

void Player::increaseHealth() {
    currentHP = maxHP;
}

void Player::levelUp() {
    level++;
    maxHP += 5;
    attackDamage += 3;
    std::cout << "🟢 Level Up! Cấp hiện tại: " << level << "\n";
}



void Player::reset() {
    // Đặt lại trạng thái ban đầu của nhân vật
    currentHP = maxHP;
    currentMP = maxMP;
    pos = Vector2D(15, 10); // Ví dụ: Đặt lại vị trí trung tâm hoặc vị trí spawn
    isDead = false;
    frame = 0;
    state = PlayerState::IdleRight; // Đặt lại trạng thái idle
    shootCooldown.resetToMax();
    damageCooldown.resetToMax();
    hurtTimer.resetToMax();
    deathTimer.resetToMax();
    coin = 0;
    level = 1;
    attackDamage = 10; // Hoặc giá trị mặc định ban đầu
    std::cout << "🟢 Nhân vật đã được hồi sinh!\n";
}





