#include "Boss.h"
#include <iostream>

Boss::Boss(SDL_Renderer* renderer, Vector2D spawnPos)
    : Unit(renderer, spawnPos), renderer_(renderer) {
    health = 5000;
    maxHealth = 5000;
    attackDamage = 10;
    speed = 0.3f;

    textureRun = TextureLoader::loadTexture(renderer, "Boss_run.png");
    textureAttack = TextureLoader::loadTexture(renderer, "Boss_attack.png");
    textureHurt = TextureLoader::loadTexture(renderer, "Boss_hurt.png");
    textureDeath = TextureLoader::loadTexture(renderer, "Boss_death.png");
    idleTexture = TextureLoader::loadTexture(renderer, "Boss_idle.png");
    spawnTexture = TextureLoader::loadTexture(renderer, "Boss_spawn.png");

    if (!spawnTexture) {
        std::cout << "Error: Failed to load Boss_spawn.png" << std::endl;
    }

    loadAnimations(spawnTexture, spawnFrames, 11);
    loadAnimations(idleTexture, idleFrames, 6);
    loadAnimations(textureRun, runFrames, 6);
    loadAnimations(textureAttack, attackFrames, 10);
    loadAnimations(textureHurt, hurtFrames, 4);
    loadAnimations(textureDeath, deathFrames, 5);

    state = UnitState::Idle;
    isSpawning = true;
    spawnTimer = 2.0f;
    frameTimeBoss = 2.0f / 11.0f;
    currentFrame = 0;
    std::cout << "Boss initialized at pos: (" << pos.x << ", " << pos.y << ")" << std::endl;
}

void Boss::loadAnimations(SDL_Texture* spriteSheet, std::vector<SDL_Rect>& frames, int frameCount) {
    int spriteWidth, spriteHeight;
    SDL_QueryTexture(spriteSheet, NULL, NULL, &spriteWidth, &spriteHeight);
    int frameHeight = 120;

    std::cout << "Loading sprite sheet - Width: " << spriteWidth << ", Height: " << spriteHeight
              << ", FrameCount: " << frameCount << std::endl;

    frames.clear();
    for (int i = 0; i < frameCount; i++) {
        SDL_Rect frame = {0, i * frameHeight, spriteWidth, frameHeight};
        frames.push_back(frame);
        std::cout << "Frame " << i << ": x=" << frame.x << ", y=" << frame.y
                  << ", w=" << frame.w << ", h=" << frame.h << std::endl;
    }
    std::cout << "Total frames loaded: " << frames.size() << std::endl;
}

void Boss::update(float dT, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits, Player& player) {
    std::cout << "Boss::update - isSpawning: " << isSpawning << ", state: " << static_cast<int>(state)
              << ", animationTimer: " << animationTimer << ", spawnTimer: " << spawnTimer
              << ", summonTimer: " << summonTimer << ", currentFrame: " << currentFrame << std::endl;

    animationTimer += dT;
    summonTimer += dT;

    if (isSpawning) {
        spawnTimer -= dT;
        if (animationTimer >= frameTimeBoss) {
            animationTimer = 0.0f;
            if (currentFrame < 10) {
                currentFrame++;
                std::cout << "Update - Spawn Frame: " << currentFrame << std::endl;
            }
        }
        if (spawnTimer <= 0.0f) {
            isSpawning = false;
            state = UnitState::Run;
            currentFrame = 0;
            summonTimer = 0.0f; // Reset để không vào Idle ngay
            std::cout << "Spawn Ended, Switching to Run" << std::endl;
        }
        return;
    }

    if (summonTimer >= 5.0f && state != UnitState::Death) {
        isIdle = true;
        isSummoning = true;
        summonTimer = 0.0f;
        summonMinions(renderer_, level, listUnits);
        idleTimer = 0.0f;
        currentFrame = 0;
        state = UnitState::Idle;
        std::cout << "Switching to Idle for summoning" << std::endl;
        return;
    }

    if (isIdle) {
        idleTimer += dT;
        if (animationTimer >= frameTimeBoss) {
            animationTimer = 0.0f;
            currentFrame = (currentFrame + 1) % idleFrames.size();
            std::cout << "Idle Frame: " << currentFrame << std::endl;
        }
        if (idleTimer >= idleDuration) {
            isIdle = false;
            isSummoning = false;
            state = UnitState::Run;
            currentFrame = 0;
            std::cout << "Idle Ended, Switching to Run" << std::endl;
        }
        return;
    }

    if (state == UnitState::Death) {
        if (animationTimer >= frameTimeBoss) {
            animationTimer = 0.0f;
            if (currentFrame < deathFrames.size() - 1) {
                currentFrame++;
                std::cout << "Death Frame: " << currentFrame << std::endl;
            }
        }
        return;
    }

    Vector2D playerPos = player.getPos();
    Vector2D dirVector = playerPos - pos;
    float distance = dirVector.magnitude();
    std::cout << "Distance to player: " << distance << ", Boss pos: (" << pos.x << ", " << pos.y
              << "), Player pos: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;

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
                    std::cout << "Player hit, damage: " << attackDamage << std::endl;
                }
                state = UnitState::Run;
                std::cout << "Attack Ended, Switching to Run" << std::endl;
            }
        }
    } else if (health <= 0) {
        state = UnitState::Death;
        currentFrame = 0;
        std::cout << "Boss Died" << std::endl;
    } else if (distance > 1.5f) {
        dirVector.normalize();
        pos += dirVector * speed * dT;
        state = UnitState::Run;
        if (animationTimer >= frameTimeBoss) {
            animationTimer = 0.0f;
            currentFrame = (currentFrame + 1) % runFrames.size();
            std::cout << "Run Frame: " << currentFrame << std::endl;
        }
        std::cout << "Boss moving to: (" << pos.x << ", " << pos.y << ")" << std::endl;
    } else {
        isAttacking = true;
        attackTimer = 0.0f;
        attackFrame = 0;
        state = UnitState::Attack;
        std::cout << "Switching to Attack" << std::endl;
    }
}

void Boss::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) {
    SDL_Texture* currentTexture = nullptr;
    SDL_Rect currentFrameRect;

    if (isSpawning) {
        currentTexture = spawnTexture;
        currentFrameRect = {0, currentFrame * 120, 120, 120};
    } else {
        switch (state) {
            case UnitState::Run:
                currentTexture = textureRun;
                currentFrameRect = runFrames[currentFrame];
                break;
            case UnitState::Attack:
                currentTexture = textureAttack;
                currentFrameRect = attackFrames[attackFrame];
                break;
            case UnitState::Hurt:
                currentTexture = textureHurt;
                currentFrameRect = hurtFrames[currentFrame];
                break;
            case UnitState::Death:
                currentTexture = textureDeath;
                currentFrameRect = deathFrames[currentFrame];
                break;
            case UnitState::Idle:
            default:
                currentTexture = idleTexture;
                currentFrameRect = idleFrames[currentFrame];
                break;
        }
    }

    if (currentTexture) {
        std::cout << "Draw - Rendering Frame: " << currentFrame << " (x: " << currentFrameRect.x
                  << ", y: " << currentFrameRect.y << ", w: " << currentFrameRect.w
                  << ", h: " << currentFrameRect.h << ") - Pos: (" << pos.x << ", " << pos.y << ")" << std::endl;
        SDL_Rect destRect = {
            (int)((pos.x - cameraPos.x) * tileSize),
            (int)((pos.y - cameraPos.y) * tileSize),
            currentFrameRect.w,
            currentFrameRect.h
        };
SDL_RenderCopy(renderer, currentTexture, &currentFrameRect, &destRect);    } else {
        std::cout << "Draw - Error: No valid texture!" << std::endl;
    }
}

void Boss::summonMinions(SDL_Renderer* renderer, Level& level, std::vector<std::shared_ptr<Unit>>& listUnits) {
    std::cout << "Boss::summonMinions - isSummoning: " << isSummoning << std::endl;
    if (!isSummoning) return;

    int numMinions = 3;
    int summonRadius = 150;

    for (int i = 0; i < numMinions; ++i) {
        int offsetX = (rand() % (2 * summonRadius)) - summonRadius;
        int offsetY = (rand() % (2 * summonRadius)) - summonRadius;
        Vector2D summonPos = pos + Vector2D((float)offsetX, (float)offsetY);
        listUnits.push_back(std::make_shared<Bat>(renderer, summonPos));
        std::cout << "Summoned Bat at: (" << summonPos.x << ", " << summonPos.y << ")" << std::endl;
    }

    isSummoning = false;
}
