#include "Boss.h"

Boss::Boss(SDL_Renderer* renderer, Vector2D setPos)
    : Unit(renderer, setPos) {
    this->health = 5000;
    this->maxHealth = 50;
    this->attackDamage = 10;
    this->speed = 0.3f;

    // Load các texture cho animation
    textureRun = TextureLoader::loadTexture(renderer, "Boss_run.png");
    textureAttack = TextureLoader::loadTexture(renderer, "Boss_attack.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Boss_hurt.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Boss_death.png");
    idleTexture = TextureLoader::loadTexture(renderer, "Boss_idle.png");
    spawnTexture = TextureLoader::loadTexture(renderer, "Boss_spawn.png");

    // Load các frames của animation với số frame chính xác
    loadAnimations(spawnTexture, spawnFrames, 11);  // 11 frames cho spawn
    loadAnimations(idleTexture, idleFrames, 6);     // 6 frames cho idle
    loadAnimations(textureRun, runFrames, 6);       // 6 frames cho run
    loadAnimations(textureAttack, attackFrames, 10); // 10 frames cho attack
    loadAnimations(textureHurt, hurtFrames, 4);     // 4 frames cho hurt
    loadAnimations(textureDeath, deathFrames, 5);   // 5 frames cho death
}

void Boss::loadAnimations(SDL_Texture* spriteSheet, std::vector<SDL_Rect>& frames, int frameCount) {
    int spriteWidth, spriteHeight;
    SDL_QueryTexture(spriteSheet, NULL, NULL, &spriteWidth, &spriteHeight);

    int frameHeight = spriteHeight / frameCount; // Mỗi frame có chiều cao bằng H / frameCount

    for (int i = 0; i < frameCount; i++) {
        SDL_Rect frame;
        frame.x = 0;
        frame.y = i * frameHeight; // Cắt từng hàng dọc
        frame.w = spriteWidth;
        frame.h = frameHeight;

        frames.push_back(frame);
    }
}

void Boss::update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player, SDL_Renderer* renderer) {
    animationTimer += dT;  // Cập nhật thời gian animation
     summonTimer += dT;
    // Kiểm tra trạng thái spawn
   if (isSpawning) {
    spawnTimer -= dT;

    // Cập nhật animation spawn
    const float spawnSpeedFactor = 2.0f;
    if (animationTimer >= frameTimeBoss * spawnSpeedFactor) {
        animationTimer = 0.0f;
        if (currentFrame < spawnFrames.size() - 1) {
            currentFrame++;
        }
    }

    // Kết thúc spawn
    if (spawnTimer <= 0.0f) {
        isSpawning = false;
        setState(UnitState::Run); // Chuyển trạng thái sang Run
        currentFrame = 0; // Reset lại frame
    }

    return;
}


    // 🟢 Triệu hồi quái mỗi 15 giây
    if (summonTimer >= 15.0f && state != UnitState::Death) {
        isIdle = true;
        isSummoning = true;
        summonTimer = 0.0f;
        summonMinions(renderer, level, listUnits);
        idleTimer = 0.0f;
        currentFrame = 0;
    }

    // 🟢 Trạng thái Idle
    if (isIdle) {
        idleTimer += dT;

        // Animation Idle
        if (animationTimer >= frameTimeBoss) {
            animationTimer = 0.0f;
            currentFrame = (currentFrame + 1) % idleFrames.size();
        }

        // Kết thúc Idle
        if (idleTimer >= 3.0f) {
            isIdle = false;
            isSummoning = false;
            currentFrame = 0;
        }
        return;
    }

    // Nếu boss chết
    if (state == UnitState::Death) {
        animationTimer += dT;
        if (animationTimer >= frameTimeBoss) {
            animationTimer = 0.0f;
            if (currentFrame < deathFrames.size() - 1) {
                currentFrame++;
            }
        }
        return; // Không thực hiện hành vi khác khi đã chết
    }

    // Logic di chuyển và tấn công
    Vector2D playerPos = player.getPos();
    Vector2D dirVector = playerPos - getPos();
    float distance = dirVector.magnitude();

    if (isAttacking) {
        attackTimer += dT;
        if (attackTimer >= frameTimeBoss) {
            attackTimer = 0.0f;
            attackFrame++;
            if (attackFrame >= attackFrames.size()) {
                isAttacking = false;
                attackFrame = 0;
                if (distance <= 1.5f) {
                    player.removeHealth(attackDamage);
                }
            }
        }
    } else if (health <= 0) { // Chuyển trạng thái chết
        setState(UnitState::Death);
        currentFrame = 0;
    } else if (distance > 1.5f) { // Di chuyển về phía người chơi
        dirVector.normalize();
        pos = getPos() + dirVector * speed * dT;
        setState(UnitState::Run);
    } else { // Gần người chơi, tấn công
        isAttacking = true;
        attackTimer = 0.0f;
        attackFrame = 0;
        setState(UnitState::Attack);
    }

    // Cập nhật animation bình thường
    if (animationTimer >= frameTimeBoss) {
        animationTimer = 0.0f;
        currentFrame++;

        if (state == UnitState::Run) {
            currentFrame %= runFrames.size();
        } else if (isAttacking) {
            currentFrame = attackFrame;
        } else if (state == UnitState::Hurt) {
            currentFrame %= hurtFrames.size();
        } else if (state == UnitState::Death) {
            currentFrame = std::min(currentFrame, (int)deathFrames.size() - 1); // Dừng ở frame cuối
        } else {
            currentFrame %= idleFrames.size();
        }
    }
}




void Boss::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) {
    SDL_Texture* currentTexture = nullptr;
    SDL_Rect currentFrameRect;

    // Vẽ texture dựa trên trạng thái
    if (isSpawning) {
        currentTexture = spawnTexture;
        currentFrameRect = spawnFrames[currentFrame];  // 11 frames
    } else if (state == UnitState::Run) {
        currentTexture = textureRun;
        currentFrameRect = runFrames[currentFrame];    // 6 frames
    } else if (isAttacking) {
        currentTexture = textureAttack;
        currentFrameRect = attackFrames[attackFrame];  // 10 frames
    } else if (state == UnitState::Hurt) {
        currentTexture = textureHurt;
        currentFrameRect = hurtFrames[currentFrame];   // 4 frames
    } else if (state == UnitState::Death) {
        currentTexture = textureDeath;
        currentFrameRect = deathFrames[currentFrame];  // 5 frames
    } else {
        currentTexture = idleTexture;
        currentFrameRect = idleFrames[currentFrame];   // 6 frames
    }

    SDL_Rect destRect = {
        (int)((getPos().x - cameraPos.x) * tileSize),
        (int)((getPos().y - cameraPos.y) * tileSize),
        currentFrameRect.w,
        currentFrameRect.h
    };

    SDL_RenderCopy(renderer, currentTexture, &currentFrameRect, &destRect);
}


void Boss::summonMinions(SDL_Renderer* renderer, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits) {
    if (!isSummoning) return; // Chỉ triệu hồi khi đến thời điểm

    int numMinions = 3; // Số quái triệu hồi
    int summonRadius = 150; // Bán kính tạo quái xung quanh boss

    for (int i = 0; i < numMinions; ++i) {
        // Chọn vị trí gần boss
        int offsetX = (rand() % (2 * summonRadius)) - summonRadius;
        int offsetY = (rand() % (2 * summonRadius)) - summonRadius;
        Vector2D summonPos = getPos() + Vector2D(offsetX, offsetY);

        // Thêm quái vào danh sách
        listUnits.push_back(std::make_shared<Bat>(renderer, summonPos));
    }

    isSummoning = false; // Reset trạng thái sau khi triệu hồi
}

