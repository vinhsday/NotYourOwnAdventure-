#pragma once
#include "SDL.h"
#include "Vector2D.h"
#include "TextureLoader.h"
#include "Timer.h"
#include <vector>
#include "Unit.h"

class Unit;

class FireEffect {
public:
    FireEffect(SDL_Renderer* renderer, Vector2D setPos);
    void update(float dT, std::vector<std::shared_ptr<Unit>>& listUnits);
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);
    bool isFinished(){ return durationTimer.timeSIsZero(); }

private:
    Vector2D pos;
    SDL_Texture* textureFire = nullptr;
    Timer durationTimer = Timer(4.0f) ; // Tồn tại 1 giây
    float frameTime = 0.1f; // Mỗi frame 0.1 giây
    float frameTimer = 0.0f;
    int frame = 0;
    int frameCount = 6; // 6 frame trong sprite sheet
    int frameWidth = 480/6; // Giả sử sprite sheet rộng 192px, chia 6 = 32px mỗi frame
    int frameHeight = 48; // Chiều cao sprite (có thể điều chỉnh)

    float damage = 10.0f; // Sát thương gây ra
    float radius = 1.5f; // Phạm vi vùng lửa
    Timer damageCooldown = Timer(1.0f); // Gây sát thương mỗi 0.5 giây
};
