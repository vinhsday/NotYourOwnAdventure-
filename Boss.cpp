#include "Boss.h"

Boss::Boss(SDL_Renderer* renderer, Vector2D setPos)
    : Unit(renderer, setPos) {
    this->health = 50;
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

void Boss::update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) {
    animationTimer += dT;  // Cập nhật thời gian animation

    // Kiểm tra trạng thái spawn
    if (isSpawning) {
        spawnTimer -= dT;
        if (spawnTimer <= 0) {
            isSpawning = false; // Sau khi spawn xong, chuyển sang run
        }
    } else {
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
                    isAttacking = false; // Kết thúc tấn công
                    attackFrame = 0;
                    // Gây sát thương cho người chơi
                    if (distance <= 1.5f) {
                        player.removeHealth(attackDamage);
                    }
                }
            }
        } else if (health <= 0) { // Trạng thái chết
            setState(UnitState::Death);
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
    }

    // Cập nhật animation
    if (animationTimer >= frameTimeBoss) {
        animationTimer = 0.0f;
        currentFrame++;

        if (isSpawning) {
            currentFrame %= spawnFrames.size(); // 11 frames
        } else if (state == UnitState::Run) {
            currentFrame %= runFrames.size();   // 6 frames
        } else if (isAttacking) {
            currentFrame = attackFrame;         // 10 frames
        } else if (state == UnitState::Hurt) {
            currentFrame %= hurtFrames.size();  // 4 frames
        } else if (state == UnitState::Death) {
            currentFrame %= deathFrames.size(); // 5 frames
        } else {
            currentFrame %= idleFrames.size();  // 6 frames
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
